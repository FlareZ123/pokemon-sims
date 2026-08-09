from __future__ import annotations

import hashlib
from dataclasses import dataclass, field
from pathlib import Path
from typing import Protocol


SIMULATOR_BUILD_INPUT = Path("CMakeLists.txt")
SIMULATOR_SOURCE_ROOT = Path("src")
SOURCE_LOCK_SUFFIX = ".lock"
SOURCE_FRAME_SEPARATOR = b"\0"
PathSequence = tuple[Path, ...]


class _Digest(Protocol):
    def update(self, data: bytes, /) -> None: ...

    def hexdigest(self) -> str: ...


def _is_simulator_source_input(path: Path) -> bool:
    """Return whether a path is a tracked simulator input for provenance."""
    return path.is_file() and path.suffix != SOURCE_LOCK_SUFFIX


def _stable_paths(paths: PathSequence) -> PathSequence:
    """Return provenance paths in deterministic filesystem-independent order."""
    return tuple(sorted(paths))


@dataclass(frozen=True)
class _SimulatorSourceManifest:
    repo_root: Path

    def source_paths(self) -> PathSequence:
        """Collect tracked simulator source inputs, excluding writer lock files."""
        source_root = self.repo_root / SIMULATOR_SOURCE_ROOT
        return tuple(
            path
            for path in source_root.rglob("*")
            if _is_simulator_source_input(path)
        )

    def required_paths(self) -> PathSequence:
        """Collect every required aggregate simulator input."""
        return (self.repo_root / SIMULATOR_BUILD_INPUT, *self.source_paths())

    def paths(self) -> PathSequence:
        """Validate and return the stable aggregate simulator input sequence."""
        return _stable_paths(_validated_paths(self.required_paths()))


def _manifest(repo_root: Path) -> _SimulatorSourceManifest:
    """Create the simulator provenance manifest for a repository root."""
    return _SimulatorSourceManifest(repo_root)


def _missing_paths(paths: PathSequence) -> PathSequence:
    """Return required provenance inputs that are absent from disk."""
    return tuple(path for path in paths if not path.is_file())


def _validated_paths(paths: PathSequence) -> PathSequence:
    """Require every provenance input to exist and return it unchanged."""
    missing = _missing_paths(paths)
    if missing:
        raise FileNotFoundError(", ".join(str(path) for path in missing))
    return paths


def simulator_policy_source_paths(repo_root: Path) -> PathSequence:
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
    return _manifest(repo_root).paths()


def _relative_path_bytes(repo_root: Path, path: Path) -> bytes:
    """Encode a source path relative to the repository root."""
    return path.relative_to(repo_root).as_posix().encode("utf-8")


def _update_digest(digest: _Digest, repo_root: Path, path: Path) -> None:
    """Frame one source path and its bytes into the aggregate digest."""
    digest.update(_relative_path_bytes(repo_root, path))
    digest.update(SOURCE_FRAME_SEPARATOR)
    digest.update(path.read_bytes())
    digest.update(SOURCE_FRAME_SEPARATOR)


@dataclass
class _SourceDigestBuilder:
    repo_root: Path
    digest: _Digest = field(default_factory=hashlib.sha256)

    def add_path(self, path: Path) -> None:
        """Add one framed source input to the aggregate digest."""
        _update_digest(self.digest, self.repo_root, path)

    def add_paths(self, paths: PathSequence) -> None:
        """Add a stable sequence of source inputs to the aggregate digest."""
        for path in paths:
            self.add_path(path)


def _build_source_digest(repo_root: Path) -> _Digest:
    """Build the aggregate digest from the stable simulator input sequence."""
    source_paths = simulator_policy_source_paths(repo_root)
    builder = _SourceDigestBuilder(repo_root)
    builder.add_paths(source_paths)
    return builder.digest


def simulator_policy_source_digest(repo_root: Path) -> str:
    """Hash aggregate simulator inputs in stable path order."""
    return _build_source_digest(repo_root).hexdigest()
