#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static bool route_available(const Engine& engine,
                              const std::vector<Card>& wanted,
                              const bool next_turn_item_locked,
                              const bool projected_vstar_available) {
    return engine.steven_held_treasure_crispin_grass_route_available(
        wanted, next_turn_item_locked, projected_vstar_available);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::State complete_route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {
      sim::Card::StevensResolve,
      sim::Card::MysteriousTreasure,
      sim::Card::MegaDragonite,
      sim::Card::LatiasEx,
  };
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::RegidragoV,
  };
  return state;
}

void test_complete_route_is_admitted() {
  const sim::Scenario scenario{"issue-1426-complete",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 2};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{142601};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, complete_route_state());
  const std::vector<sim::Card> wanted{sim::Card::RegidragoVstar};

  // Held Mysterious Treasure can discard held Mega Dragonite ex while another
  // legal Dragon target remains. Steven can therefore reserve VSTAR, Crispin,
  // and Grass instead of duplicating the payload axis with Brilliant Blender:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official rules: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1426
  expect(sim::EngineTestAccess::route_available(engine, wanted, false, true),
         "The complete held-Treasure route was not admitted.");
}

void test_incomplete_routes_keep_blender_fallback() {
  const std::vector<sim::Card> wanted{sim::Card::RegidragoVstar};
  const auto blocked = [&wanted](sim::State state, const sim::LockMode lock,
                                 const bool item_locked,
                                 const char* message, const std::uint64_t seed) {
    const sim::Scenario scenario{"issue-1426-negative",
                                 sim::DciProfile::StrictJit, lock, false, 2};
    const sim::DeckRecipe recipe = sim::baseline_recipe();
    std::mt19937_64 rng{seed};
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(
               engine, wanted, item_locked, true),
           message);
  };

  sim::State no_treasure = complete_route_state();
  no_treasure.hand.erase(std::find(no_treasure.hand.begin(),
                                   no_treasure.hand.end(),
                                   sim::Card::MysteriousTreasure));
  blocked(std::move(no_treasure), sim::LockMode::None, false,
          "A route without held Treasure was admitted.", 142602);

  sim::State no_payload = complete_route_state();
  no_payload.hand.erase(std::find(no_payload.hand.begin(), no_payload.hand.end(),
                                  sim::Card::MegaDragonite));
  blocked(std::move(no_payload), sim::LockMode::None, false,
          "A route without a held Dragon payload was admitted.", 142603);

  sim::State one_grass = complete_route_state();
  one_grass.deck.erase(std::find(one_grass.deck.begin(), one_grass.deck.end(),
                                 sim::Card::Grass));
  blocked(std::move(one_grass), sim::LockMode::None, false,
          "A route without two deck Grass Energy was admitted.", 142604);

  sim::State no_fire = complete_route_state();
  no_fire.deck.erase(std::find(no_fire.deck.begin(), no_fire.deck.end(),
                               sim::Card::Fire));
  blocked(std::move(no_fire), sim::LockMode::None, false,
          "A route without deck Fire Energy was admitted.", 142605);

  sim::State no_remaining_target = complete_route_state();
  no_remaining_target.deck.erase(
      std::remove(no_remaining_target.deck.begin(), no_remaining_target.deck.end(),
                  sim::Card::RegidragoV),
      no_remaining_target.deck.end());
  blocked(std::move(no_remaining_target), sim::LockMode::None, false,
          "A route without a post-Steven Treasure target was admitted.", 142606);

  blocked(complete_route_state(), sim::LockMode::TurnTwoItem, true,
          "A scheduled Item lock admitted the Treasure route.", 142607);
}

void test_registered_seed_44_reaches_t2() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered seed-44 scenario is unavailable.");

  std::mt19937_64 rng{44};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The source-bound K1 route must bank the missing Energy axis and preserve the
  // ACE SPEC because held Treasure already supplies the same-turn Dragon discard:
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1426
  expect(outcome.first_ready_turn == 2,
         "Seed 44 did not improve from setup failure to T2 readiness.");
  expect(trace_contains(trace,
                        "Regidrago VSTAR, Crispin, Grass Energy"),
         "Steven did not reserve the direct Energy package.");
  expect(!trace_contains(trace,
                         "Regidrago VSTAR, Crispin, Brilliant Blender"),
         "Steven still duplicated the held payload route with Blender.");
  expect(trace_contains(trace,
                        "Mega Dragonite ex (Mysterious Treasure cost)"),
         "Held Treasure did not supply the strict-JIT payload.");
  expect(trace_contains(trace, "T2 | READY"),
         "Seed 44 did not record T2 readiness.");
}
}  // namespace

int main() {
  try {
    test_complete_route_is_admitted();
    test_incomplete_routes_keep_blender_fallback();
    test_registered_seed_44_reaches_t2();
    std::cout << "Issue 1426 Steven Treasure-Grass tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
