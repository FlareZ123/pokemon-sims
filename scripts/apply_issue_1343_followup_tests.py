from __future__ import annotations

import os
from pathlib import Path


ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1343"


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    if text.count(old) != 1:
        raise RuntimeError(f"{label} changed unexpectedly")
    return text.replace(old, new, 1)


def atomic_write(path: Path, text: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(text, encoding="utf-8")
    os.replace(temporary, path)


def update_issue_1092_trace() -> None:
    path = Path("tests/run_issue_1092_quick_ball_energy_jit_gate.cmake")
    text = path.read_text(encoding="utf-8")
    old = '''# A payload-only Quick Ball play advances no searched Basic axis. Seed 71 has
# Active Regidrago VSTAR at GG after the manual attachment is spent and no preferred
# Basic target, so the held payload and Quick Ball must wait while the later ready
# turn uses the strongest observable legal payload outlet available then:
# Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
# Regidrago VSTAR and GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
# Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
# Official Item, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# Energy-axis projection: https://github.com/FlareZ123/pokemon-sims/issues/737
# Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1092
run_trace("strict-jit/go-first" 71 issue_1092_seed_71)
if(issue_1092_seed_71 MATCHES "T2 \\| DISCARD \\|.*Quick Ball cost")
  message(FATAL_ERROR "Seed 71 still spent a payload-only Quick Ball cost before GGF could finish:\n${issue_1092_seed_71}")
endif()
if(issue_1092_seed_71 MATCHES "T2 \\| PLAY ITEM \\|.*Quick Ball")
  message(FATAL_ERROR "Seed 71 still played a payload-only Quick Ball before the strict-JIT Energy window:\n${issue_1092_seed_71}")
endif()
if(issue_1092_seed_71 MATCHES "LEGACY STAR")
  message(FATAL_ERROR "Seed 71 still spent the game-wide VSTAR Power after preserving its held resources:\n${issue_1092_seed_71}")
endif()
if(NOT issue_1092_seed_71 MATCHES "T3 \\| PLAY ITEM \\|.*Brilliant Blender")
  message(FATAL_ERROR "Seed 71 did not use the strongest observable T3 payload outlet:\n${issue_1092_seed_71}")
endif()
if(NOT issue_1092_seed_71 MATCHES "T3 \\| READY \\|")
  message(FATAL_ERROR "Seed 71 lost its T3 readiness after the target-aware gate:\n${issue_1092_seed_71}")
endif()'''
    new = '''# Seed 71 must wait until GGF is complete, then use the held one-discard Item
# and public Dragon payload. Quick Ball legally searches the remaining Regidrago V,
# reaches the same strict-JIT T3 ready state, and preserves Arven plus the one-copy
# Brilliant Blender for a later axis:
# Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
# Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
# Arven: https://api.pokemontcg.io/v2/cards/sv1-166
# Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
# Regidrago VSTAR and GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
# Official Item, attachment, search, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# Original timing boundary: https://github.com/FlareZ123/pokemon-sims/issues/1092
# Confirmed stronger route: https://github.com/FlareZ123/pokemon-sims/issues/1343
run_trace("strict-jit/go-first" 71 issue_1092_seed_71)
if(issue_1092_seed_71 MATCHES "T2 \\| DISCARD \\|.*Quick Ball cost")
  message(FATAL_ERROR "Seed 71 still spent Quick Ball before GGF could finish:\n${issue_1092_seed_71}")
endif()
if(issue_1092_seed_71 MATCHES "T2 \\| PLAY ITEM \\|.*Quick Ball")
  message(FATAL_ERROR "Seed 71 still played Quick Ball before the strict-JIT Energy window:\n${issue_1092_seed_71}")
endif()
if(issue_1092_seed_71 MATCHES "LEGACY STAR")
  message(FATAL_ERROR "Seed 71 spent the game-wide VSTAR Power despite the held direct outlet:\n${issue_1092_seed_71}")
endif()
if(NOT issue_1092_seed_71 MATCHES "T3 \\| DISCARD \\|.*Mega Dragonite ex.*Quick Ball cost")
  message(FATAL_ERROR "Seed 71 did not use the held Dragon as Quick Ball's T3 cost:\n${issue_1092_seed_71}")
endif()
if(NOT issue_1092_seed_71 MATCHES "T3 \\| PLAY ITEM \\|.*Quick Ball")
  message(FATAL_ERROR "Seed 71 did not use the resource-preserving T3 Quick Ball route:\n${issue_1092_seed_71}")
endif()
if(issue_1092_seed_71 MATCHES "T3 \\| PLAY ITEM \\|.*Brilliant Blender")
  message(FATAL_ERROR "Seed 71 still consumed Brilliant Blender despite the held payload outlet:\n${issue_1092_seed_71}")
endif()
if(NOT issue_1092_seed_71 MATCHES "T3 \\| READY \\|")
  message(FATAL_ERROR "Seed 71 lost T3 readiness after preserving Arven and Blender:\n${issue_1092_seed_71}")
endif()'''
    atomic_write(path, replace_once(text, old, new, "Issue 1092 trace expectation"))


def update_issue_1139_trace() -> None:
    path = Path("tests/issue_1139_fss_bank_tate_tests.cpp")
    text = path.read_text(encoding="utf-8")
    old = '''void test_seed_100_reaches_t5_via_tate_and_blender() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  expect(scenario.has_value(), "Missing strict-JIT going-first scenario.");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{100};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const bool searched_tate = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("STAR ALCHEMY") != std::string::npos &&
               line.find("Tate & Liza") != std::string::npos;
      });
  const bool switched_t5 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T5 | PLAY SUPPORTER") != std::string::npos &&
               line.find("R-TATE-01") != std::string::npos;
      });
  const bool blender_t5 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T5 | PLAY ITEM") != std::string::npos &&
               line.find("R-BLENDER-01") != std::string::npos;
      });

  // This deterministic trace is the issue reproduction and validates the complete
  // banked route at the repository's diagnostic T5 horizon:
  // https://github.com/FlareZ123/pokemon-sims/issues/1139
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/T5_FAILURE_POLICY.md
  expect(outcome.first_ready_turn == 5,
         "Seed 100 must recover to the earliest legal T5 ready state.");
  expect(searched_tate, "Seed 100 must bank Tate with Star Alchemy on T4.");
  expect(switched_t5, "Seed 100 must use Tate switch mode on T5.");
  expect(blender_t5, "Seed 100 must use held Blender for the T5 payload.");
}'''
    new = '''void test_seed_100_reaches_t4_via_held_payload_outlet() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  expect(scenario.has_value(), "Missing strict-JIT going-first scenario.");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{100};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const bool discarded_dragapult_t4 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T4 | DISCARD") != std::string::npos &&
               line.find("Dragapult ex (Quick Ball cost)") != std::string::npos;
      });
  const bool searched_lele_t4 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T4 | PLAY ITEM") != std::string::npos &&
               line.find("Tapu Lele-GX") != std::string::npos;
      });
  const bool wonder_tag_tate_t4 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T4 | WONDER TAG") != std::string::npos &&
               line.find("Tate & Liza") != std::string::npos;
      });
  const bool switched_t4 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T4 | PLAY SUPPORTER") != std::string::npos &&
               line.find("R-TATE-01") != std::string::npos;
      });
  const bool consumed_blender_t4 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T4 | PLAY ITEM") != std::string::npos &&
               line.find("R-BLENDER-01") != std::string::npos;
      });
  const bool used_star_alchemy = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("STAR ALCHEMY") != std::string::npos;
      });

  // Seed 100 now exposes a stronger public T4 route. Quick Ball discards the held
  // Dragapult ex, searches Tapu Lele-GX, Wonder Tag finds Tate & Liza, and Tate
  // promotes the GGF Regidrago VSTAR. The route reaches readiness one turn earlier
  // while preserving Arven, Brilliant Blender, and the game-wide VSTAR Power:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core search, Ability, Supporter, switch, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest-route and resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original T5 fallback: https://github.com/FlareZ123/pokemon-sims/issues/1139
  // Confirmed stronger route: https://github.com/FlareZ123/pokemon-sims/issues/1343
  expect(outcome.first_ready_turn == 4,
         "Seed 100 must use the stronger legal T4 ready route.");
  expect(discarded_dragapult_t4,
         "Seed 100 must use held Dragapult ex as Quick Ball's strict-JIT cost.");
  expect(searched_lele_t4,
         "Seed 100 must search Tapu Lele-GX with Quick Ball on T4.");
  expect(wonder_tag_tate_t4,
         "Seed 100 must use Wonder Tag to find Tate & Liza on T4.");
  expect(switched_t4,
         "Seed 100 must use Tate switch mode to promote Regidrago VSTAR on T4.");
  expect(!consumed_blender_t4,
         "Seed 100 must preserve Brilliant Blender when Quick Ball supplies payload.");
  expect(!used_star_alchemy,
         "Seed 100 must preserve the game-wide VSTAR Power on the direct T4 route.");
}'''
    text = replace_once(text, old, new, "Issue 1139 deterministic route")
    text = replace_once(
        text,
        "  test_seed_100_reaches_t5_via_tate_and_blender();",
        "  test_seed_100_reaches_t4_via_held_payload_outlet();",
        "Issue 1139 test registration",
    )
    atomic_write(path, text)


def main() -> int:
    update_issue_1092_trace()
    update_issue_1139_trace()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
