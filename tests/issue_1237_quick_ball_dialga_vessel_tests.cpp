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
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool quick_ball(Engine& engine) { return engine.play_quick_ball(true); }
  static bool vessel(Engine& engine) { return engine.play_earthen_vessel(true); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State state_with(std::vector<sim::Card> hand, std::vector<sim::Card> deck) {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = std::move(hand);
  state.deck = std::move(deck);
  state.supporter_used = true;
  return state;
}

sim::Engine engine_with(const sim::Scenario& scenario, sim::State state,
                        const std::uint64_t seed) {
  static sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng;
  rng.seed(seed);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);
  return engine;
}

void test_exact_route() {
  const sim::Scenario scenario{"issue-1237-exact", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 4};
  auto engine = engine_with(
      scenario,
      state_with({sim::Card::Channeler, sim::Card::Fire, sim::Card::QuickBall,
                  sim::Card::EarthenVessel},
                 {sim::Card::DialgaGX, sim::Card::Grass}),
      1237001);

  // Quick Ball discards one other card and searches a Basic Pokémon. Dialga-GX is
  // Basic, and Earthen Vessel can discard it while searching Basic Energy. Rule Box
  // Ability lock does not prohibit Items, while Channeler has no modeled opposing
  // attack effect to remove:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sm11-190
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#opponent-actions
  // https://github.com/FlareZ123/pokemon-sims/issues/1237
  expect(sim::EngineTestAccess::quick_ball(engine), "Quick Ball route should play.");
  const auto& after_quick = sim::EngineTestAccess::state(engine);
  expect(contains(after_quick.discard, sim::Card::Channeler),
         "Channeler should pay the first discard cost.");
  expect(contains(after_quick.hand, sim::Card::DialgaGX),
         "Quick Ball should search Dialga-GX.");
  expect(sim::EngineTestAccess::vessel(engine), "Vessel should discard Dialga-GX.");
  const auto& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discarded_this_turn, sim::Card::DialgaGX),
         "Dialga-GX should enter discard this turn.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The route should complete strict-JIT payload timing.");
}

void test_surplus_energy_cost() {
  const sim::Scenario scenario{"issue-1237-surplus", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  auto engine = engine_with(
      scenario,
      state_with({sim::Card::Fire, sim::Card::QuickBall, sim::Card::EarthenVessel},
                 {sim::Card::DialgaGX, sim::Card::Grass}),
      1237002);
  expect(sim::EngineTestAccess::quick_ball(engine),
         "Observable surplus Fire should pay the complete route.");
  expect(contains(sim::EngineTestAccess::state(engine).discard, sim::Card::Fire),
         "Surplus Fire should be discarded when Channeler is absent.");
}

void test_incomplete_routes_hold() {
  const sim::Scenario open{"issue-1237-controls", sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 4};
  const auto holds = [&](std::vector<sim::Card> hand, std::vector<sim::Card> deck,
                         const std::uint64_t seed, const char* message) {
    auto engine = engine_with(open, state_with(std::move(hand), std::move(deck)), seed);
    expect(!sim::EngineTestAccess::quick_ball(engine), message);
  };
  holds({sim::Card::Channeler, sim::Card::QuickBall, sim::Card::EarthenVessel},
        {sim::Card::RegidragoV, sim::Card::Grass}, 1237003,
        "Missing Dialga-GX must hold.");
  holds({sim::Card::Channeler, sim::Card::QuickBall},
        {sim::Card::DialgaGX, sim::Card::Grass}, 1237004,
        "Missing Vessel must hold.");
  holds({sim::Card::Channeler, sim::Card::QuickBall, sim::Card::EarthenVessel},
        {sim::Card::DialgaGX, sim::Card::Dipplin}, 1237005,
        "Missing Basic Energy target must hold.");
  holds({sim::Card::QuickBall, sim::Card::EarthenVessel},
        {sim::Card::DialgaGX, sim::Card::Grass}, 1237006,
        "Missing first discard cost must hold.");

  const sim::Scenario item_lock{"issue-1237-lock", sim::DciProfile::StrictJit,
                                sim::LockMode::FullItem, false, 4};
  auto locked = engine_with(
      item_lock,
      state_with({sim::Card::Channeler, sim::Card::QuickBall,
                  sim::Card::EarthenVessel},
                 {sim::Card::DialgaGX, sim::Card::Grass}),
      1237007);
  expect(!sim::EngineTestAccess::quick_ball(locked), "Item lock must hold.");

  auto energy_incomplete = engine_with(
      open,
      state_with({sim::Card::Channeler, sim::Card::QuickBall,
                  sim::Card::EarthenVessel},
                 {sim::Card::DialgaGX, sim::Card::Grass}),
      1237008);
  sim::State incomplete = sim::EngineTestAccess::state(energy_incomplete);
  incomplete.active->grass = 1;
  sim::EngineTestAccess::set_state(energy_incomplete, std::move(incomplete));
  expect(!sim::EngineTestAccess::quick_ball(energy_incomplete),
         "Incomplete GGF must hold the payload shortcut.");
}
}  // namespace

int main() {
  try {
    test_exact_route();
    test_surplus_energy_cost();
    test_incomplete_routes_hold();
    std::cout << "issue 1237 Quick Ball-Dialga-Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
