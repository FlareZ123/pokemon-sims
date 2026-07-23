from __future__ import annotations

import json
import os
import re
import tempfile
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator

from scripts.baseline_provenance import simulator_policy_source_digest


ROOT = Path(__file__).resolve().parents[1]
BASELINE_MANIFEST = ROOT / "results" / "baseline_manifest.json"
MULTI_MANIFEST = ROOT / "results" / "multi_deck_manifest.json"
MULTI_REPORT = ROOT / "docs" / "MULTI_DECK_REPORT.md"
LOCK_PATH = Path(tempfile.gettempdir()) / "pokemon-sims-issue-1389-provenance.lock"


@contextmanager
def exclusive_lock(path: Path) -> Iterator[None]:
    descriptor = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))
        yield
    finally:
        os.close(descriptor)
        path.unlink(missing_ok=True)


def atomic_write(path: Path, content: str) -> None:
    with tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
    ) as handle:
        handle.write(content)
        temporary = Path(handle.name)
    os.replace(temporary, path)


def update_manifest(path: Path, digest: str) -> None:
    payload = json.loads(path.read_text(encoding="utf-8"))
    payload["simulator_policy_source_sha256"] = digest
    atomic_write(path, json.dumps(payload, indent=2, sort_keys=True) + "\n")


def main() -> int:
    with exclusive_lock(LOCK_PATH):
        digest = simulator_policy_source_digest(ROOT)
        update_manifest(BASELINE_MANIFEST, digest)
        update_manifest(MULTI_MANIFEST, digest)

        report = MULTI_REPORT.read_text(encoding="utf-8")
        updated, count = re.subn(
            r"Simulator policy digest: `[0-9a-f]{64}`\.",
            f"Simulator policy digest: `{digest}`.",
            report,
            count=1,
        )
        if count != 1:
            raise RuntimeError(f"Unexpected report digest anchor count: {count}")
        atomic_write(MULTI_REPORT, updated)
        print(digest)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
