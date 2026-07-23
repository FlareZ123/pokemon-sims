from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path


TEST = Path("tests/issue_1378_no_lock_steven_grass_tests.cpp")
LOCK = Path(".issue-1379-compat.lock")
OLD = """void test_exact_seed_12_improves_to_t4() {
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
"""
NEW = """void test_exact_seed_12_keeps_steven_package_and_reaches_by_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered no-lock seed-12 fixture is unavailable.");

  std::mt19937_64 rng{12};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // #1378 owns Steven's public K1 target package. #1379 may improve the later
  // Star Alchemy continuation from the isolated T4 result to T3, so this
  // upstream regression requires the package to remain ready no later than T4:
  // https://github.com/FlareZ123/pokemon-sims/issues/1378
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 4,
         "The corrected no-lock seed 12 did not remain ready by T4.");
  expect(trace_contains(
             trace,
             "Searched up to 3 cards: Regidrago V, Crispin, Grass Energy"),
         "Steven still selected the redundant held Regidrago VSTAR axis.");
}
"""
OLD_CALL = "    test_exact_seed_12_improves_to_t4();\n"
NEW_CALL = "    test_exact_seed_12_keeps_steven_package_and_reaches_by_t4();\n"


@contextmanager
def locked(path: Path):
    handle = path.open("a+", encoding="utf-8")
    try:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        yield
    finally:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
        handle.close()


def main() -> int:
    with locked(LOCK):
        text = TEST.read_text(encoding="utf-8")
        if NEW not in text:
            function_count = text.count(OLD)
            call_count = text.count(OLD_CALL)
            if function_count != 1 or call_count != 1:
                raise RuntimeError(
                    "Unexpected #1378 downstream contract anchors: "
                    f"function={function_count}, call={call_count}"
                )
            text = text.replace(OLD, NEW, 1).replace(OLD_CALL, NEW_CALL, 1)
            temporary = TEST.with_suffix(TEST.suffix + ".tmp")
            temporary.write_text(text, encoding="utf-8")
            os.replace(temporary, TEST)
    LOCK.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
