from __future__ import annotations

import contextlib
import os
from pathlib import Path
import tempfile


SOURCE = Path("src/trace_engine_v2/part_issue_1071_fss_oricorio_treasure_decomposition_override.inc")
TEST = Path("tests/issue_1379_t3_fss_latias_burnet_tests.cpp")
LOCK = Path(".issue-1379-multi-regi.lock")

OLD_SOURCE = """    const Pokemon* evolving_regi = nullptr;
    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card == Card::RegidragoV &&
          pokemon.entered_turn < state_.turn) {
        evolving_regi = &pokemon;
        break;
      }
    }
    if (evolving_regi == nullptr) return false;

    const int missing_grass = std::max(0, 2 - evolving_regi->grass);
    const int missing_fire = std::max(0, 1 - evolving_regi->fire);
    const bool energy_complete_now = missing_grass == 0 && missing_fire == 0;
    const bool unused_manual_completes = !state_.manual_energy_used &&
        missing_grass + missing_fire == 1 &&
        ((missing_grass == 1 && hand_count(Card::Grass) > 0) ||
         (missing_fire == 1 && hand_count(Card::Fire) > 0));
    if (!energy_complete_now && !unused_manual_completes) return false;
"""

NEW_SOURCE = """    bool eligible_regi = false;
    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn) {
        continue;
      }

      const int missing_grass = std::max(0, 2 - pokemon.grass);
      const int missing_fire = std::max(0, 1 - pokemon.fire);
      const bool energy_complete_now = missing_grass == 0 && missing_fire == 0;
      const bool unused_manual_completes = !state_.manual_energy_used &&
          missing_grass + missing_fire == 1 &&
          ((missing_grass == 1 && hand_count(Card::Grass) > 0) ||
           (missing_fire == 1 && hand_count(Card::Fire) > 0));
      if (energy_complete_now || unused_manual_completes) {
        eligible_regi = true;
        break;
      }
    }
    if (!eligible_regi) return false;
"""

TEST_ANCHOR = """  expect(target_for(std::move(completed)) == sim::Card::LatiasEx,
         \"The actual post-attachment GGF state did not choose Latias.\");
"""

TEST_INSERT = """  expect(target_for(std::move(completed)) == sim::Card::LatiasEx,
         \"The actual post-attachment GGF state did not choose Latias.\");

  sim::State multiple_regi = post_crispin_state();
  multiple_regi.bench.insert(
      multiple_regi.bench.begin() + 1,
      sim::Pokemon{sim::Card::RegidragoV, 2, 1, 0});
  // Bench order cannot invalidate a legal evolution and GGF route on another
  // prior-turn Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(target_for(std::move(multiple_regi)) == sim::Card::LatiasEx,
         \"An earlier underpowered Regidrago hid a later eligible attacker.\");
"""


@contextlib.contextmanager
def exclusive_lock(path: Path):
    handle = path.open("a+", encoding="utf-8")
    try:
        import fcntl

        fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        yield
    finally:
        try:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
        finally:
            handle.close()


def atomic_write(path: Path, text: str) -> None:
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=path.parent, delete=False, newline=""
    ) as temporary:
        temporary.write(text)
        temporary.flush()
        os.fsync(temporary.fileno())
        temporary_path = Path(temporary.name)
    os.replace(temporary_path, path)


def replace_once(text: str, old: str, new: str, name: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Unexpected {name} anchor count: {count}")
    return text.replace(old, new, 1)


def main() -> int:
    with exclusive_lock(LOCK):
        source = SOURCE.read_text(encoding="utf-8")
        source = replace_once(source, OLD_SOURCE, NEW_SOURCE, "source")
        atomic_write(SOURCE, source)

        test = TEST.read_text(encoding="utf-8")
        test = replace_once(test, TEST_ANCHOR, TEST_INSERT, "test")
        atomic_write(TEST, test)

    LOCK.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
