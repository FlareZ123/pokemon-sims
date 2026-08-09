#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool serena_latias_route(const Engine& engine) {
    return engine.issue_2205_serena_payload_latias_draw_route_available();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon vstar(const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoVstar, 1, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2434", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2434};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State route_state(const sim::Pokemon& benched_vstar) {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {benched_vstar};
  state.hand = {sim::Card::Serena, sim::Card::Dragapult,
                sim::Card::WishfulBaton};
  state.deck = {sim::Card::MysteriousTreasure, sim::Card::LatiasEx};
  return state;
}

void test_dde_ready_vstar(const int grass, const int fire) {
  Fixture fixture;
  sim::EngineTestAccess::set_state(
      fixture.engine, route_state(vstar(grass, fire, 1)));

  // Serena may spend the held Dragon payload only because the already-powered
  // Benched VSTAR can be promoted through the K1 Treasure -> Latias connector.
  // DDE + either Basic pays Apex Dragon's GGF cost.
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2434
  expect(sim::EngineTestAccess::serena_latias_route(fixture.engine),
         "Serena-Latias rejected a DDE-complete Benched VSTAR.");
}

void test_basic_ggf_control() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(
      fixture.engine, route_state(vstar(2, 1, 0)));
  // Preserve the original issue-2205 Basic GGF route.
  // Original route: https://github.com/FlareZ123/pokemon-sims/issues/2205
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::serena_latias_route(fixture.engine),
         "Original Basic GGF Serena-Latias route regressed.");
}

void test_incomplete_basic_control() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(
      fixture.engine, route_state(vstar(2, 0, 0)));
  // Raw GG without DDE still lacks Fire and cannot satisfy Apex Dragon.
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(!sim::EngineTestAccess::serena_latias_route(fixture.engine),
         "Incomplete Basic-only VSTAR was accepted.");
}

}  // namespace

int main() {
  try {
    test_dde_ready_vstar(1, 0);
    test_dde_ready_vstar(0, 1);
    test_basic_ggf_control();
    test_incomplete_basic_control();
    std::cout << "Issue 2434 Serena-Latias DDE tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
