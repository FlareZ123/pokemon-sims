#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool route_available(const Engine& engine) {
    return engine.issue_1236_pre_steven_vessel_route_available();
  }
  static std::optional<Card> vessel_cost(const Engine& engine) {
    return engine.choose_discard(false, true, true, Card::EarthenVessel);
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void start_t2(Engine& engine) {
    engine.state_.turn = 2;
    engine.state_.turn_ended = false;
    engine.state_.supporter_used = false;
    engine.state_.manual_energy_used = false;
  }
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

sim::Scenario flex_second(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-1236", sim::DciProfile::MatchupFlexJit, locks, false, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::LatiasEx, sim::Card::RoseannesBackup,
                sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure,
                sim::Card::GoodraVstar, sim::Card::EarthenVessel,
                sim::Card::ForestSealStone};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::StevensResolve,
                sim::Card::Crispin, sim::Card::BrilliantBlender,
                sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Fire,
                sim::Card::RegidragoV, sim::Card::DialgaGX};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_vessel_and_attachment_precede_steven() {
  std::mt19937_64 rng{123601};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(flex_second(), rng, route_state(), &trace);

  // Earthen Vessel first uses one duplicate Treasure, the surviving Treasure uses
  // Roseanne's Backup for VSTAR, and the manual Grass attachment occurs before
  // Steven's Resolve ends T1. Crispin, the T2 manual Fire, and Brilliant Blender
  // then complete GGF plus a current-turn payload on T2:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Official Item, Tool, VSTAR Power, Supporter, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1236
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact pre-Steven Vessel route must be recognized.");
  expect(sim::EngineTestAccess::vessel_cost(engine) == sim::Card::MysteriousTreasure,
         "One duplicate Treasure must pay Earthen Vessel before Roseanne's Backup.");
  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after_t1 = sim::EngineTestAccess::state(engine);
  if (!after_t1.turn_ended) {
    for (const auto& line : trace.lines) std::cerr << line << '\n';
  }
  expect(after_t1.turn_ended, "Steven's Resolve must end T1 after the setup actions.");
  expect(after_t1.active && after_t1.active->grass == 1,
         "The T1 manual Grass attachment must occur before Steven.");
  expect(count_card(after_t1.discard, sim::Card::EarthenVessel) == 1,
         "Earthen Vessel must be played on T1.");
  expect(count_card(after_t1.discard, sim::Card::MysteriousTreasure) == 2,
         "Both Treasure copies must enter discard, one as Vessel cost and one as the played search Item.");
  expect(count_card(after_t1.discard, sim::Card::RoseannesBackup) == 1,
         "Roseanne's Backup must pay the surviving Treasure.");
  expect(count_card(after_t1.hand, sim::Card::RegidragoVstar) == 1,
         "Mysterious Treasure must search Regidrago VSTAR.");
  expect(count_card(after_t1.hand, sim::Card::Crispin) == 1 &&
         count_card(after_t1.hand, sim::Card::BrilliantBlender) == 1,
         "Steven must search the T2 Crispin and Blender package.");

  sim::EngineTestAccess::start_t2(engine);
  sim::EngineTestAccess::run_turn(engine);
  if (!sim::EngineTestAccess::ready(engine)) {
    for (const auto& line : trace.lines) std::cerr << line << '\n';
  }
  expect(sim::EngineTestAccess::ready(engine),
         "The preserved T1 attachment window must produce T2 readiness.");
}

void test_controls_do_not_force_vessel() {
  const auto blocked = [](sim::State state, const sim::Scenario scenario,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  sim::State one_treasure = route_state();
  one_treasure.hand.erase(std::find(one_treasure.hand.begin(), one_treasure.hand.end(),
                                   sim::Card::MysteriousTreasure));
  blocked(one_treasure, flex_second(), 123602,
          "A single Treasure cannot pay Vessel and still search VSTAR.");

  sim::State no_roseanne = route_state();
  no_roseanne.hand.erase(std::find(no_roseanne.hand.begin(), no_roseanne.hand.end(),
                                  sim::Card::RoseannesBackup));
  blocked(no_roseanne, flex_second(), 123603,
          "Missing Roseanne's Backup must block the two-cost projection.");

  sim::State no_fss = route_state();
  no_fss.hand.erase(std::find(no_fss.hand.begin(), no_fss.hand.end(),
                             sim::Card::ForestSealStone));
  blocked(no_fss, flex_second(), 123604,
          "Missing Forest Seal Stone must preserve ordinary Item ordering.");

  blocked(route_state(), flex_second(sim::LockMode::FullItem), 123605,
          "Item lock must block the route.");

  sim::State held_vstar = route_state();
  held_vstar.hand.push_back(sim::Card::RegidragoVstar);
  blocked(held_vstar, flex_second(), 123606,
          "A held VSTAR removes the need for the exact two-search sequence.");
}
}  // namespace

int main() {
  try {
    test_vessel_and_attachment_precede_steven();
    test_controls_do_not_force_vessel();
    std::cout << "issue 1236 tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
