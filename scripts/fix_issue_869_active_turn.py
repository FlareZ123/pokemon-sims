from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

LEGACY_OLD = """    attach_manual();
    // Steven's Resolve ends the turn, so attach a live Powerglass before choosing
    // that Supporter: https://api.pokemontcg.io/v2/cards/sm7-145
    if (should_play_steven()) attach_powerglass();
"""
LEGACY_NEW = """    attach_manual();
    // The confirmed K1 route must evolve the prior-turn Regidrago V before the
    // turn-ending Steven search so the remaining targets solve payload and promotion:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    if (late_steven_has_known_t3_compression_route()) evolve_best_regi();
    // Steven's Resolve ends the turn, so attach a live Powerglass before choosing
    // that Supporter: https://api.pokemontcg.io/v2/cards/sm7-145
    if (should_play_steven()) attach_powerglass();
"""

ACTIVE_ANCHOR = """    play_chaotic_swell();
    play_field_blower();
    // A redundant Regidrago V must not consume the final Bench slot before the
"""
ACTIVE_REPLACEMENT = """    play_chaotic_swell();
    play_field_blower();
    // Resolve the proven K1 Steven route before generic Items can consume Quick Ball
    // and erase the public-state predicate. Steven searches any three cards and ends
    // the turn, so attaching Fire and evolving the prior-turn Regidrago V first banks
    // the exact Latias ex, Mysterious Treasure, and Mega Dragonite ex continuation:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    if (late_steven_has_known_t3_compression_route()) {
      attach_manual();
      evolve_best_regi();
      choose_supporter();
      if (state_.turn_ended) {
        trace("TURN END", "R-STEVEN-01", state_line());
        return;
      }
    }
    // A redundant Regidrago V must not consume the final Bench slot before the
"""


def atomic_locked_replace(path: str, old: str, new: str) -> None:
    file_path = Path(path)
    with file_path.open("r+", encoding="utf-8") as locked:
        fcntl.flock(locked.fileno(), fcntl.LOCK_EX)
        original = locked.read()
        if new in original:
            return
        if old not in original:
            raise SystemExit(f"Unable to find turn-order anchor in {path}")
        updated = original.replace(old, new, 1)
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=file_path.parent, delete=False
        ) as temporary:
            temporary.write(updated)
            temporary.flush()
            os.fsync(temporary.fileno())
            temporary_path = Path(temporary.name)
        os.replace(temporary_path, file_path)


# The legacy body is macro-renamed by the active composition. Undo the applicator's
# legacy edit and place the route in the active replacement run_turn instead:
# https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc#L181-L186
# https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c_latias_bench_override.inc#L153-L175
# https://github.com/FlareZ123/pokemon-sims/issues/869
atomic_locked_replace("src/trace_engine_v2/part_014c.inc", LEGACY_NEW, LEGACY_OLD)
atomic_locked_replace(
    "src/trace_engine_v2/part_014c_latias_bench_override.inc",
    ACTIVE_ANCHOR,
    ACTIVE_REPLACEMENT,
)
