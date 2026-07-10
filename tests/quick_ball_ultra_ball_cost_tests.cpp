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
  static bool play_quick_ball(Engine& engine, const bool permit_payload = false) {
    return engine.play_quick_ball(permit_payload);
  }
  static bool play_earthen_vessel(Engine& engine) { return engine.play_earthen_vessel(true); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_quick_ball_can_discard_a_distinct_ultra_ball() {
  // Quick Ball discards another card from hand before it searches for a Basic Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago V is a Basic Pokémon and is therefore a legal Quick Ball target:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  sim::Scenario scenario{"quick-ball-ultra-ball-cost", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::UltraBall};
  state.deck = {sim::Card::RegidragoV};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{137};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball should discard the distinct Ultra Ball and search Regidrago V.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::RegidragoV),
         "Quick Ball should put the Basic Regidrago V into hand.");
  expect(contains(result.discard, sim::Card::QuickBall) && contains(result.discard, sim::Card::UltraBall),
         "Quick Ball and its distinct Ultra Ball discard cost should both enter discard.");
}

void test_quick_ball_holds_when_k1_payload_is_evolution_only() {
  const sim::Scenario scenario{"quick-ball-evolution-only-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::Dipplin, sim::Card::EarthenVessel};
  state.deck = {sim::Card::MegaDragonite};
  state.supporter_used = true;

  // Quick Ball searches only a Basic Pokémon, while Mega Dragonite ex is Stage 2:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{195};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  expect(!sim::EngineTestAccess::play_quick_ball(engine, true),
         "Quick Ball should be held when K1 proves the only payload is an Evolution Pokémon.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::QuickBall) && contains(result.hand, sim::Card::Dipplin),
         "The dead payload-search attempt must not spend Quick Ball or its discard cost.");
  expect(contains(result.deck, sim::Card::MegaDragonite),
         "The evolution-only payload should remain in deck for a legal search effect.");
}

void test_quick_ball_fetches_basic_dialga_payload() {
  const sim::Scenario scenario{"quick-ball-basic-dialga-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::Dipplin, sim::Card::EarthenVessel};
  state.deck = {sim::Card::DialgaGX, sim::Card::Grass, sim::Card::Fire};
  state.supporter_used = true;

  // Dialga-GX is a Basic Pokémon, so Quick Ball may fetch it. Earthen Vessel can
  // then discard the fetched card as its other-card cost in the same turn:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sv4-163
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{196};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  expect(sim::EngineTestAccess::play_quick_ball(engine, true),
         "Quick Ball should fetch the modeled Basic Dragon payload Dialga-GX.");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::DialgaGX),
         "Dialga-GX should enter hand from Quick Ball's Basic Pokémon search.");
  expect(sim::EngineTestAccess::play_earthen_vessel(engine),
         "Earthen Vessel should discard the fetched Dialga-GX in the current turn.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::DialgaGX) &&
             contains(result.discarded_this_turn, sim::Card::DialgaGX),
         "The fetched Dialga-GX should become the current-turn Dragon payload.");
  // Apex Dragon uses an attack from a Dragon Pokémon in discard:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Quick Ball into Earthen Vessel should satisfy strict-JIT payload timing.");
}

}  // namespace

int main() {
  try {
    test_quick_ball_can_discard_a_distinct_ultra_ball();
    test_quick_ball_holds_when_k1_payload_is_evolution_only();
    test_quick_ball_fetches_basic_dialga_payload();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
