from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path


SOURCE = Path("src/trace_engine_v2/part_forretress_ex_combo.inc")
LOCK = Path(".issue-1377-compat.lock")
OLD = """  const bool combo_stadium_is_useful = !state_.stadium_used && need_energy() &&
      hand_count(Card::ForestOfVitality) > 0 &&
      state_.stadium != Stadium::ForestOfVitality &&
      in_play_count(Card::ForretressEx) == 0 && !pineco_can_evolve_normally &&
      forretress_available && (current_turn_pineco || pineco_can_enter_this_turn);
"""
NEW = """  // Playing Forest into Chaotic Swell still resolves the established Stadium
  // replacement interaction and clears both Stadiums:
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Repository contract: https://github.com/FlareZ123/pokemon-sims/issues/1096
  const bool chaotic_swell_replacement =
      state_.stadium == Stadium::ChaoticSwell;
  const bool combo_stadium_is_useful = !state_.stadium_used && need_energy() &&
      hand_count(Card::ForestOfVitality) > 0 &&
      state_.stadium != Stadium::ForestOfVitality &&
      (chaotic_swell_replacement ||
       (in_play_count(Card::ForretressEx) == 0 &&
        !pineco_can_evolve_normally && forretress_available &&
        (current_turn_pineco || pineco_can_enter_this_turn)));
"""


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
        text = SOURCE.read_text(encoding="utf-8")
        if NEW not in text:
            count = text.count(OLD)
            if count != 1:
                raise RuntimeError(f"Unexpected Chaotic Swell compatibility anchor count: {count}")
            temporary = SOURCE.with_suffix(SOURCE.suffix + ".tmp")
            temporary.write_text(text.replace(OLD, NEW, 1), encoding="utf-8")
            os.replace(temporary, SOURCE)
    LOCK.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
