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

  static Card choose_supporter_after_search_started(Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State issue_1597_wonder_tag_state(const bool path_lock_removed = false) {
  sim::State state;
  state.turn = 1;
  state.path_lock_removed = path_lock_removed;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::EarthenVessel, sim::Card::ProfessorTuro,
                sim::Card::Gladion, sim::Card::FieldBlower};
  state.discard = {sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::StevensResolve, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::Arven};
  return state;
}

sim::Card selected_supporter(const sim::LockMode locks,
                             const bool path_lock_removed = false) {
  // The #1597 route spends its Item window on T1. TurnTwoItem begins on the
  // player's second turn, when the remaining deterministic schedule is Steven on
  // T2, Crispin on T3, and Professor Turo on T4. The route requires T1 Item
  // legality, Wonder Tag, and future Supporter legality. A removed Path-style
  // Ability lock therefore restores Wonder Tag without restoring any later Item
  // dependency.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Official rulebook: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Advanced Item, Ability, Supporter, attachment, evolution, and promotion procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // TurnTwoItem schedule: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Lock semantics: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Original deterministic route: https://github.com/FlareZ123/pokemon-sims/issues/1597
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4050
  const sim::Scenario scenario{"issue-4050-turntwo-item",
                               sim::DciProfile::NoDiscardControl,
                               locks, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{4050};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, issue_1597_wonder_tag_state(path_lock_removed));
  return sim::EngineTestAccess::choose_supporter_after_search_started(engine);
}

void turn_two_item_preserves_the_banked_steven_turo_route() {
  expect(selected_supporter(sim::LockMode::None) == sim::Card::StevensResolve,
         "Unlocked #1597 state must bank Steven's Resolve.");
  expect(selected_supporter(sim::LockMode::TurnTwoItem) ==
             sim::Card::StevensResolve,
         "TurnTwoItem must preserve the T1 Wonder Tag -> Steven route.");
}

void route_uses_action_legality_after_path_lock_removal() {
  expect(selected_supporter(sim::LockMode::FullRuleBoxAbility, true) ==
             sim::Card::StevensResolve,
         "Removed Path-style Ability lock must restore the Wonder Tag route.");
  expect(selected_supporter(sim::LockMode::FullCombined, true) ==
             sim::Card::StevensResolve,
         "Removed Ability lock plus T2 Item lock must preserve the completed T1 Item route.");
}

void route_rejects_locks_that_block_required_t1_or_supporter_actions() {
  expect(selected_supporter(sim::LockMode::FullItem) != sim::Card::StevensResolve,
         "Full Item lock must reject the T1 Treasure/Vessel route.");
  expect(selected_supporter(sim::LockMode::FullRuleBoxAbility) !=
             sim::Card::StevensResolve,
         "Live Rule Box Ability lock must reject Wonder Tag.");
  expect(selected_supporter(sim::LockMode::FullCombined) !=
             sim::Card::StevensResolve,
         "Live combined Item/Rule Box Ability lock must reject Wonder Tag.");
  expect(selected_supporter(sim::LockMode::FullSupporter) !=
             sim::Card::StevensResolve,
         "Supporter lock must reject the T2/T3/T4 Supporter schedule.");
}
}  // namespace

int main() {
  turn_two_item_preserves_the_banked_steven_turo_route();
  route_uses_action_legality_after_path_lock_removal();
  route_rejects_locks_that_block_required_t1_or_supporter_actions();
  return 0;
}
