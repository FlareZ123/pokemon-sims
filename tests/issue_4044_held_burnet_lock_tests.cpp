#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }

  static bool held_burnet_route_ready(const Engine& engine) {
    return engine.late_steven_vstar_grass_with_held_burnet_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State exact_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Fire, sim::Card::Crispin, sim::Card::Arven,
                  sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                  sim::Card::TapuLeleGX};
  return state;
}

bool route_ready_for(const sim::LockMode locks) {
  // Steven's Resolve is the current-turn Supporter. The following turn uses a
  // manual Grass attachment, normal Regidrago V -> VSTAR evolution, and the held
  // Professor Burnet Supporter to put a Dragon payload into discard. Item-only and
  // Rule Box Ability-only locks therefore leave every required action legal, while
  // FullSupporter blocks the Supporter sequence.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official rulebook: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Advanced Supporter, evolution, attachment, and attack procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Repository lock semantics: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4044
  const sim::Scenario scenario{"issue-4044-held-burnet-locks",
                               sim::DciProfile::StrictJit, locks, true, 3};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{4044};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_state());
  return sim::EngineTestAccess::held_burnet_route_ready(engine);
}

void test_action_specific_lock_admission() {
  expect(route_ready_for(sim::LockMode::None),
         "Unlocked route must remain available.");
  expect(route_ready_for(sim::LockMode::FullItem),
         "Full Item lock must not block the Supporter-only continuation.");
  expect(route_ready_for(sim::LockMode::TurnTwoItem),
         "Scheduled Item lock must not block the Supporter-only continuation.");
  expect(route_ready_for(sim::LockMode::FullRuleBoxAbility),
         "Rule Box Ability lock must not block the Trainer/evolution continuation.");
  expect(route_ready_for(sim::LockMode::FullCombined),
         "Combined Item/Rule Box Ability lock must not block the continuation.");
  expect(!route_ready_for(sim::LockMode::FullSupporter),
         "Full Supporter lock must block Steven and Burnet.");
}
}  // namespace

int main() {
  test_action_specific_lock_admission();
  return 0;
}
