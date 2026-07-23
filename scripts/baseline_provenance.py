from __future__ import annotations

import hashlib
from pathlib import Path


def simulator_policy_source_paths(repo_root: Path) -> tuple[Path, ...]:
    """Return every tracked input that can affect aggregate simulator output."""
    # Include the complete simulator source tree. In particular, part_016 owns the
    # aggregate scenario order, per-scenario seed derivation, simulate() invocation,
    # and CSV fields, so excluding it would permit stale fixed-seed output:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
    # https://github.com/FlareZ123/pokemon-sims/issues/642
    source_paths = [path for path in (repo_root / "src").rglob("*") if path.is_file()]

    # The executable target and compile configuration are also simulator inputs:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt#L1-L11
    paths = [repo_root / "CMakeLists.txt", *source_paths]
    missing = [path for path in paths if not path.is_file()]
    if missing:
        raise FileNotFoundError(", ".join(str(path) for path in missing))
    return tuple(sorted(paths))


def simulator_policy_source_digest(repo_root: Path) -> str:
    """Hash aggregate simulator inputs in stable path order."""
    digest = hashlib.sha256()
    for path in simulator_policy_source_paths(repo_root):
        relative_path = path.relative_to(repo_root).as_posix().encode("utf-8")
        digest.update(relative_path)
        digest.update(b"\0")
        digest.update(path.read_bytes())
        digest.update(b"\0")
    value = digest.hexdigest()
    print(f"SIMULATOR_POLICY_SOURCE_SHA256={value}")
    return value
