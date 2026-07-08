#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void begin_turn(Engine& engine, const int turn) { engine.begin_turn(turn); }
};

}  // namespace sim

namespace {

void test_empty_deck_at_turn_start_ends_a_turn_with_a_required_draw() {
  const sim::Scenario scenario{"start-turn-deck-exhaustion", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{118};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);

  sim::State state;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Crispin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A player loses when the required beginning-of-turn draw cannot be made:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::begin_turn(engine, 2);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.turn_ended) {
    throw std::runtime_error("An empty deck on a drawing turn must end the simulated game before actions.");
  }
  if (after.supporter_used || after.manual_energy_used || after.hand.size() != 1U) {
    throw std::runtime_error("The required-draw deck loss must not consume or permit a turn action.");
  }
  if (trace.lines.empty() || trace.lines.back().find("GAME LOST") == std::string::npos) {
    throw std::runtime_error("The required-draw deck loss must be visible in the trace.");
  }
}

void test_empty_deck_does_not_lose_when_going_first_skips_the_draw() {
  const sim::Scenario scenario{"first-turn-no-draw-deck-exhaustion", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{119};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);

  sim::State state;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Crispin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The player going first does not draw at the beginning of their first turn:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::begin_turn(engine, 1);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.turn_ended) {
    throw std::runtime_error("The skipped opening-turn draw must not cause a deck-out loss.");
  }
  if (after.supporter_used || after.manual_energy_used || after.hand.size() != 1U) {
    throw std::runtime_error("Skipping the opening draw must not consume a turn action or change the hand.");
  }
  if (trace.lines.empty() || trace.lines.back().find("TURN START") == std::string::npos ||
      trace.lines.back().find("GAME LOST") != std::string::npos) {
    throw std::runtime_error("The skipped opening draw must be visible without a deck-loss trace.");
  }
}

}  // namespace

int main() {
  try {
    test_empty_deck_at_turn_start_ends_a_turn_with_a_required_draw();
    test_empty_deck_does_not_lose_when_going_first_skips_the_draw();
    std::cout << "start-turn deck exhaustion tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
