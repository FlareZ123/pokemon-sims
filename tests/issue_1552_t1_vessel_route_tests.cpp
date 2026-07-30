#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static bool attach_manual(Engine& engine) {
    return engine.attach_manual();
  }

  static const State& state(const Engine& engine) {
    return engine.state_;
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State issue_1932_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0};
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::DialgaGX,
      sim::Card::Serena,
      sim::Card::BrilliantBlender,
      sim::Card::QuickBall,
      sim::Card::Fire,
      sim::Card::Grass,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::RegidragoV,
      sim::Card::Channeler,
  };
  state.prizes = {
      sim::Card::Arven,
      sim::Card::FieldBlower,
      sim::Card::Guzma,
      sim::Card::Klara,
      sim::Card::Lusamine,
      sim::Card::PathToPeak,
  };
  state.discard = {
      sim::Card::EarthenVessel,
      sim::Card::MysteriousTreasure,
  };
  return state;
}

struct ProvenanceOutcome {
  bool ready{};
  bool payload_this_turn{};
  bool evolved{};
  bool used_supporter{};
  bool used_manual_attachment{};
};

ProvenanceOutcome run_issue_1932_control(const bool deck_seen,
                                         const bool prizes_revealed,
                                         const std::uint64_t seed) {
  const sim::Scenario scenario{
      "issue-1932", sim::DciProfile::StrictJit,
      sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{seed};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::set_state(
      engine, issue_1932_state(), deck_seen, prizes_revealed);

  expect(sim::EngineTestAccess::attach_manual(engine),
         "The exact turn-two state did not take a legal action");

  const sim::State& state = sim::EngineTestAccess::state(engine);
  const bool evolved = state.active &&
      state.active->card == sim::Card::RegidragoVstar;
  const bool ready = evolved &&
      state.active->grass >= 2 && state.active->fire >= 1;
  const bool payload_this_turn =
      std::find(state.discarded_this_turn.begin(),
                state.discarded_this_turn.end(),
                sim::Card::DialgaGX) != state.discarded_this_turn.end();

  return ProvenanceOutcome{
      ready,
      payload_this_turn,
      evolved,
      state.supporter_used,
      state.manual_energy_used,
  };
}

void test_issue_1932_k1_provenance_equivalence() {
  const ProvenanceOutcome deck_only =
      run_issue_1932_control(true, false, 193201);
  const ProvenanceOutcome prize_only =
      run_issue_1932_control(false, true, 193202);
  const ProvenanceOutcome k0 =
      run_issue_1932_control(false, false, 193203);

  // A resolved deck search or a full Prize inspection establishes the same K1
  // knowledge. The physical Quick Ball, Tapu Lele-GX, Crispin, and attachment
  // route therefore must not depend on which legal inspection supplied K1:
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Correction precedent: https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official evolution, Item, cost, search, Bench, Ability, Supporter,
  // Energy attachment, Prize, and turn procedure:
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1932
  const auto complete = [](const ProvenanceOutcome& outcome) {
    return outcome.ready && outcome.payload_this_turn && outcome.evolved &&
           outcome.used_supporter && outcome.used_manual_attachment;
  };
  expect(complete(deck_only),
         "Deck-search K1 did not execute the issue-1552 turn-two route");
  expect(complete(prize_only),
         "Prize-inspection K1 did not execute the issue-1552 turn-two route");
  expect(!k0.ready && !k0.payload_this_turn && !k0.evolved &&
             !k0.used_supporter && k0.used_manual_attachment,
         "K0 used Prize-aware route information");
}

void test_seed_104_uses_t1_vessel_and_reaches_t2() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1552 fixture is unavailable.");
  std::mt19937_64 rng{104};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  // Earthen Vessel can spend route-replaced Mysterious Treasure on T1, establish
  // K1, and preserve Quick Ball for the T2 Tapu Lele-GX to Crispin continuation:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1552
  // https://github.com/FlareZ123/pokemon-sims/issues/1932
  expect(outcome.first_ready_turn == 2 && !outcome.setup_failed,
         "Seed 104 did not reach strict-JIT readiness on turn two.");
  expect(trace_contains(trace, "Mysterious Treasure (Earthen Vessel issue-1552 route cost)") &&
             trace_contains(trace, "Quick Ball issue-1552 route cost") &&
             trace_contains(trace, "T2 | WONDER TAG") &&
             trace_contains(trace, "T2 | READY |"),
         "Seed 104 did not execute the source-bound Vessel to Quick Ball route.");
}
}  // namespace

int main() {
  try {
    test_issue_1932_k1_provenance_equivalence();
    test_seed_104_uses_t1_vessel_and_reaches_t2();
    std::cout << "Issue 1552 and 1932 K1 route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
