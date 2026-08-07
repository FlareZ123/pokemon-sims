#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static bool play_route(Engine& engine) {
    return engine.play_turo_tapu_future_supporter_route();
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
  state.turn = 3;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 2, 2, 0, sim::Tool::None}};
  state.hand = {sim::Card::ProfessorTuro, sim::Card::Fire,
                sim::Card::GoodraVstar, sim::Card::Lusamine};
  state.deck = {sim::Card::Gladion, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Arven};
  return state;
}

void test_turo_records_before_tapu_replay() {
  const sim::Scenario scenario{"issue-2244", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(2244);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, route_state(), true, true);

  expect(sim::EngineTestAccess::play_route(engine),
         "The source-bounded Professor Turo and Tapu Lele-GX route must resolve.");

  // Professor Turo finishes returning the Active Tapu Lele-GX and choosing the
  // replacement Active before Tapu can be played from hand to the Bench. Wonder
  // Tag then triggers from that later Bench play, so readable history must preserve
  // PLAY SUPPORTER -> BENCH -> WONDER TAG. Match stable action details because the
  // public trace schema inserts a `rules:` field between action and description.
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Gladion searched by Wonder Tag: https://api.pokemontcg.io/v2/cards/sm4-95
  // Core Supporter, Active replacement, Bench, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Stable state fixture precedent: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_1165_turo_tapu_replay_tests.cpp
  // Readable-trace contract and `rules:` field: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
  // Confirmed chronology bug: https://github.com/FlareZ123/pokemon-sims/issues/2244
  const std::size_t supporter = trace_index(
      trace, "Professor Turo returned Tapu Lele-GX and promoted the established Regidrago V.");
  const std::size_t bench = trace_index(trace, "Tapu Lele-GX from hand.");
  const std::size_t wonder_tag = trace_index(trace, "Searched and revealed Gladion.");

  expect(supporter < trace.lines.size(),
         "The route did not record the Professor Turo Supporter action.");
  expect(bench < trace.lines.size(),
         "The route did not record the replayed Tapu Lele-GX Bench action.");
  expect(wonder_tag < trace.lines.size(),
         "The route did not record the replayed Tapu Lele-GX Wonder Tag action.");
  expect(supporter < bench && bench < wonder_tag,
         "The trace did not preserve PLAY SUPPORTER -> BENCH -> WONDER TAG chronology.");
}
}  // namespace

int main() {
  try {
    test_turo_records_before_tapu_replay();
    std::cout << "Issue 2244 Professor Turo trace-order tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
