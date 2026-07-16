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

sim::State issue_674_hand() {
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::Powerglass,
                sim::Card::RegidragoVstar};
  return state;
}

void assert_lock_preserves_oricorio(const sim::LockMode lock, const char* label) {
  const sim::Scenario scenario{label, sim::DciProfile::StrictJit, lock, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{674};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_674_hand());

  // Oricorio is a Basic without a Rule Box. Vital Dance is available when it is
  // played from hand onto the Bench, while Mega Dragonite ex keeps a second
  // modeled Dragon payload in hand under every modeled lock state:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://github.com/FlareZ123/pokemon-sims/issues/674
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::Oricorio) ||
      !contains(after.hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Lock scenario should start redundant Dialga-GX and preserve Vital Dance.");
  }
}

void test_all_modeled_locks_preserve_oricorio() {
  assert_lock_preserves_oricorio(sim::LockMode::TurnTwoItem, "issue-674-turn-two-item");
  assert_lock_preserves_oricorio(sim::LockMode::FullItem, "issue-674-full-item");
  assert_lock_preserves_oricorio(sim::LockMode::FullRuleBoxAbility, "issue-674-rulebox");
  assert_lock_preserves_oricorio(sim::LockMode::FullCombined, "issue-674-combined");
}

void test_no_lock_remains_in_issue_663_scope() {
  const sim::Scenario scenario{"issue-674-no-lock-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{67401};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_674_hand());
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::Oricorio ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("No-lock behavior belongs to issue 663 and must remain unchanged here.");
  }
}

void test_unique_dialga_payload_stays_protected() {
  const sim::Scenario scenario{"issue-674-unique-payload-control", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{67402};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = issue_674_hand();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite),
                   state.hand.end());
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::Oricorio ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("The unique strict-JIT payload must remain in hand.");
  }
}

void test_complete_energy_hand_does_not_spend_payload() {
  const sim::Scenario scenario{"issue-674-energy-complete-control", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{67403};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = issue_674_hand();
  state.hand.push_back(sim::Card::Grass);
  state.hand.push_back(sim::Card::Fire);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::Oricorio ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("A hand already holding both Energy types should preserve Dialga-GX.");
  }
}
}  // namespace

int main() {
  test_all_modeled_locks_preserve_oricorio();
  test_no_lock_remains_in_issue_663_scope();
  test_unique_dialga_payload_stays_protected();
  test_complete_energy_hand_does_not_spend_payload();
  return 0;
}
