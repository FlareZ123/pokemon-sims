#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

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

sim::State opening_with(const sim::Card search_item, const bool add_fodder) {
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, search_item,
                sim::Card::MegaDragonite, sim::Card::ForestSealStone};
  state.hand.push_back(add_fodder ? sim::Card::Dipplin : sim::Card::RegidragoVstar);
  return state;
}

void expect_tapu_active_for_payable_single_item(const sim::Card search_item,
                                                 const std::uint64_t seed) {
  const sim::Scenario scenario{"issue-653-payable-single-item", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{seed};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, opening_with(search_item, true));

  // Either one-discard Item can search Regidrago V when the centralized DCI selector
  // supplies legal fodder. Keeping Dialga-GX in hand preserves the Dragon payload:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("A payable single search Item should preserve Dialga-GX in hand.");
  }
}

void cross_item_cost_remains_live() {
  const sim::Scenario scenario{"issue-653-cross-item", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65303};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::QuickBall,
                sim::Card::MysteriousTreasure, sim::Card::MegaDragonite,
                sim::Card::ForestSealStone};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Each printed effect may discard the other distinct search Item as its cost:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  if (!sim::EngineTestAccess::state(engine).active ||
      sim::EngineTestAccess::state(engine).active->card != sim::Card::TapuLeleGX) {
    throw std::runtime_error("The already-merged cross-Item route regressed.");
  }
}

void protected_hand_rejects_unpayable_single_item() {
  const sim::Scenario scenario{"issue-653-unpayable", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65304};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, opening_with(sim::Card::QuickBall, false));

  // Quick Ball requires another card. Protected singleton setup pieces do not become
  // legal DCI fodder merely because the search Item is present:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("An unpayable single Item must preserve Wonder Tag in hand.");
  }
}

void full_item_lock_rejects_payable_item_route() {
  const sim::Scenario scenario{"issue-653-full-item-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65305};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, opening_with(sim::Card::MysteriousTreasure, true));

  // A payable discard cost does not make an Item usable through full Item lock:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-scenarios
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Full Item lock must reject the one-discard Item signal.");
  }
}
}  // namespace

int main() {
  expect_tapu_active_for_payable_single_item(sim::Card::QuickBall, 65301);
  expect_tapu_active_for_payable_single_item(sim::Card::MysteriousTreasure, 65302);
  cross_item_cost_remains_live();
  protected_hand_rejects_unpayable_single_item();
  full_item_lock_rejects_payable_item_route();
  return 0;
}
