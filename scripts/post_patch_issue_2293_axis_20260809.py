from pathlib import Path
import os
import tempfile

path = Path(__file__).resolve().parents[1] / "src/trace_engine_v2/part_007.inc"
text = path.read_text(encoding="utf-8")
old = """        !need_active_vstar() || !need_energy() || !need_payload() ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
"""
new = """        !ability_available_for_pokemon(Card::LatiasEx) ||
"""
if old not in text:
    old = """        !need_active_vstar() ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
"""
if text.count(old) != 1:
    raise RuntimeError("#2293 projected-axis predicate anchor mismatch")
text = text.replace(old, new, 1)
fd, temporary = tempfile.mkstemp(prefix=path.name + ".", dir=path.parent, text=True)
try:
    with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
        handle.write(text)
        handle.flush()
        os.fsync(handle.fileno())
    os.replace(temporary, path)
except BaseException:
    try:
        os.unlink(temporary)
    except FileNotFoundError:
        pass
    raise
