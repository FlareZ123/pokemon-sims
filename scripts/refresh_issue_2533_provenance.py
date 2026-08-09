from __future__ import annotations

import fcntl
import json
import os
import tempfile
from pathlib import Path

from baseline_provenance import simulator_policy_source_digest

ROOT = Path(__file__).resolve().parents[1]
OLD = "b24678b66228fbf43dfc650d2519444aec4d3471d0d9b4cb129d7ccff0357f65"


def atomic_write(path: Path, text: str) -> None:
    lock_path = path.with_name(f"{path.name}.lock")
    with lock_path.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile("w", encoding="utf-8", newline="\n", dir=path.parent, delete=False) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_name = tmp.name
        os.replace(tmp_name, path)
    lock_path.unlink(missing_ok=True)


digest = simulator_policy_source_digest(ROOT)
for relative in ("results/baseline_manifest.json", "results/multi_deck_manifest.json"):
    path = ROOT / relative
    data = json.loads(path.read_text(encoding="utf-8"))
    if data.get("simulator_policy_source_sha256") != OLD:
        raise RuntimeError(f"unexpected prior digest in {relative}")
    data["simulator_policy_source_sha256"] = digest
    atomic_write(path, json.dumps(data, indent=2) + "\n")

report = ROOT / "docs/MULTI_DECK_REPORT.md"
text = report.read_text(encoding="utf-8")
needle = f"Simulator policy digest: `{OLD}`."
replacement = f"Simulator policy digest: `{digest}`."
if text.count(needle) != 1:
    raise RuntimeError(f"unexpected report digest count: {text.count(needle)}")
atomic_write(report, text.replace(needle, replacement, 1))
print(digest)
