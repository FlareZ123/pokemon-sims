from __future__ import annotations

import fcntl
import os
from pathlib import Path
import tempfile

PATH = Path("src/trace_engine_v2/part_tapu_tate_switch_override.inc")
LOCK_PATH = PATH.with_suffix(PATH.suffix + ".lock")

OLD = """      const Pokemon* target = target_regi();
      if (target == nullptr) return false;

      const bool grass_searchable = might_be_unseen(Card::Grass);
"""

NEW = """      const Pokemon* target = target_regi();
      if (target == nullptr) {
        // Preserve the established pre-Regidrago opening projection. With no
        // physical Regidrago target yet, this is the legacy Basic-only planning
        // path: it needs the current manual attachment plus Crispin's two
        // different-type Basic search and the following turn's manual attachment.
        // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
        // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
        // Turn and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // K0/K1 and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
        // Existing pre-target contract: https://github.com/FlareZ123/pokemon-sims/issues/1022
        // Refined DDE interaction: https://github.com/FlareZ123/pokemon-sims/issues/2446
        if (state_.manual_energy_used || !might_be_unseen(Card::Grass) ||
            !might_be_unseen(Card::Fire)) {
          return false;
        }

        const int initial_grass_needed = grass_needed();
        const int initial_fire_needed = fire_needed();
        const int held_grass = hand_count(Card::Grass);
        const int held_fire = hand_count(Card::Fire);

        const auto completes_after_current_manual = [&](const int current_manual) {
          int remaining_grass = initial_grass_needed;
          int remaining_fire = initial_fire_needed;
          int grass_in_hand = held_grass;
          int fire_in_hand = held_fire;

          if (current_manual == 1) {
            if (remaining_grass <= 0 || grass_in_hand <= 0) return false;
            --remaining_grass;
            --grass_in_hand;
          } else if (current_manual == 2) {
            if (remaining_fire <= 0 || fire_in_hand <= 0) return false;
            --remaining_fire;
            --fire_in_hand;
          }

          const auto completes_after_crispin = [&](const bool attach_grass) {
            int grass_after_crispin = remaining_grass;
            int fire_after_crispin = remaining_fire;
            int grass_available_for_manual = grass_in_hand;
            int fire_available_for_manual = fire_in_hand;

            if (attach_grass) {
              if (grass_after_crispin <= 0) return false;
              --grass_after_crispin;
              ++fire_available_for_manual;
            } else {
              if (fire_after_crispin <= 0) return false;
              --fire_after_crispin;
              ++grass_available_for_manual;
            }

            if (grass_after_crispin == 0 && fire_after_crispin == 0) return true;
            if (grass_after_crispin == 1 && fire_after_crispin == 0) {
              return grass_available_for_manual > 0;
            }
            if (grass_after_crispin == 0 && fire_after_crispin == 1) {
              return fire_available_for_manual > 0;
            }
            return false;
          };

          return completes_after_crispin(true) || completes_after_crispin(false);
        };

        return completes_after_current_manual(0) ||
               completes_after_current_manual(1) ||
               completes_after_current_manual(2);
      }

      const bool grass_searchable = might_be_unseen(Card::Grass);
"""


def atomic_replace(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temp_name = tempfile.mkstemp(prefix=path.name + ".", dir=path.parent, text=True)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp_name, path)
    finally:
        if os.path.exists(temp_name):
            os.unlink(temp_name)


with LOCK_PATH.open("w", encoding="utf-8") as lock:
    fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
    current = PATH.read_text(encoding="utf-8")
    if current.count(OLD) != 1:
        raise SystemExit("expected exactly one #2446 target-null block")
    atomic_replace(PATH, current.replace(OLD, NEW, 1))
    fcntl.flock(lock.fileno(), fcntl.LOCK_UN)

LOCK_PATH.unlink(missing_ok=True)
