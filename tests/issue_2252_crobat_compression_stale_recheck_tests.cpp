#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static State& state(Engine& engine) { return engine.state_; }
  static bool needs_tapu_current(Engine& engine) {
    return engine.needs_tapu_connector();
  }
  static bool needs_tapu_legacy(Engine& engine) {
    return engine.needs_tapu_connector_original();
  }
  static bool complete(Engine& engine) {
    return engine.complete_issue_1724_crobat_stadium_compression();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::size_t trace_index(const sim::TraceLog& trace, const std::string& needle) {
  const auto found = std::find_if(
      trace.lines.begin(), trace.lines.end(),
      [&needle](const std::string& line) {
        return line.find(needle) != std::string::npos;
      });
  return found == trace.lines.end()
      ? trace.lines.size()
      : static_cast<std::size_t>(std::distance(trace.lines.begin(), found));
}

sim::State stale_recheck_state() {
  sim::State state;
  state.turn = 5;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {
      sim::Card::ChaoticSwell,
      sim::Card::CrobatV,
      sim::Card::TapuLeleGX,
      sim::Card::Grass,
      sim::Card::PathToPeak,
      sim::Card::Powerglass,
      sim::Card::TeamYellsCheer,
  };
  state.deck = {
      sim::Card::ProfessorBurnet,
      sim::Card::MegaDragonite,
      sim::Card::Fire,
  };
  state.discard = {sim::Card::Dragapult};
  state.discarded_this_turn = {sim::Card::Dragapult};
  state.vstar_power_used = true;
  return state;
}

void test_current_guard_does_not_reenter_legacy_tapu_policy() {
  sim::Scenario scenario{"issue-2252-unit", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{2252};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, stale_recheck_state());

  // The held Grass and unused manual attachment complete GGF, while Dragapult ex
  // already satisfies strict JIT this turn. Current route priority therefore treats
  // Wonder Tag -> Crispin as redundant, even though the legacy connector predicate
  // still sees the missing-Energy axis and asks for Tapu Lele-GX:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Bench, Ability, attachment, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Current decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed stale-recheck bug: https://github.com/FlareZ123/pokemon-sims/issues/2252
  expect(!sim::EngineTestAccess::needs_tapu_current(engine),
         "Current policy unexpectedly required redundant Wonder Tag -> Crispin.");
  expect(sim::EngineTestAccess::needs_tapu_legacy(engine),
         "Fixture no longer exposes the legacy/current Tapu predicate disagreement.");

  expect(sim::EngineTestAccess::complete(engine),
         "Current issue-1724 guard did not complete the legal Crobat compression route.");
  const sim::State& state = sim::EngineTestAccess::state(engine);

  expect(state.dark_asset_used,
         "Crobat V was not played from hand and Dark Asset did not resolve.");
  expect(std::any_of(state.bench.begin(), state.bench.end(),
                     [](const sim::Pokemon& pokemon) {
                       return pokemon.card == sim::Card::CrobatV;
                     }),
         "The already-approved Crobat V Bench transition disappeared.");
  expect(state.active && state.active->card == sim::Card::RegidragoVstar &&
             state.active->grass >= 2 && state.active->fire >= 1,
         "The held manual Energy did not complete the proven GGF route.");

  const std::size_t stadium = trace_index(trace, "PLAY STADIUM");
  const std::size_t crobat = trace_index(trace, "BENCH");
  const std::size_t dark_asset = trace_index(trace, "DARK ASSET");
  expect(stadium < crobat && crobat < dark_asset,
         "The legal Chaotic Swell -> Crobat V -> Dark Asset order was lost.");
}
}  // namespace

int main() {
  test_current_guard_does_not_reenter_legacy_tapu_policy();
}
