#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const std::string& label, const std::uint64_t seed)
      : scenario{label, sim::DciProfile::StrictJit, sim::LockMode::None, false, 4},
        recipe(sim::baseline_recipe()),
        rng(seed),
        engine(scenario, recipe, rng) {}
};

sim::State fallback_state(std::vector<sim::Card> prizes) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Gladion};
  state.deck = {sim::Card::RegidragoVstar};
  state.prizes = std::move(prizes);
  return state;
}

void test_erika_is_selected_before_an_unrelated_live_resource() {
  Fixture fixture{"issue-855-erika-low-impact", 85501};
  sim::EngineTestAccess::set_state(
      fixture.engine,
      fallback_state({sim::Card::ForestSealStone, sim::Card::Arven,
                      sim::Card::ErikasInvitation, sim::Card::QuickBall,
                      sim::Card::EarthenVessel, sim::Card::TapuLeleGX}));

  // Gladion must exchange itself for one revealed Prize. Erika's Invitation is the
  // modeled opponent-dependent inert singleton, so the deterministic low-impact
  // fallback selects it before an unrelated live Forest Seal Stone:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/855
  if (!sim::EngineTestAccess::play_gladion(fixture.engine)) {
    throw std::runtime_error("Gladion should perform its mandatory Prize exchange.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::ErikasInvitation) ||
      contains(after.hand, sim::Card::ForestSealStone) ||
      !contains(after.prizes, sim::Card::ForestSealStone) ||
      !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error(
        "Gladion should choose Erika's Invitation before the unrelated live first Prize.");
  }
}

void test_explicit_missing_axis_target_still_precedes_erika() {
  Fixture fixture{"issue-855-explicit-vstar", 85502};
  sim::EngineTestAccess::set_state(
      fixture.engine,
      fallback_state({sim::Card::ErikasInvitation, sim::Card::RegidragoVstar,
                      sim::Card::ForestSealStone, sim::Card::Arven,
                      sim::Card::QuickBall, sim::Card::EarthenVessel}));

  // A revealed Regidrago VSTAR is the exact missing evolution axis and retains
  // priority over the low-impact Erika fallback:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/855
  if (!sim::EngineTestAccess::play_gladion(fixture.engine)) {
    throw std::runtime_error("Gladion should recover the revealed missing VSTAR axis.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) ||
      contains(after.hand, sim::Card::ErikasInvitation) ||
      !contains(after.prizes, sim::Card::ErikasInvitation) ||
      !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error(
        "The exact missing Regidrago VSTAR target should remain ahead of Erika.");
  }
}
}  // namespace

int main() {
  try {
    test_erika_is_selected_before_an_unrelated_live_resource();
    test_explicit_missing_axis_target_still_precedes_erika();
    std::cout << "Gladion Erika low-impact tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
