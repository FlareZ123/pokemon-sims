from __future__ import annotations

import hashlib
from pathlib import Path
from typing import Protocol


SIMULATOR_BUILD_INPUT = "CMakeLists.txt"
SIMULATOR_SOURCE_ROOT = "src"
SOURCE_LOCK_SUFFIX = ".lock"


class _DigestWriter(Protocol):
    def update(self, data: bytes, /) -> None: ...


def _is_simulator_source_input(path: Path) -> bool:
    """Return whether a path is a tracked simulator input for provenance."""
    return path.is_file() and path.suffix != SOURCE_LOCK_SUFFIX


def _simulator_source_paths(repo_root: Path) -> list[Path]:
    """Collect tracked simulator source inputs, excluding writer lock files."""
    return [
        path
        for path in (repo_root / SIMULATOR_SOURCE_ROOT).rglob("*")
        if _is_simulator_source_input(path)
    ]


def _missing_paths(paths: list[Path]) -> list[Path]:
    """Return required provenance inputs that are absent from disk."""
    return [path for path in paths if not path.is_file()]


def _required_policy_paths(repo_root: Path) -> list[Path]:
    """Collect and validate every required aggregate simulator input."""
    paths = [repo_root / SIMULATOR_BUILD_INPUT, *_simulator_source_paths(repo_root)]
    missing = _missing_paths(paths)
    if missing:
        raise FileNotFoundError(", ".join(str(path) for path in missing))
    return paths


def simulator_policy_source_paths(repo_root: Path) -> tuple[Path, ...]:
    """Return every tracked input that can affect aggregate simulator output."""
    # Include the complete simulator source tree. In particular, part_016 owns the
    # aggregate scenario order, per-scenario seed derivation, simulate() invocation,
    # and CSV fields, so excluding it would permit stale fixed-seed output:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
    # https://github.com/FlareZ123/pokemon-sims/issues/642
    # Source-update lock files coordinate writers and cannot affect compilation or
    # simulator behavior, so they must stay outside source-bound evidence:
    # https://github.com/FlareZ123/pokemon-sims/issues/1492
    # https://github.com/FlareZ123/pokemon-sims/issues/1300
    # The executable target and compile configuration are also simulator inputs:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt#L1-L11
    return tuple(sorted(_required_policy_paths(repo_root)))


def _update_digest(digest: _DigestWriter, repo_root: Path, path: Path) -> None:
    """Frame one source path and its bytes into the aggregate digest."""
    relative_path = path.relative_to(repo_root).as_posix().encode("utf-8")
    digest.update(relative_path)
    digest.update(b"\0")
    digest.update(path.read_bytes())
    digest.update(b"\0")


def simulator_policy_source_digest(repo_root: Path) -> str:
    """Hash aggregate simulator inputs in stable path order."""
    digest = hashlib.sha256()
    for path in simulator_policy_source_paths(repo_root):
        _update_digest(digest, repo_root, path)
    return digest.hexdigest()
