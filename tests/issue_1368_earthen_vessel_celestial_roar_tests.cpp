#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool deck_seen(const Engine& engine) { return engine.deck_seen_; }
  static bool play_earthen_vessel(Engine& engine) {
    return engine.play_earthen_vessel(false);
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

sim::State exact_celestial_roar_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                sim::Card::Lusamine, sim::Card::Grass, sim::Card::Grass,
                sim::Card::EarthenVessel, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Fire, sim::Card::Fire, sim::Card::Fire,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::QuickBall,
                sim::Card::MysteriousTreasure, sim::Card::Crispin};
  return state;
}

void test_searches_only_fire_before_immediate_celestial_roar() {
  const sim::Scenario scenario{"issue-1368-exact",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 3};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1368};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_celestial_roar_state());

  // Earthen Vessel says "up to 2" Basic Energy. Two held Grass already cover
  // the T1 manual attachment and the complementary T2 attachment after any
  // Celestial Roar Energy hit, so only Fire should leave the deck:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1368
  expect(sim::EngineTestAccess::play_earthen_vessel(engine),
         "The exact Earthen Vessel route must resolve.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(count(result.hand, sim::Card::Fire) == 1,
         "Earthen Vessel must search the missing Fire Energy.");
  expect(count(result.hand, sim::Card::Grass) == 2,
         "Earthen Vessel must leave both already-held Grass Energy in hand.");
  expect(count(result.deck, sim::Card::Grass) == 4,
         "The surplus Grass must remain in deck for Celestial Roar density.");
  expect(count(result.deck, sim::Card::Fire) == 2,
         "Exactly one Fire Energy must leave the deck.");
  expect(sim::EngineTestAccess::deck_seen(engine),
         "Resolving Earthen Vessel must establish K1 deck knowledge.");
}

void test_going_first_keeps_normal_two_target_search() {
  const sim::Scenario scenario{"issue-1368-going-first-control",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 3};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1369};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = exact_celestial_roar_state();
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The first player cannot attack on turn one, so the immediate Celestial Roar
  // density override must stay inactive and the ordinary up-to-two search remains:
  // First-turn attack rule: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1368
  expect(sim::EngineTestAccess::play_earthen_vessel(engine),
         "The going-first control must still resolve Earthen Vessel.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(count(result.hand, sim::Card::Grass) +
             count(result.hand, sim::Card::Fire) == 4,
         "The ordinary selector must add two Basic Energy in the control.");
}

void test_single_held_grass_keeps_normal_two_target_search() {
  const sim::Scenario scenario{"issue-1368-one-grass-control",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 3};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1370};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = exact_celestial_roar_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Grass));
  state.hand.push_back(sim::Card::QuickBall);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // With only one held Grass, the second search target still advances the
  // deterministic attachment schedule, so the ordinary two-target route remains:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(sim::EngineTestAccess::play_earthen_vessel(engine),
         "The one-Grass control must still resolve Earthen Vessel.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(count(result.hand, sim::Card::Grass) +
             count(result.hand, sim::Card::Fire) == 3,
         "The ordinary selector must add two Basic Energy when the second advances the schedule.");
}

}  // namespace

int main() {
  try {
    test_searches_only_fire_before_immediate_celestial_roar();
    test_going_first_keeps_normal_two_target_search();
    test_single_held_grass_keeps_normal_two_target_search();
    std::cout << "Issue 1368 Earthen Vessel Celestial Roar tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
