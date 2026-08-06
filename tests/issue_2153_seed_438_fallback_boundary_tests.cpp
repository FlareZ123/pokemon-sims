#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {

struct EngineTestAccess {
  static bool used_issue_2153_route(const TraceLog& trace) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [](const std::string& line) {
                         return line.find("deterministic T3 Latias route") !=
                                std::string::npos;
                       });
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void seed_438_does_not_invent_prized_latias() {
  const auto selected = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value(), "Missing strict-JIT going-second scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{438};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected, deck->recipe, rng, &trace};
  static_cast<void>(engine.run());

  const bool latias_is_known_prized = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("DEBUG ONLY: prizes=") != std::string::npos &&
            line.find("Latias ex") != std::string::npos;
      });
  expect(latias_is_known_prized,
         "Seed 438 no longer proves Latias ex is prized");
  expect(!sim::EngineTestAccess::used_issue_2153_route(trace),
         "The issue-2153 route invented a prized Latias ex");

  // Seed 438 is a negative boundary for the stronger Latias package. A positive
  // Tate package is legal only when three Steven targets cover every remaining
  // evolution, Energy, promotion, and strict-JIT payload axis. The later actual
  // Fire draw instead exposes the separately owned paid-retreat defect in #2158:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Oricorio Retreat Cost: https://api.pokemontcg.io/v2/cards/sm2-55
  // Official search, attachment, Retreat, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Observable-information and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original target-construction bug: https://github.com/FlareZ123/pokemon-sims/issues/2153
  // Separately claimed paid-retreat route: https://github.com/FlareZ123/pokemon-sims/issues/2158
}

}  // namespace

int main() {
  try {
    seed_438_does_not_invent_prized_latias();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
