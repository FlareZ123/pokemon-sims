import os
from pathlib import Path


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


base_path = Path(
    "src/trace_engine_v2/part_issue_991_wonder_tag_burnet_legacy_star_override_base.inc"
)
base = base_path.read_text(encoding="utf-8")
old_axis = r'''    const bool held_gladion_has_known_axis = hand_count(Card::Gladion) > 0 &&
        ((need_regi() && bench_space() > 0 &&
          prize_count_after_reveal(Card::RegidragoV) > 0) ||
         (need_vstar() && prize_count_after_reveal(Card::RegidragoVstar) > 0) ||
         prize_count_after_reveal(Card::Grass) > 0 ||
         prize_count_after_reveal(Card::Fire) > 0);
'''
new_axis = r'''    const bool known_prized_payload_axis =
        scenario_.dci == DciProfile::NoDiscardControl &&
        std::any_of(
            std::array{Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
                       Card::GoodraVstar, Card::Appletun}.begin(),
            std::array{Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
                       Card::GoodraVstar, Card::Appletun}.end(),
            [this](const Card card) {
              return prize_count_after_reveal(card) > 0;
            });
    const bool held_gladion_has_known_axis = hand_count(Card::Gladion) > 0 &&
        ((need_regi() && bench_space() > 0 &&
          prize_count_after_reveal(Card::RegidragoV) > 0) ||
         (need_vstar() && prize_count_after_reveal(Card::RegidragoVstar) > 0) ||
         prize_count_after_reveal(Card::Grass) > 0 ||
         prize_count_after_reveal(Card::Fire) > 0 ||
         known_prized_payload_axis);
'''
if base.count(old_axis) != 1:
    raise SystemExit("Expected one duplicate-Crispin Gladion-axis block")
base = base.replace(old_axis, new_axis, 1)
old_comment = r'''    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // One Supporter per turn: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
'''
new_comment = r'''    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Accepted Apex Dragon payloads: https://api.pokemontcg.io/v2/cards/sv6pt5-130 https://api.pokemontcg.io/v2/cards/me1-132 https://api.pokemontcg.io/v2/cards/sm5-100 https://api.pokemontcg.io/v2/cards/swsh11-136 https://api.pokemontcg.io/v2/cards/sv8-140
    // One Supporter per turn: https://www.pokemon.com/us/pokemon-tcg/rules
    // No-control payload banking and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed payload-axis extension: https://github.com/FlareZ123/pokemon-sims/issues/1516
'''
if base.count(old_comment) != 1:
    raise SystemExit("Expected one duplicate-Crispin source comment block")
atomic_write(base_path, base.replace(old_comment, new_comment, 1))

source_path = Path("src/regidrago_sim.cpp")
source = source_path.read_text(encoding="utf-8")
old_include = '#include "trace_engine_v2/part_issue_1476_redundant_burnet_route_override.inc"'
new_include = (
    '#define play_quick_ball play_quick_ball_issue1516_original\n'
    '#include "trace_engine_v2/part_issue_1476_redundant_burnet_route_override.inc"\n'
    '#undef play_quick_ball\n'
    '#include "trace_engine_v2/part_issue_1516_quick_ball_tapu_crispin_override.inc"'
)
if source.count(old_include) != 1:
    raise SystemExit("Expected one final issue-1476 include anchor")
atomic_write(source_path, source.replace(old_include, new_include, 1))

override = r'''  bool issue_1516_quick_ball_tapu_duplicate_crispin_is_redundant(
      const bool permit_payload) const {
    if (!deck_seen_ || !prizes_known() ||
        scenario_.dci != DciProfile::NoDiscardControl ||
        hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        hand_count(Card::Gladion) == 0 ||
        !need_energy()) {
      return false;
    }

    const bool known_prized_payload = std::any_of(
        std::array{Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
                   Card::GoodraVstar, Card::Appletun}.begin(),
        std::array{Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
                   Card::GoodraVstar, Card::Appletun}.end(),
        [this](const Card card) {
          return prize_count_after_reveal(card) > 0;
        });
    if (!known_prized_payload) return false;

    std::mt19937_64 projected_rng = rng_;
    Engine projected(scenario_, recipe_, projected_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    const int tapu_before = projected.hand_count(Card::TapuLeleGX);
    if (!projected.play_quick_ball_issue1516_original(permit_payload) ||
        projected.hand_count(Card::TapuLeleGX) <= tapu_before) {
      return false;
    }

    // Quick Ball pays one discard and searches one Basic Pokemon. The projection
    // executes that exact final selector before suppressing the real play. When it
    // selects Tapu Lele-GX and the established duplicate-Crispin guard sees held
    // Gladion covering a known no-control payload Prize, the searched Supporter
    // adds no earlier setup axis and every physical resource should be preserved:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // One Supporter per turn and Item cost procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and resource-preserving route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Prior duplicate-Crispin guard: https://github.com/FlareZ123/pokemon-sims/issues/1038
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1516
    return projected.wonder_tag_duplicate_held_crispin_has_no_marginal_route();
  }

  bool play_quick_ball(const bool permit_payload) {
    if (issue_1516_quick_ball_tapu_duplicate_crispin_is_redundant(
            permit_payload)) {
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1516-qb-tapu-duplicate-crispin", state_.turn,
            "HOLD QUICK BALL",
            "R-QB-01; R-TAPU-01; R-CRISPIN-01; R-GLADION-01; R-GAME-SUPPORTER; P-CONNECTOR-01; P-KNOWLEDGE-01",
            "Preserved Quick Ball, its discard, Tapu Lele-GX, Wonder Tag, and the Bench slot because held Gladion covers the known no-control payload Prize and held Crispin covers the later Energy turn.");
      }
      return false;
    }
    return play_quick_ball_issue1516_original(permit_payload);
  }
'''
atomic_write(
    Path("src/trace_engine_v2/part_issue_1516_quick_ball_tapu_crispin_override.inc"),
    override,
)

tests = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static const State& state(const Engine& engine) {
    return engine.state_;
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

struct SeedResult {
  sim::TrialOutcome outcome;
  sim::TraceLog trace;
};

SeedResult run_seed(const std::string& scenario_label,
                    const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1516 fixture is unavailable.");
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}

void test_seed_42_preserves_quick_ball_and_tapu() {
  const SeedResult result =
      run_seed("no-discard-control/go-first", 42);

  // Earthen Vessel has already established K1 and supplied Grass plus Fire.
  // Held Gladion covers the known prized accepted payload, while held Crispin
  // remains available for the Energy Supporter turn. Quick Ball into Tapu and
  // duplicate Crispin therefore spends six resources without improving T3:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // No-control policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1516
  expect(result.outcome.first_ready_turn == 3 &&
             !result.outcome.setup_failed,
         "Seed 42 lost its legal T3 ready turn.");
  expect(trace_contains(result.trace, "T1 | HOLD QUICK BALL"),
         "Seed 42 did not preserve the redundant Quick Ball route.");
  expect(!trace_contains(result.trace,
                         "T1 | PLAY ITEM | Searched a Basic Pokémon: Tapu Lele-GX") &&
             !trace_contains(result.trace,
                             "T1 | BENCH | Tapu Lele-GX from hand.") &&
             !trace_contains(result.trace, "T1 | WONDER TAG"),
         "Seed 42 still spent Quick Ball and Tapu on duplicate Crispin.");
  expect(trace_contains(result.trace, "T2 | PLAY SUPPORTER") &&
             trace_contains(result.trace, "Gladion") &&
             trace_contains(result.trace, "T3 | READY"),
         "Seed 42 did not retain the Gladion-to-Crispin T3 continuation.");
}

void test_strict_jit_seed_104_keeps_distinct_tapu_route() {
  const SeedResult result = run_seed("strict-jit/go-first", 104);

  // Under strict JIT, Quick Ball into Tapu Lele-GX supplies a distinct Crispin
  // connector before Earthen Vessel. The no-control payload guard cannot apply:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Existing route: https://github.com/FlareZ123/pokemon-sims/issues/962
  expect(result.outcome.first_ready_turn == 3 &&
             !result.outcome.setup_failed,
         "Strict-JIT seed 104 lost its T3 route.");
  expect(trace_contains(result.trace,
                        "Searched a Basic Pokémon: Tapu Lele-GX") &&
             trace_contains(result.trace, "WONDER TAG") &&
             trace_contains(result.trace, "Crispin"),
         "Strict-JIT seed 104 lost its distinct Tapu-Crispin connector.");
}

}  // namespace

int main() {
  test_seed_42_preserves_quick_ball_and_tapu();
  test_strict_jit_seed_104_keeps_distinct_tapu_route();
  return 0;
}
'''
atomic_write(Path("tests/issue_1516_quick_ball_tapu_crispin_tests.cpp"), tests)
