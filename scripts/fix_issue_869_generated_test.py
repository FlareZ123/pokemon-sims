from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

PATH = Path("tests/late_steven_t3_compression_tests.cpp")
OLD = "  static bool ready(const Engine& engine) { return engine.ready(); }\n"
NEW = """  static bool ready(const Engine& engine) {
    const State& state = engine.state_;
    return state.turn >= 2 && engine.active_is_vstar() &&
        state.active->grass >= 2 && state.active->fire >= 1 && engine.payload_ready();
  }
"""

with PATH.open("r+", encoding="utf-8") as locked:
    fcntl.flock(locked.fileno(), fcntl.LOCK_EX)
    text = locked.read()
    if OLD not in text:
        raise SystemExit("Unable to find generated readiness helper")
    updated = text.replace(OLD, NEW, 1)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=PATH.parent, delete=False
    ) as temporary:
        temporary.write(updated)
        temporary.flush()
        os.fsync(temporary.fileno())
        temporary_path = Path(temporary.name)
    os.replace(temporary_path, PATH)
