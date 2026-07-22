from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


SIM_PATH = Path("src/regidrago_sim.cpp")
LOCK_PATH = SIM_PATH.with_suffix(SIM_PATH.suffix + ".issue1369.lock")


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
        old_celestial = (
            '#undef use_fss\n'
            '#undef use_celestial_roar\n'
            '#include "trace_engine_v2/part_celestial_roar_override.inc"\n'
            '#include "trace_engine_v2/part_legacy_star_projection_provenance_override.inc"\n'
        )
        new_celestial = (
            '#undef use_fss\n'
            '#undef use_celestial_roar\n'
            '#define use_celestial_roar use_celestial_roar_issue1369_original\n'
            '#include "trace_engine_v2/part_celestial_roar_override.inc"\n'
            '#undef use_celestial_roar\n'
            '#include "trace_engine_v2/part_legacy_star_projection_provenance_override.inc"\n'
        )
        old_secret_box = (
            '#include "trace_engine_v2/part_issue_1235_oricorio_treasure_tapu_override.inc"\n'
            '#include "trace_engine_v2/part_issue_1118_secret_box.inc"\n'
            '#define play_field_blower play_field_blower_original\n'
        )
        new_secret_box = (
            '#include "trace_engine_v2/part_issue_1235_oricorio_treasure_tapu_override.inc"\n'
            '#include "trace_engine_v2/part_issue_1118_secret_box.inc"\n'
            '#include "trace_engine_v2/part_issue_1369_celestial_roar_secret_box_override.inc"\n'
            '#define play_field_blower play_field_blower_original\n'
        )
        if new_celestial in text and new_secret_box in text:
            return 0
        if text.count(old_celestial) != 1:
            raise RuntimeError(
                f"Unexpected issue 1369 Celestial Roar anchor count: {text.count(old_celestial)}"
            )
        if text.count(old_secret_box) != 1:
            raise RuntimeError(
                f"Unexpected issue 1369 Secret Box anchor count: {text.count(old_secret_box)}"
            )
        text = text.replace(old_celestial, new_celestial, 1)
        text = text.replace(old_secret_box, new_secret_box, 1)
        atomic_write(SIM_PATH, text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
