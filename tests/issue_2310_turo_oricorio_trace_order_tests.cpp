#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }

  static bool play_route(Engine& engine) {
    return engine.play_turo_oricorio_energy_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::size_t trace_index(const sim::TraceLog& trace, const std::string& text) {
  for (std::size_t index = 0; index < trace.lines.size(); ++index) {
    if (trace.lines[index].find(text) != std::string::npos) return index;
  }
  return trace.lines.size();
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::Oricorio, 1}};
  state.hand = {sim::Card::ProfessorTuro};
  state.deck = {sim::Card::Fire, sim::Card::Grass};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  return state;
}

void test_turo_records_before_oricorio_replay_and_energy_resolution() {
  const sim::Scenario scenario{"issue-2310", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(2310);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, route_state());

  expect(sim::EngineTestAccess::play_route(engine),
         "The source-bounded Professor Turo and Oricorio route must resolve.");

  // Professor Turo returns Oricorio to hand before that Basic can be replayed.
  // Vital Dance then triggers from the hand-to-Bench play, and the searched Energy
  // can subsequently be used for the turn's manual attachment. Readable history
  // must therefore preserve PLAY SUPPORTER -> BENCH -> VITAL DANCE -> ATTACH.
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Core Supporter, Bench, Ability, search, and Energy-attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Underlying Turo-Oricorio route: https://github.com/FlareZ123/pokemon-sims/issues/267
  // Prior Tapu replay chronology precedent: https://github.com/FlareZ123/pokemon-sims/issues/2244
  // Readable-trace contract: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
  // Confirmed Oricorio chronology bug: https://github.com/FlareZ123/pokemon-sims/issues/2310
  const std::size_t supporter = trace_index(
      trace, "Professor Turo returned Oricorio to hand for the Active-specific final Energy route.");
  const std::size_t bench = trace_index(trace, "Oricorio GRI 55 from hand.");
  const std::size_t vital_dance = trace_index(
      trace, "Searched the exact Basic Energy required by the Active Regidrago VSTAR.");
  const std::size_t attach = trace_index(
      trace, "Manually attached the Vital Dance Energy to the Active Regidrago VSTAR.");

  expect(supporter < trace.lines.size(),
         "The route did not record the Professor Turo Supporter action.");
  expect(bench < trace.lines.size(),
         "The route did not record the replayed Oricorio Bench action.");
  expect(vital_dance < trace.lines.size(),
         "The route did not record the replayed Oricorio Vital Dance action.");
  expect(attach < trace.lines.size(),
         "The route did not record the searched Energy attachment.");
  expect(supporter < bench && bench < vital_dance && vital_dance < attach,
         "The trace did not preserve PLAY SUPPORTER -> BENCH -> VITAL DANCE -> ATTACH chronology.");
}
}  // namespace

int main() {
  try {
    test_turo_records_before_oricorio_replay_and_energy_resolution();
    std::cout << "Issue 2310 Professor Turo Oricorio trace-order tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
