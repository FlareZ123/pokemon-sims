from __future__ import annotations

import importlib.util
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LEGACY_PATH = ROOT / "tools" / "apply_issue_792_v3.py"

spec = importlib.util.spec_from_file_location("apply_issue_792_legacy", LEGACY_PATH)
if spec is None or spec.loader is None:
    raise SystemExit("Unable to load the issue 792 applicator")
legacy = importlib.util.module_from_spec(spec)
spec.loader.exec_module(legacy)

original_replace_once = legacy.replace_once


def unified_replace_once(path: Path, old: str, new: str) -> None:
    if path == ROOT / "CMakeLists.txt":
        # Current main discovers every tests/*.cpp source through the unified runner:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt#L21-L47
        # https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py#L109-L139
        return
    original_replace_once(path, old, new)


legacy.replace_once = unified_replace_once
legacy.main()
