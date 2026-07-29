#!/usr/bin/env python3
from __future__ import annotations

import fcntl
import os
import tempfile
from contextlib import contextmanager
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


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
    with locked_path(path):
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            newline="\n",
            dir=path.parent,
            prefix=f".{path.name}.",
            delete=False,
        ) as handle:
            handle.write(content)
            handle.flush()
            os.fsync(handle.fileno())
            temporary_path = Path(handle.name)
        os.replace(temporary_path, path)


def replace_once(path: Path, old: str, new: str) -> None:
    content = path.read_text(encoding="utf-8")
    if new in content:
        return
    if content.count(old) != 1:
        raise RuntimeError(f"Expected exactly one composition marker in {path}")
    atomic_write(path, content.replace(old, new, 1))


def main() -> int:
    replace_once(
        ROOT / "src/trace_engine_v2/part_014c.inc",
        '#undef run_turn\n#include "part_014c_issue_1152_bridge.inc"',
        '#undef run_turn\n#include "part_issue_1821_oricorio_steven_latias_override.inc"\n#include "part_014c_issue_1152_bridge.inc"',
    )
    replace_once(
        ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc",
        "    play_basics_from_hand();\n    play_items_until_stable(!strict_payload_timing());\n\n    attach_manual();",
        "    play_basics_from_hand();\n"
        "    play_items_until_stable(!strict_payload_timing());\n\n"
        "    // Resolve the confirmed K1 Oricorio retreat and Steven package before the\n"
        "    // generic attachment and Supporter selectors can consume either channel:\n"
        "    // https://api.pokemontcg.io/v2/cards/sm2-55\n"
        "    // https://api.pokemontcg.io/v2/cards/sm7-145\n"
        "    // https://github.com/FlareZ123/pokemon-sims/issues/1821\n"
        "    if (play_issue_1821_oricorio_steven_route()) {\n"
        "      trace(\"TURN END\", \"R-STEVEN-01; R-GAME-RETREAT\", state_line());\n"
        "      return;\n"
        "    }\n\n"
        "    attach_manual();",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
