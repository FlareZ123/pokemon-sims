#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_quick_ball(Engine& engine, const bool permit_payload) {
    return engine.play_quick_ball(permit_payload);
  }
  static bool play_mysterious_treasure(Engine& engine,
                                       const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State quick_ball_payload_state(const bool include_held_payload) {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::ForestSealStone};
  state.bench = {sim::Pokemon{sim::Card::Oricorio, 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                sim::Card::Fire, sim::Card::Grass, sim::Card::Grass};
  if (include_held_payload) state.hand.push_back(sim::Card::MegaDragonite);
  state.deck = {sim::Card::DialgaGX, sim::Card::TapuLeleGX,
                sim::Card::RegidragoV};
  return state;
}

void test_held_payload_suppresses_redundant_dialga_search() {
  const sim::Scenario scenario{"issue-1335-held-payload",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1335};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, quick_ball_payload_state(true));

  // Mysterious Treasure can discard the held Mega Dragonite ex after Fire is
  // manually attached. Quick Ball must preserve itself, its extra discard, and the
  // deck-resident Dialga-GX because a second payload advances no setup axis:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1335
  expect(!sim::EngineTestAccess::play_quick_ball(engine, false),
         "Quick Ball must hold when a public payload and direct outlet already finish the route.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::QuickBall),
         "The redundant Quick Ball must remain in hand.");
  expect(contains(result.hand, sim::Card::MegaDragonite),
         "The held payload must remain available for Mysterious Treasure.");
  expect(contains(result.deck, sim::Card::DialgaGX),
         "Dialga-GX must remain in deck when its search is redundant.");
}

void test_held_payload_suppresses_redundant_treasure_search() {
  const sim::Scenario scenario{"issue-1335-held-payload-treasure",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1337};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, quick_ball_payload_state(true));

  // A held payload plus Quick Ball already supplies a legal current-turn discard
  // outlet after Fire is attached. Mysterious Treasure must not fetch a second
  // Dragon before that direct route becomes playable:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/issues/1335
  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine, false),
         "Mysterious Treasure must hold when a public payload and direct outlet already finish the route.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::MysteriousTreasure),
         "The redundant Mysterious Treasure must remain in hand.");
  expect(contains(result.hand, sim::Card::MegaDragonite),
         "The held payload must remain available for the surviving outlet.");
}

void test_no_held_payload_preserves_quick_ball_dialga_bridge() {
  const sim::Scenario scenario{"issue-1335-no-held-payload",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1336};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, quick_ball_payload_state(false));

  // With no public payload, Quick Ball may still search Basic Dialga-GX for the
  // surviving Mysterious Treasure outlet. This is the required positive boundary:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://github.com/FlareZ123/pokemon-sims/issues/1335
  expect(sim::EngineTestAccess::play_quick_ball(engine, false),
         "Quick Ball-Dialga must remain live when no payload is held.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::DialgaGX),
         "Quick Ball must search Dialga-GX in the no-held-payload control.");
}

void test_seed_107_preserves_resources_at_same_ready_turn() {
  const sim::Scenario scenario{"strict-jit/go-first",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{107};
  sim::Engine engine(scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& state = engine.state();

  // Issue #1356 refines this exact witness by paying one surplus Grass to let
  // Mysterious Treasure search Regidrago VSTAR, while Star Alchemy supplies Fire.
  // The core #1335 invariant remains: preserve Quick Ball, Oricorio, the Bench slot,
  // and deck-resident Dialga-GX instead of manufacturing a redundant payload route:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1335
  // https://github.com/FlareZ123/pokemon-sims/issues/1356
  expect(outcome.first_ready_turn == 3,
         "The resource-preserving route must retain T3 readiness.");
  expect(contains(state.hand, sim::Card::QuickBall),
         "Seed 107 must preserve the redundant Quick Ball.");
  expect(state.bench.empty(),
         "Seed 107 must preserve the Bench slot instead of playing Oricorio.");
  expect(contains(state.deck, sim::Card::Oricorio),
         "Seed 107 must leave Oricorio in deck.");
  expect(contains(state.deck, sim::Card::DialgaGX),
         "Seed 107 must leave Dialga-GX in deck.");
  expect(contains(state.discard, sim::Card::MegaDragonite),
         "Seed 107 must still discard Mega Dragonite ex through Mysterious Treasure.");
}

}  // namespace

int main() {
  try {
    test_held_payload_suppresses_redundant_dialga_search();
    test_held_payload_suppresses_redundant_treasure_search();
    test_no_held_payload_preserves_quick_ball_dialga_bridge();
    test_seed_107_preserves_resources_at_same_ready_turn();
    std::cout << "Issue 1335 costed-search held-payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
