from __future__ import annotations

import hashlib
import json
import os
import tempfile
from pathlib import Path

from scripts.baseline_provenance import simulator_policy_source_digest

ROOT = Path(__file__).resolve().parents[1]


def atomic_json(path: Path, value: dict[str, object]) -> None:
    fd, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as handle:
            json.dump(value, handle, indent=2)
            handle.write("\n")
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary_name, path)
    finally:
        if os.path.exists(temporary_name):
            os.unlink(temporary_name)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


digest = simulator_policy_source_digest(ROOT)

baseline_path = ROOT / "results" / "baseline_manifest.json"
baseline = json.loads(baseline_path.read_text(encoding="utf-8"))
baseline["simulator_policy_source_sha256"] = digest
atomic_json(baseline_path, baseline)

multi_path = ROOT / "results" / "multi_deck_manifest.json"
multi = json.loads(multi_path.read_text(encoding="utf-8"))
multi["comparison_csv_sha256"] = sha256(ROOT / "results" / "multi_deck_comparison.csv")
multi["simulator_policy_source_sha256"] = digest
atomic_json(multi_path, multi)
