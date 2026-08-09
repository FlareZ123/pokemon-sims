from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"


def atomic_write(path: Path, text: str) -> None:
    lock_path = path.with_name(f"{path.name}.lock")
    with lock_path.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile("w", encoding="utf-8", newline="\n", dir=path.parent, delete=False) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            name = tmp.name
        os.replace(name, path)
    lock_path.unlink(missing_ok=True)

text = PATH.read_text(encoding="utf-8")
old_guard = '''        !state_.active || hand_count(Card::BrilliantBlender) == 0 ||
        !payload_might_be_in_deck() || !can_play_payload_this_turn()) {
'''
new_guard = '''        !state_.active || hand_count(Card::BrilliantBlender) == 0 ||
        !payload_might_be_in_deck() || !can_play_payload_this_turn() ||
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
'''
if text.count(old_guard) != 1:
    raise RuntimeError(f"guard anchor count {text.count(old_guard)}")
text = text.replace(old_guard, new_guard, 1)
start = text.index('    // Preflight the complete post-retreat Blender line on a copy')
end = text.index('    if (!remove_one(state_.hand, payment)) return false;', start)
replacement = '''    // This reclaimed route is deliberately narrower than generic Blender policy:
    // it fires only when no Dragon payload is already held, so Blender cannot be
    // superseded by a cheaper visible hand-discard outlet. The remaining Blender
    // gates are already satisfied by the preconditions above plus the Apex-ready
    // post-retreat target, which keeps this mobility fix deterministic and avoids
    // policy projection side effects.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Resource-priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
'''
text = text[:start] + replacement + text[end:]
atomic_write(PATH, text)
