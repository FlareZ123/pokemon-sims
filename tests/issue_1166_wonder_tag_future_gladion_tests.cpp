#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string_view>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool future_gladion_route(const Engine& engine) {
    return engine.wonder_tag_future_gladion_treasure_route();
  }
  static Card choose_supporter(const Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  explicit Fixture(const sim::LockMode locks = sim::LockMode::None,
                   const int max_turn = 5)
      : scenario{"issue-1166", sim::DciProfile::StrictJit, locks, true, max_turn},
        recipe{sim::baseline_recipe()},
        rng{1166},
        engine{scenario, recipe, rng} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.supporter_used = true;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 2, 2, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 3, 0, 0, sim::Tool::None}};
  state.hand = {sim::Card::Fire, sim::Card::GoodraVstar, sim::Card::Lusamine};
  state.deck = {sim::Card::Gladion, sim::Card::Crispin,
                sim::Card::RegidragoVstar, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Arven};
  return state;
}

void expect_crispin(sim::State state, const std::string_view label,
                    const sim::LockMode locks = sim::LockMode::None,
                    const int max_turn = 5, const bool deck_seen = true,
                    const bool prizes_revealed = true) {
  Fixture fixture{locks, max_turn};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), deck_seen,
                                   prizes_revealed);
  if (sim::EngineTestAccess::future_gladion_route(fixture.engine) ||
      sim::EngineTestAccess::choose_supporter(fixture.engine) != sim::Card::Crispin) {
    throw std::runtime_error(std::string(label) + ": generic Crispin route must remain selected");
  }
}

void test_banks_gladion_over_redundant_crispin() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state(), true, true);

  // GG is already attached and Fire is held for T4, so Crispin has no marginal
  // setup value. Wonder Tag banks Gladion for the known prized Treasure continuation:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1166
  if (!sim::EngineTestAccess::future_gladion_route(fixture.engine) ||
      sim::EngineTestAccess::choose_supporter(fixture.engine) != sim::Card::Gladion) {
    throw std::runtime_error("Wonder Tag must bank Gladion for the K1 Treasure route");
  }
}

void test_requires_known_prized_treasure_chain() {
  sim::State state = route_state();
  state.prizes.clear();
  expect_crispin(std::move(state), "Treasure absent from Prizes");

  state = route_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Gladion));
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true, true);
  if (sim::EngineTestAccess::future_gladion_route(fixture.engine)) {
    throw std::runtime_error("the route must reject an absent Gladion");
  }

  state = route_state();
  expect_crispin(std::move(state), "unknown Prize map", sim::LockMode::None, 5, true, false);
}

void test_requires_held_fire_and_protected_payload() {
  sim::State state = route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Fire));
  expect_crispin(std::move(state), "Fire absent from hand");

  state = route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::GoodraVstar));
  expect_crispin(std::move(state), "payload absent from hand");
}

void test_requires_future_legal_window() {
  expect_crispin(route_state(), "Item lock", sim::LockMode::TurnTwoItem);
  expect_crispin(route_state(), "insufficient turns", sim::LockMode::None, 4);

  sim::State state = route_state();
  state.manual_energy_used = false;
  expect_crispin(std::move(state), "unused T3 attachment");
}

}  // namespace

int main() {
  test_banks_gladion_over_redundant_crispin();
  test_requires_known_prized_treasure_chain();
  test_requires_held_fire_and_protected_payload();
  test_requires_future_legal_window();
  return 0;
}
