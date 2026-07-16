from __future__ import annotations

import hashlib
from pathlib import Path


def simulator_policy_source_digest(repo_root: Path) -> str:
    """Hash the exact simulation-policy inputs in stable path order."""
    # A policy source change invalidates the published fixed-seed matrix until
    # regeneration records the new digest: https://github.com/FlareZ123/pokemon-sims/issues/642
    # The translation-unit wrapper enumerates the policy includes:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/src/regidrago_sim.cpp
    source_root = repo_root / "src"
    excluded = {
        # part_016 contains the CLI parser, trace search, output writing, and main;
        # changes such as #641 do not alter aggregate trial outcomes:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
        # https://github.com/FlareZ123/pokemon-sims/issues/641
        repo_root / "src" / "trace_engine_v2" / "part_016.inc",
    }
    paths = sorted(
        path for path in source_root.rglob("*")
        if path.is_file() and path not in excluded
    )
    digest = hashlib.sha256()
    for path in paths:
        relative = path.relative_to(repo_root).as_posix().encode("utf-8")
        digest.update(relative)
        digest.update(b"\0")
        digest.update(path.read_bytes())
        digest.update(b"\0")
    return digest.hexdigest()
