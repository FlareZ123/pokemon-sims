#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static Card fss_target(const Engine& engine) {
    return engine.fss_target_after_search_started();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario no_discard_scenario(
    const sim::LockMode locks = sim::LockMode::None,
    const int max_turn = 5) {
  return sim::Scenario{"issue-1735-fss-blender-priority",
                       sim::DciProfile::NoDiscardControl,
                       locks, false, max_turn};
}

sim::State complete_visible_t3_route() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0,
                              sim::Tool::ForestSealStone};
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::EarthenVessel,
      sim::Card::PathToPeak,
  };
  state.deck = {
      sim::Card::StevensResolve,
      sim::Card::BrilliantBlender,
      sim::Card::MegaDragonite,
      sim::Card::Dragapult,
      sim::Card::GoodraVstar,
      sim::Card::DialgaGX,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
  };
  state.supporter_used = true;
  state.manual_energy_used = true;
  return state;
}

sim::Card target_for(sim::State state,
                     const sim::Scenario scenario = no_discard_scenario()) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1735};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));
  return sim::EngineTestAccess::fss_target(engine);
}

void immediate_blender_outranks_banked_steven() {
  // Star Alchemy searches any card. No-discard-control allows the singleton ACE
  // SPEC to establish the payload before the ready turn, and the held VSTAR plus
  // visible Grass and Fire attachments already guarantee the remaining T3 axes:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official attachment and evolution procedure:
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // DCI and route priority:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1735
  expect(target_for(complete_visible_t3_route()) == sim::Card::BrilliantBlender,
         "Star Alchemy did not prefer immediate Blender payload banking");
}

void strict_jit_preserves_steven() {
  sim::Scenario scenario = no_discard_scenario();
  scenario.dci = sim::DciProfile::StrictJit;
  expect(target_for(complete_visible_t3_route(), scenario) ==
             sim::Card::StevensResolve,
         "Strict JIT incorrectly banked the payload on T1");
}

void incomplete_energy_schedule_preserves_steven() {
  sim::State state = complete_visible_t3_route();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Fire));
  expect(target_for(std::move(state)) == sim::Card::StevensResolve,
         "Blender displaced Steven without the complete manual schedule");
}

void missing_vstar_preserves_steven() {
  sim::State state = complete_visible_t3_route();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::RegidragoVstar));
  expect(target_for(std::move(state)) == sim::Card::StevensResolve,
         "Blender displaced Steven without a held VSTAR");
}

void missing_blender_does_not_invent_the_ace_spec() {
  sim::State state = complete_visible_t3_route();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::BrilliantBlender));
  expect(target_for(std::move(state)) != sim::Card::BrilliantBlender,
         "Star Alchemy selected an absent Brilliant Blender");
}

void current_item_lock_blocks_blender() {
  expect(target_for(complete_visible_t3_route(),
                    no_discard_scenario(sim::LockMode::FullItem)) !=
             sim::Card::BrilliantBlender,
         "Star Alchemy selected Blender through current Item lock");
}

void short_horizon_preserves_the_existing_target() {
  expect(target_for(complete_visible_t3_route(),
                    no_discard_scenario(sim::LockMode::None, 2)) ==
             sim::Card::StevensResolve,
         "Blender displaced Steven outside the modeled T3 horizon");
}

void exact_seed_reaches_by_turn_three() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-second");
  expect(scenario.has_value(), "Missing no-discard-control going-second scenario");
  const sim::CrobatModelingDeck* modeling =
      sim::crobat_modeling_deck_by_id("crobat1-heavy-ball");
  expect(modeling != nullptr, "Missing crobat1-heavy-ball modeling recipe");

  std::mt19937_64 rng{2026};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*scenario, modeling->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();

  const auto has_line = [&trace](const std::string& first,
                                 const std::string& second) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&first, &second](const std::string& line) {
      return line.find(first) != std::string::npos &&
             line.find(second) != std::string::npos;
    });
  };

  // The exact K1 seed now uses Star Alchemy for Blender on T1. Blender banks four
  // legal Dragon payloads, leaving the held VSTAR and manual attachment schedule:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1735
  expect(outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 3,
         "Seed 2026 did not reach readiness by T3");
  expect(has_line("T1 | STAR ALCHEMY", "Brilliant Blender"),
         "Seed 2026 did not search Brilliant Blender with Star Alchemy");
  expect(has_line("T1 | PLAY ITEM", "R-BLENDER-01"),
         "Seed 2026 did not bank the payload with Brilliant Blender");
}

}  // namespace

int main() {
  immediate_blender_outranks_banked_steven();
  strict_jit_preserves_steven();
  incomplete_energy_schedule_preserves_steven();
  missing_vstar_preserves_steven();
  missing_blender_does_not_invent_the_ace_spec();
  current_item_lock_blocks_blender();
  short_horizon_preserves_the_existing_target();
  exact_seed_reaches_by_turn_three();
  return 0;
}
