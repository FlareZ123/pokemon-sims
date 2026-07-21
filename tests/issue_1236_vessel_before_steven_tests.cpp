#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool route_available(const Engine& engine) {
    return engine.issue_1236_vessel_steven_compression_available();
  }
  static bool search_step(Engine& engine) {
    return engine.run_search_items_one_step(false);
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

sim::Scenario scenario(const sim::LockMode locks = sim::LockMode::None,
                       const bool going_first = false) {
  return sim::Scenario{"issue-1236", sim::DciProfile::MatchupFlexJit,
                       locks, going_first, 4};
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::LatiasEx, sim::Card::RoseannesBackup,
                sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure,
                sim::Card::GoodraVstar, sim::Card::EarthenVessel,
                sim::Card::ForestSealStone};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::StevensResolve, sim::Card::Crispin,
                sim::Card::BrilliantBlender, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::DialgaGX, sim::Card::Dipplin,
                sim::Card::Gladion};
  return state;
}

sim::Engine make_engine(const sim::Scenario& selected, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = false) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(selected, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto it = std::find(cards.begin(), cards.end(), card);
  if (it != cards.end()) cards.erase(it);
}

void test_vessel_precedes_vstar_search() {
  std::mt19937_64 rng{1236001};
  sim::Engine engine = make_engine(scenario(), rng, route_state());

  // The duplicate Treasure pays Vessel while its surviving copy pays its own cost
  // with Roseanne's Backup and searches Regidrago VSTAR. Grass is available for the
  // T1 manual attachment before Steven ends the turn:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1236
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact Vessel-Steven compression route must be available.");
  expect(sim::EngineTestAccess::search_step(engine),
         "The first search step must play Earthen Vessel.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count_card(after.discard, sim::Card::EarthenVessel) == 1,
         "Earthen Vessel must be played first.");
  expect(count_card(after.discard, sim::Card::MysteriousTreasure) == 1,
         "One duplicate Treasure must pay Vessel's cost.");
  expect(count_card(after.hand, sim::Card::MysteriousTreasure) == 1,
         "The second Treasure must remain for the VSTAR search.");
  expect(count_card(after.hand, sim::Card::Grass) == 1 &&
             count_card(after.hand, sim::Card::Fire) == 1,
         "Vessel must establish both Energy channels.");
}

void test_seed_307_reaches_t2() {
  std::mt19937_64 rng{307};
  sim::TraceLog trace;
  trace.enabled = true;
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario(), recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The deterministic reproduction attaches Grass before Steven ends T1, then uses
  // Crispin, manual Fire, evolution, and Brilliant Blender to finish on T2:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1236
  expect(outcome.ready_by_2,
         "The seed-307 reproduction must reach readiness on T2.");
  const auto position = [&trace](const std::string& needle) {
    const auto it = std::find_if(trace.lines.begin(), trace.lines.end(),
        [&needle](const std::string& line) {
          return line.find(needle) != std::string::npos;
        });
    return it == trace.lines.end() ? trace.lines.size()
                                   : static_cast<std::size_t>(it - trace.lines.begin());
  };
  expect(position("Earthen Vessel cost") < position("Mysterious Treasure cost"),
         "Vessel must precede the VSTAR search.");
  expect(position("Grass Energy manually") < position("PLAY SUPPORTER"),
         "The Grass attachment must precede Steven's resolution.");
}

void test_incomplete_routes_hold() {
  const auto blocked = [](sim::State state, const sim::Scenario selected,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(selected, rng, std::move(state));
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  sim::State one_treasure = route_state();
  erase_one(one_treasure.hand, sim::Card::MysteriousTreasure);
  blocked(one_treasure, scenario(), 1236003,
          "A singleton Treasure must remain protected.");

  sim::State no_roseanne = route_state();
  erase_one(no_roseanne.hand, sim::Card::RoseannesBackup);
  blocked(no_roseanne, scenario(), 1236004,
          "Missing the second printed discard cost must block the route.");

  sim::State no_fss = route_state();
  erase_one(no_fss.hand, sim::Card::ForestSealStone);
  blocked(no_fss, scenario(), 1236005,
          "Missing Forest Seal Stone must block Steven access.");

  sim::State no_grass = route_state();
  no_grass.deck.erase(
      std::remove(no_grass.deck.begin(), no_grass.deck.end(), sim::Card::Grass),
      no_grass.deck.end());
  {
    std::mt19937_64 rng{1236006};
    sim::Engine engine = make_engine(scenario(), rng, std::move(no_grass), true);
    expect(!sim::EngineTestAccess::route_available(engine),
           "Known missing Grass must block the route.");
  }

  sim::State held_vstar = route_state();
  held_vstar.hand.push_back(sim::Card::RegidragoVstar);
  blocked(held_vstar, scenario(), 1236007,
          "The exception must stay limited to a missing VSTAR axis.");

  blocked(route_state(), scenario(sim::LockMode::FullItem), 1236008,
          "Item lock must block the route.");
  blocked(route_state(), scenario(sim::LockMode::None, true), 1236009,
          "Going first must block the T1 Steven route.");
  blocked(route_state(),
          sim::Scenario{"issue-1236-strict", sim::DciProfile::StrictJit,
                        sim::LockMode::None, false, 4},
          1236010, "The matchup-flex route must not alter strict-JIT policy.");
}
}  // namespace

int main() {
  try {
    test_vessel_precedes_vstar_search();
    test_seed_307_reaches_t2();
    test_incomplete_routes_hold();
    std::cout << "issue 1236 Vessel-before-Steven tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
