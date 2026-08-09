from pathlib import Path
import os
import tempfile

path = Path(__file__).resolve().parents[1] / "src/trace_engine_v2/part_007.inc"
text = path.read_text(encoding="utf-8")
old = "        !is_basic(state_.active->card) || state_.active->card == Card::RegidragoV ||\n        !need_active_vstar() || bench_space() == 0 ||\n"
new = "        !is_basic(state_.active->card) || state_.active->card == Card::RegidragoV ||\n        bench_space() == 0 ||\n"
if text.count(old) != 1:
    raise RuntimeError("#2293 pre-evolution mobility gate anchor mismatch")
text = text.replace(old, new, 1)
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
