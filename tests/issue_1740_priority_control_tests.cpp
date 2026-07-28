#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

void test_existing_lower_dci_cost_keeps_priority() {
  const sim::Scenario scenario{"issue-1740-priority", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1745};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure,
                sim::Card::Channeler,
                sim::Card::UltraBall};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The issue override replaces only the duplicate-Treasure fallback. A distinct
  // held Ultra Ball remains the established lower-DCI cost, so Channeler stays held:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item and discard-cost procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Dynamic DCI and decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1740
  expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Mysterious Treasure did not resolve with the lower-DCI Ultra Ball cost.");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count(after.discard, sim::Card::UltraBall) == 1,
         "The existing lower-DCI Ultra Ball cost lost priority.");
  expect(count(after.hand, sim::Card::Channeler) == 1,
         "Channeler was spent before an established lower-DCI cost.");
  expect(count(after.hand, sim::Card::MysteriousTreasure) == 1,
         "The second Treasure was not preserved after the lower-DCI cost.");
}
}  // namespace

int main() {
  test_existing_lower_dci_cost_keeps_priority();
}
