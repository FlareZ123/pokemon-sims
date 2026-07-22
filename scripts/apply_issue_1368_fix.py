from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


SIM_PATH = Path("src/regidrago_sim.cpp")
LOCK_PATH = SIM_PATH.with_suffix(SIM_PATH.suffix + ".issue1368.lock")


@contextmanager
def locked_file(path: Path) -> Iterator[TextIO]:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = path.open("a+", encoding="utf-8")
    try:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        yield handle
    finally:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
        handle.close()


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def main() -> int:
    with locked_file(LOCK_PATH):
        text = SIM_PATH.read_text(encoding="utf-8")
        old = (
            '#include "trace_engine_v2/part_blender_vstar_axis_override.inc"\n'
            '#include "trace_engine_v2/part_earthen_vessel_vstar_window_override.inc"\n'
            '#define play_mysterious_treasure play_mysterious_treasure_issue1209_original\n'
        )
        new = (
            '#include "trace_engine_v2/part_blender_vstar_axis_override.inc"\n'
            '#define play_earthen_vessel play_earthen_vessel_issue1368_original\n'
            '#include "trace_engine_v2/part_earthen_vessel_vstar_window_override.inc"\n'
            '#undef play_earthen_vessel\n'
            '#include "trace_engine_v2/part_issue_1368_earthen_vessel_celestial_roar_override.inc"\n'
            '#define play_mysterious_treasure play_mysterious_treasure_issue1209_original\n'
        )
        if new in text:
            return 0
        if text.count(old) != 1:
            raise RuntimeError(f"Unexpected issue 1368 include anchor count: {text.count(old)}")
        atomic_write(SIM_PATH, text.replace(old, new, 1))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
