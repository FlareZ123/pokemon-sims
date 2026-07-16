from __future__ import annotations

import hashlib
from pathlib import Path


def simulator_source_digest(repo_root: Path) -> str:
    """Hash every simulator translation-unit input in stable path order."""
    # The published fixed-seed matrix is invalid after any simulator source change:
    # https://github.com/FlareZ123/pokemon-sims/issues/642
    # The wrapper and every included implementation fragment live under src/:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/src/regidrago_sim.cpp
    # part_016.inc is intentionally included because it owns aggregate scenario
    # iteration, derived seeds, simulate() calls, and CSV serialization:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
    source_root = repo_root / "src"
    paths = sorted(path for path in source_root.rglob("*") if path.is_file())
    if not paths:
        raise FileNotFoundError(f"No simulator sources found under {source_root}")

    digest = hashlib.sha256()
    for path in paths:
        relative = path.relative_to(repo_root).as_posix().encode("utf-8")
        digest.update(relative)
        digest.update(b"\0")
        digest.update(path.read_bytes())
        digest.update(b"\0")
    return digest.hexdigest()
