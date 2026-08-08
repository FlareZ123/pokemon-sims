from pathlib import Path
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    fd, temp_name = tempfile.mkstemp(prefix=path.name + ".", dir=path.parent, text=True)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp_name, path)
    except BaseException:
        try:
            os.unlink(temp_name)
        except FileNotFoundError:
            pass
        raise


# The first bootstrap proved the new route must wrap the existing empty-deck
# Arven guard rather than bypassing it. Preserve the established wrapper stack
# and make #2291 the outermost Arven policy layer.
sim = ROOT / "src/regidrago_sim.cpp"
s = sim.read_text(encoding="utf-8")
wrong = '''#define play_arven play_arven_issue2291_original
#include "trace_engine_v2/part_012_arven_fss_blender_contention_override.inc"
#undef play_arven
#include "trace_engine_v2/part_issue_2291_arven_vessel_fss_override.inc"
#define choose_supporter choose_supporter_original
'''
original = '''#define play_arven play_arven_empty_deck_original
#include "trace_engine_v2/part_012_arven_fss_blender_contention_override.inc"
#undef play_arven
#define choose_supporter choose_supporter_original
'''
if wrong in s:
    s = s.replace(wrong, original, 1)

old = '''#define use_fss use_fss_issue1356_original
#include "trace_engine_v2/part_empty_deck_search_override.inc"
#undef use_fss
#undef play_steven
'''
new = '''#define use_fss use_fss_issue1356_original
#define play_arven play_arven_issue2291_original
#include "trace_engine_v2/part_empty_deck_search_override.inc"
#undef play_arven
#undef use_fss
#include "trace_engine_v2/part_issue_2291_arven_vessel_fss_override.inc"
#undef play_steven
'''
if new not in s:
    if s.count(old) != 1:
        raise RuntimeError("#2291 outer Arven wrapper anchor mismatch")
    s = s.replace(old, new, 1)
atomic_write(sim, s)

# The same route exists before Wonder Tag while Arven is in deck and after Wonder
# Tag while Arven is in hand. Permit either public zone so the downstream Arven
# resolver recognizes the already-selected route.
part7 = ROOT / "src/trace_engine_v2/part_007.inc"
p = part7.read_text(encoding="utf-8")
old_gate = "        deck_count_after_search_started(Card::Arven) == 0 ||\n"
new_gate = "        (deck_count_after_search_started(Card::Arven) == 0 && hand_count(Card::Arven) == 0) ||\n"
if new_gate not in p:
    if p.count(old_gate) != 1:
        raise RuntimeError("#2291 Arven zone gate mismatch")
    p = p.replace(old_gate, new_gate, 1)
atomic_write(part7, p)
