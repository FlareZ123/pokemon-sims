#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_opening_active(Engine& engine) { engine.choose_opening_active(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_dense_public_graph_preserves_wonder_tag() {
  const sim::Scenario scenario{"opening-tapu-oricorio-dense-connectors",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{65401};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Oricorio, sim::Card::Grass,
                sim::Card::MysteriousTreasure, sim::Card::ChaoticSwell,
                sim::Card::Arven, sim::Card::Crispin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Setup may choose either Basic. Energy, Arven, and direct Pokemon search
  // already cover much of Vital Dance's route, while Wonder Tag retains broader
  // post-K1 Supporter access:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/654
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::Oricorio ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error(
        "Oricorio should start while Tapu Lele-GX remains available for Wonder Tag.");
  }
}

void test_duplicate_crispin_with_one_energy_type_preserves_vital_dance() {
  const sim::Scenario scenario{"opening-tapu-oricorio-duplicate-crispin",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{67801};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Oricorio, sim::Card::Crispin,
                sim::Card::Fire, sim::Card::Arven, sim::Card::QuickBall,
                sim::Card::Crispin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // One held Energy type does not complete GGF. Duplicate Crispin and Arven compete
  // for the one-Supporter-per-turn resource, while Vital Dance can find the missing
  // Grass Energy without consuming that Supporter play:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/678
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::Oricorio)) {
    throw std::runtime_error(
        "Duplicate Crispin with one Energy type should preserve Vital Dance.");
  }
}

void test_both_energy_types_preserve_wonder_tag() {
  const sim::Scenario scenario{"opening-tapu-oricorio-both-energy-types",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{67802};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Oricorio, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Arven, sim::Card::QuickBall,
                sim::Card::Crispin};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::Oricorio ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error(
        "Both held Energy types should keep Wonder Tag as the broader connector.");
  }
}

void test_single_crispin_with_one_energy_type_preserves_wonder_tag() {
  const sim::Scenario scenario{"opening-tapu-oricorio-single-crispin",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{67803};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Oricorio, sim::Card::Fire,
                sim::Card::Arven, sim::Card::QuickBall, sim::Card::Crispin,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::Oricorio ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error(
        "A single Crispin should not trigger duplicate-Supporter contention.");
  }
}

void test_unique_wonder_tag_connector_is_preserved() {
  const sim::Scenario scenario{"opening-tapu-oricorio-unique-wonder-tag",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{67804};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Oricorio, sim::Card::Grass,
                sim::Card::QuickBall, sim::Card::ForestSealStone,
                sim::Card::ChaoticSwell, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::Oricorio ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error(
        "Wonder Tag should remain available when it is the only held Supporter connector.");
  }
}

void test_crispin_without_dense_arven_graph_preserves_vital_dance() {
  const sim::Scenario scenario{"opening-tapu-oricorio-crispin-control",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{65402};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Oricorio, sim::Card::Grass,
                sim::Card::MysteriousTreasure, sim::Card::Crispin};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::Oricorio)) {
    throw std::runtime_error(
        "Held Crispin without the dense Arven graph should preserve Vital Dance.");
  }
}

void test_rule_box_lock_still_preserves_vital_dance() {
  const sim::Scenario scenario{"opening-tapu-oricorio-rule-box-control",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{65403};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Oricorio, sim::Card::Grass,
                sim::Card::MysteriousTreasure, sim::Card::Arven};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::Oricorio)) {
    throw std::runtime_error(
        "Rule Box Ability lock should still preserve the non-Rule-Box Vital Dance connector.");
  }
}

}  // namespace

int main() {
  test_dense_public_graph_preserves_wonder_tag();
  test_duplicate_crispin_with_one_energy_type_preserves_vital_dance();
  test_both_energy_types_preserve_wonder_tag();
  test_single_crispin_with_one_energy_type_preserves_wonder_tag();
  test_unique_wonder_tag_connector_is_preserved();
  test_crispin_without_dense_arven_graph_preserves_vital_dance();
  test_rule_box_lock_still_preserves_vital_dance();
  return 0;
}
