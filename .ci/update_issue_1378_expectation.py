from __future__ import annotations

import os
from pathlib import Path


PATH = Path("tests/issue_1378_no_lock_steven_grass_tests.cpp")
OLD = '''void test_exact_seed_12_improves_to_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered no-lock seed-12 fixture is unavailable.");

  std::mt19937_64 rng{12};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 4,
         "The corrected no-lock seed 12 did not reach the proven T4 state.");
  expect(trace_contains(
             trace,
             "Searched up to 3 cards: Regidrago V, Crispin, Grass Energy"),
         "Steven still selected the redundant held Regidrago VSTAR axis.");
  // This PR intentionally leaves the separately confirmed Star Alchemy target
  // defect to issue #1379, so T4 is the expected isolated result:
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
}
'''
NEW = '''void test_exact_seed_12_improves_to_t3_with_latias_follow_up() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered no-lock seed-12 fixture is unavailable.");

  std::mt19937_64 rng{12};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 3,
         "The corrected no-lock seed 12 did not reach the complete T3 route.");
  expect(trace_contains(
             trace,
             "Searched up to 3 cards: Regidrago V, Crispin, Grass Energy"),
         "Steven still selected the redundant held Regidrago VSTAR axis.");
  expect(trace_contains(trace, "Searched any card: Latias ex"),
         "Star Alchemy did not resolve the remaining Active-position axis.");
  // Expected route marker: STAR ALCHEMY searched Latias ex.
  // The independently merged Steven correction supplies the GG foundation.
  // Issue #1379 completes the same public continuation by searching Latias ex,
  // attaching Fire, evolving, using Burnet, and retreating the Basic Active:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed dependent improvement: https://github.com/FlareZ123/pokemon-sims/issues/1379
}
'''
OLD_CALL = "    test_exact_seed_12_improves_to_t4();\n"
NEW_CALL = "    test_exact_seed_12_improves_to_t3_with_latias_follow_up();\n"


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def main() -> int:
    content = PATH.read_text(encoding="utf-8")
    if NEW in content and NEW_CALL in content:
        return 0
    if content.count(OLD) != 1 or content.count(OLD_CALL) != 1:
        raise RuntimeError("Issue 1378 dependent expectation anchor was not found")
    content = content.replace(OLD, NEW, 1).replace(OLD_CALL, NEW_CALL, 1)
    atomic_write(PATH, content)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
