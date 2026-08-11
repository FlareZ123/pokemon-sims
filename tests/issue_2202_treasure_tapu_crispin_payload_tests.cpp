#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static std::optional<Card> payload_cost(const Engine& engine) {
    return engine.issue_2202_treasure_tapu_crispin_payload_cost();
  }
};
}  // namespace sim

namespace {
bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
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

void test_issue_3129_proven_second_tapu_copy() {
  const sim::Scenario scenario{"issue-3129-2202", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{312920};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  // Pokemon aggregate order is card, entered_turn, grass, fire, tool, DDE.
  // This is the intended GF Active VSTAR before Crispin supplies the second Grass.
  // State contract: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/game_state_types.inc
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 0}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::DialgaGX};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, state, true);

  // Wonder Tag belongs to the new physical Tapu Lele-GX played from hand. K1
  // proves a second copy remains in the deck even though one copy is already in play.
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Bench, Ability, Item, Supporter, and search procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Shared copy-aware rule: https://github.com/FlareZ123/pokemon-sims/issues/746
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/3129
  expect(sim::EngineTestAccess::payload_cost(engine) == sim::Card::DialgaGX,
         "#2202 rejected the K1-proven second Tapu Lele-GX route.");

  state.deck.erase(state.deck.begin());
  sim::EngineTestAccess::set_state(engine, state, true);
  expect(!sim::EngineTestAccess::payload_cost(engine).has_value(),
         "#2202 invented a second Tapu Lele-GX after K1 proved none remained.");
}
}  // namespace

int main() {
  check("strict-jit/go-first", 1052, 3, "Dialga-GX");
  check("matchup-flex-jit/go-first", 691, 4, "Dragapult ex");
  test_issue_3129_proven_second_tapu_copy();
  return 0;
}