#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {

struct EngineTestAccess {};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

void verify_modeling_seed_218() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat1-heavy-ball");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1700 modeling setup is unavailable.");

  std::mt19937_64 rng{218};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Vessel may discard the held Dragon, search Grass and Fire, and attach Grass
  // before Steven ends T1. Steven then searches Latias ex and the second Grass.
  // The prior-turn Regidrago evolves and retreats on T2, then Fire plus Blender
  // establishes the matchup-flex current-turn payload and T3 ready state:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1700
  expect(outcome.first_ready_turn == 3,
         "Modeling seed 218 must complete the deterministic T3 route.");
  expect(trace_contains(trace,
                        "Earthen Vessel searched Grass and Fire before Steven's Resolve") &&
             trace_contains(trace,
                            "Grass Energy manually to Regidrago V before Steven's Resolve") &&
             trace_contains(trace,
                            "Searched the complete post-Vessel T3 route: Latias ex, Grass Energy") &&
             trace_contains(trace, "T3 | READY"),
         "Modeling seed 218 did not preserve the complete post-Vessel route.");
}

void verify_registered_seed_218(const std::string& label) {
  const auto scenario = sim::scenario_by_label(label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1700 registered setup is unavailable.");

  std::mt19937_64 rng{218};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // All Fire Energy is known prized after Wonder Tag. Vessel searches two Grass and
  // makes the first attachment before Steven. Steven preserves the missing VSTAR axis
  // when its held copy paid the strict-JIT cost, then searches Latias ex and Gladion.
  // Gladion recovers Fire on T2 and the final attachment plus Blender reaches T3:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1700
  expect(outcome.first_ready_turn == 3,
         "Registered seed 218 must complete the deterministic T3 route.");
  expect(trace_contains(trace,
                        "Earthen Vessel searched two Grass before Steven's Resolve; Fire is known prized") &&
             trace_contains(trace, "Gladion") &&
             trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-GLADION-01") &&
             trace_contains(trace, "T3 | READY"),
         "Registered seed 218 did not preserve the Prize-recovery route.");
}

void verify_lock_control() {
  const auto scenario =
      sim::scenario_by_label("strict-jit-full-item-lock/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1700 lock control is unavailable.");

  std::mt19937_64 rng{218};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  static_cast<void>(engine.run());
  expect(!trace_contains(trace, "post-Vessel T3 route"),
         "The override must not play Earthen Vessel through Item lock.");
}

}  // namespace

int main() {
  try {
    verify_modeling_seed_218();
    verify_registered_seed_218("strict-jit/go-second");
    verify_registered_seed_218("matchup-flex-jit/go-second");
    verify_registered_seed_218("no-discard-control/go-second");
    verify_lock_control();
    std::cout << "Issue 1700 pre-Steven Vessel route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
