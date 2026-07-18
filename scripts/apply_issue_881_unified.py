from __future__ import annotations

import fcntl
import importlib.util
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LEGACY_PATH = ROOT / "scripts" / "apply_issue_881.py"

spec = importlib.util.spec_from_file_location("apply_issue_881_legacy", LEGACY_PATH)
if spec is None or spec.loader is None:
    raise SystemExit("Unable to load the issue 881 applicator")
legacy = importlib.util.module_from_spec(spec)
spec.loader.exec_module(legacy)

original_insert_after_once = legacy.insert_after_once


def unified_insert_after_once(path: Path, anchor: str, addition: str) -> None:
    if path == ROOT / "CMakeLists.txt":
        # Current main discovers every tests/*.cpp source through the unified generator:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt#L21-L47
        # https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py#L109-L139
        return
    original_insert_after_once(path, anchor, addition)


legacy.insert_after_once = unified_insert_after_once
legacy.LOCK_PATH.touch(exist_ok=True)
with legacy.LOCK_PATH.open("r+", encoding="utf-8") as lock_handle:
    fcntl.flock(lock_handle.fileno(), fcntl.LOCK_EX)
    legacy.apply()
