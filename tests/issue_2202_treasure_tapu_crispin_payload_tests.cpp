#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {
bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void check(const char* scenario_label, const std::uint64_t seed,
           const int ready_turn, const char* payload) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const auto* deck = sim::deck_by_id("regidrago-shell");
  if (!scenario || !deck) {
    throw std::runtime_error("issue-2202 fixture unavailable");
  }

  std::mt19937_64 rng{seed};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Treasure's mandatory discard supplies the same-turn Apex Dragon payload,
  // then Tapu Lele-GX searches Crispin and Crispin completes the GF Active VSTAR.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dialga-GX and Dragapult ex: https://api.pokemontcg.io/v2/cards/sm5-100 https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, search, discard, Ability, Supporter, and Energy procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1/DCI/JIT/earliest-route specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Reclaimed confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2202
  const std::string payload_cost =
      std::string(payload) + " (Mysterious Treasure cost)";
  if (outcome.first_ready_turn != ready_turn || outcome.setup_failed ||
      !has(trace, payload_cost) || !has(trace, "WONDER TAG") ||
      !has(trace, "T" + std::to_string(ready_turn) + " | READY")) {
    throw std::runtime_error("issue-2202 route regression");
  }
}
}  // namespace

int main() {
  check("strict-jit/go-first", 1052, 3, "Dialga-GX");
  check("matchup-flex-jit/go-first", 691, 4, "Dragapult ex");
  return 0;
}
