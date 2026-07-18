#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool seen) { engine.deck_seen_ = seen; }
  static bool play_turo_active_promotion_route(Engine& engine) {
    return engine.play_turo_active_promotion_route();
  }
  static bool play_professor_burnet(Engine& engine) { return engine.play_professor_burnet(); }
  static bool setup_axis_missing(const Engine& engine) { return engine.setup_axis_missing(); }
  static State& mutable_state(Engine& engine) { return engine.state_; }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State delayed_payload_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::ProfessorTuro, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Grass};
  return state;
}

sim::Engine make_engine(const sim::LockMode lock_mode = sim::LockMode::TurnTwoItem,
                        const int max_turn = 4) {
  static std::mt19937_64 rng{900};
  static sim::DeckRecipe recipe = sim::baseline_recipe();
  static sim::Scenario scenario;
  scenario = sim::Scenario{"issue-900", sim::DciProfile::StrictJit,
                           lock_mode, false, max_turn};
  return sim::Engine(scenario, recipe, rng);
}

void test_turo_then_burnet_reaches_next_turn_readiness() {
  sim::Engine engine = make_engine();
  sim::EngineTestAccess::set_state(engine, delayed_payload_state());
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // Turo legally returns the Basic Active and forces promotion of the powered
  // Benched Regidrago VSTAR. Burnet remains held for the following turn, when its
  // deck-to-discard effect establishes the strict-JIT payload on the actual ready turn:
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/900
  expect(sim::EngineTestAccess::play_turo_active_promotion_route(engine),
         "Turo should admit the deterministic next-turn Burnet payload line.");
  const sim::State& promoted = sim::EngineTestAccess::state(engine);
  expect(promoted.active && promoted.active->card == sim::Card::RegidragoVstar &&
             promoted.active->grass == 2 && promoted.active->fire == 1,
         "Turo must promote the powered Benched Regidrago VSTAR.");
  expect(contains(promoted.hand, sim::Card::ProfessorBurnet) &&
             contains(promoted.hand, sim::Card::TapuLeleGX) &&
             contains(promoted.discard, sim::Card::ProfessorTuro),
         "Turo must preserve Burnet and return the Basic Active to hand.");

  sim::State& next_turn = sim::EngineTestAccess::mutable_state(engine);
  next_turn.turn = 4;
  next_turn.supporter_used = false;
  next_turn.discarded_this_turn.clear();
  expect(sim::EngineTestAccess::play_professor_burnet(engine),
         "Burnet should discard the searchable Dragon payload on turn four.");
  expect(!sim::EngineTestAccess::setup_axis_missing(engine),
         "The Turo-then-Burnet sequence should satisfy every setup axis by turn four.");
}

void test_route_requires_held_burnet() {
  sim::Engine engine = make_engine();
  sim::State state = delayed_payload_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::ProfessorBurnet));
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);
  expect(!sim::EngineTestAccess::play_turo_active_promotion_route(engine),
         "Turo must not postpone a missing payload without held Burnet.");
}

void test_route_requires_a_remaining_turn() {
  sim::Engine engine = make_engine(sim::LockMode::TurnTwoItem, 3);
  sim::EngineTestAccess::set_state(engine, delayed_payload_state());
  sim::EngineTestAccess::set_deck_seen(engine, true);
  expect(!sim::EngineTestAccess::play_turo_active_promotion_route(engine),
         "Turo must not consume the final modeled turn before Burnet can resolve.");
}

void test_route_requires_item_lock() {
  sim::Engine engine = make_engine(sim::LockMode::None, 4);
  sim::EngineTestAccess::set_state(engine, delayed_payload_state());
  sim::EngineTestAccess::set_deck_seen(engine, true);
  expect(!sim::EngineTestAccess::play_turo_active_promotion_route(engine),
         "The delayed exception must stay scoped to the confirmed Item-lock route.");
}

void test_route_requires_searchable_payload() {
  sim::Engine engine = make_engine();
  sim::State state = delayed_payload_state();
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);
  expect(!sim::EngineTestAccess::play_turo_active_promotion_route(engine),
         "Turo must preserve the state when Burnet has no payload target.");
}

}  // namespace

int main() {
  test_turo_then_burnet_reaches_next_turn_readiness();
  test_route_requires_held_burnet();
  test_route_requires_a_remaining_turn();
  test_route_requires_item_lock();
  test_route_requires_searchable_payload();
  std::cout << "Turo delayed-Burnet payload tests passed\n";
}
