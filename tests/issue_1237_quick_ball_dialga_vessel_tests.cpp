#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static std::optional<Card> route_cost(const Engine& engine) {
    return engine.quick_ball_dialga_vessel_first_cost();
  }
  static bool outlet_available(Engine& engine) {
    return engine.has_payable_payload_outlet_after_costed_search(Card::QuickBall);
  }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(true); }
  static bool play_earthen_vessel(Engine& engine) { return engine.play_earthen_vessel(true); }
  static bool ready(const Engine& engine) {
    return engine.active_is_vstar() && engine.state_.active->grass >= 2 &&
        engine.state_.active->fire >= 1 && engine.payload_ready();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count_card(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::Scenario scenario(const sim::LockMode locks = sim::LockMode::FullRuleBoxAbility) {
  return sim::Scenario{"issue-1237", sim::DciProfile::StrictJit, locks, false, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::Channeler, sim::Card::Fire,
                sim::Card::QuickBall, sim::Card::EarthenVessel};
  state.deck = {sim::Card::DialgaGX, sim::Card::Grass,
                sim::Card::RegidragoV, sim::Card::Arven};
  return state;
}

sim::Engine make_engine(const sim::Scenario& value, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(value, recipe, rng, nullptr);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void test_channeler_cost_completes_t3_route() {
  std::mt19937_64 rng{123701};
  sim::Engine engine = make_engine(scenario(), rng, route_state());

  // The exact K1 graph is fully paid: Quick Ball discards inert Channeler and
  // searches Basic Dialga-GX, then Earthen Vessel discards Dialga-GX and searches
  // a legal Basic Energy target. Dialga enters discard on the strict-JIT ready turn:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1237
  expect(sim::EngineTestAccess::route_cost(engine) == sim::Card::Channeler,
         "Channeler must be the narrow route-conditioned first cost.");
  expect(sim::EngineTestAccess::outlet_available(engine),
         "The Quick Ball to Dialga to Earthen Vessel outlet must pass preflight.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must execute the admitted route.");
  expect(count_card(sim::EngineTestAccess::state(engine).discard, sim::Card::Channeler) == 1,
         "Quick Ball must discard Channeler.");
  expect(count_card(sim::EngineTestAccess::state(engine).hand, sim::Card::DialgaGX) == 1,
         "Quick Ball must search Dialga-GX.");
  expect(sim::EngineTestAccess::play_earthen_vessel(engine),
         "Earthen Vessel must use the fetched Dialga-GX.");
  expect(count_card(sim::EngineTestAccess::state(engine).discard, sim::Card::DialgaGX) == 1,
         "Dialga-GX must enter discard during the current turn.");
  expect(sim::EngineTestAccess::ready(engine),
         "The Active GGF VSTAR must be ready after the complete chain.");
}

void test_surplus_energy_fallback() {
  std::mt19937_64 rng{123702};
  sim::State state = route_state();
  state.hand.erase(state.hand.begin());
  sim::Engine engine = make_engine(scenario(), rng, std::move(state));
  expect(sim::EngineTestAccess::route_cost(engine) == sim::Card::Fire,
         "A Basic Energy is surplus when GGF is already complete.");
}

void test_controls_block_route() {
  const auto blocked = [](sim::State state, const sim::Scenario value,
                          const bool deck_seen, const std::uint64_t seed,
                          const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(value, rng, std::move(state), deck_seen);
    expect(!sim::EngineTestAccess::route_cost(engine).has_value(), message);
  };

  sim::State no_dialga = route_state();
  no_dialga.deck.erase(no_dialga.deck.begin());
  blocked(no_dialga, scenario(), true, 123703, "Known absent Dialga must block the route.");

  sim::State no_vessel = route_state();
  no_vessel.hand.pop_back();
  blocked(no_vessel, scenario(), true, 123704, "Missing Earthen Vessel must block the route.");

  sim::State no_energy_target = route_state();
  no_energy_target.deck.erase(no_energy_target.deck.begin() + 1);
  blocked(no_energy_target, scenario(), true, 123705, "No legal Vessel Energy target must block the route.");

  sim::State energy_incomplete = route_state();
  energy_incomplete.active->grass = 1;
  blocked(energy_incomplete, scenario(), true, 123706, "Incomplete GGF must preserve the route resources.");

  blocked(route_state(), scenario(sim::LockMode::FullItem), true, 123707,
          "Item lock must block the two-Item route.");
  blocked(route_state(), scenario(), false, 123708,
          "K0 must not assume the exact Dialga and Vessel target chain.");
}
}  // namespace

int main() {
  try {
    test_channeler_cost_completes_t3_route();
    test_surplus_energy_fallback();
    test_controls_block_route();
    std::cout << "issue 1237 tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
