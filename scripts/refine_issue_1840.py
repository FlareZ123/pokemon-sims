from __future__ import annotations

import fcntl
import os
import tempfile
from contextlib import contextmanager
from pathlib import Path


@contextmanager
def locked_path(path: Path):
    lock_path = path.with_suffix(path.suffix + ".lock")
    descriptor = os.open(lock_path, os.O_CREAT | os.O_RDWR, 0o600)
    try:
        fcntl.flock(descriptor, fcntl.LOCK_EX)
        yield
    finally:
        fcntl.flock(descriptor, fcntl.LOCK_UN)
        os.close(descriptor)
        lock_path.unlink(missing_ok=True)


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with locked_path(path):
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            newline="",
            dir=path.parent,
            prefix=f".{path.name}.",
            delete=False,
        ) as handle:
            handle.write(content)
            temporary = Path(handle.name)
        os.replace(temporary, path)


old_id = "me2" + "-132"
new_id = "me2pt5-152"
for path_text in (
    "scripts/apply_issue_1840.py",
    "src/trace_engine_v2/part_issue_1118_secret_box.inc",
    "tests/issue_1840_secret_box_surplus_fire_tests.cpp",
):
    path = Path(path_text)
    text = path.read_text(encoding="utf-8")
    if old_id not in text:
        continue
    atomic_write(path, text.replace(old_id, new_id))
