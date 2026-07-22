from __future__ import annotations

import os
import sys
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


CMAKE_PATH = Path("CMakeLists.txt")
LOCK_PATH = CMAKE_PATH.with_suffix(CMAKE_PATH.suffix + ".issue1368.lock")


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
    if len(sys.argv) != 2 or not sys.argv[1].isdigit():
        raise SystemExit("usage: update_issue_1368_rulebox_seed.py READY_SEED")
    seed = int(sys.argv[1])

    old = (
        "# The source-bound audit manifest includes this Rule Box Ability lock scenario in\n"
        "# the six-trace inventory, so CTest must enforce its ready-by deadline as well:\n"
        "# https://github.com/FlareZ123/pokemon-sims/blob/main/results/baseline_manifest.json\n"
        "# https://github.com/FlareZ123/pokemon-sims/issues/917\n"
        "add_test(NAME trace_rulebox_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-rulebox-ability-lock/go-second --seed 20 --require-ready-by 3)\n"
    )
    new = (
        "# The source-bound audit manifest includes this Rule Box Ability lock scenario in\n"
        "# the six-trace inventory, so CTest must enforce its ready-by deadline as well.\n"
        "# Issue #1368 changes Earthen Vessel's legal target count and therefore the\n"
        "# post-search shuffle for historical seed 20. This concrete ready seed is\n"
        "# discovered through the public --find-ready interface during validation, while\n"
        "# seed 20 remains covered by the exact policy regression:\n"
        "# https://github.com/FlareZ123/pokemon-sims/blob/main/results/baseline_manifest.json\n"
        "# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#find-known-good-trace-seeds\n"
        "# https://api.pokemontcg.io/v2/cards/sv4-163\n"
        "# https://api.pokemontcg.io/v2/cards/swsh12-135\n"
        "# https://github.com/FlareZ123/pokemon-sims/issues/917\n"
        "# https://github.com/FlareZ123/pokemon-sims/issues/1368\n"
        f"add_test(NAME trace_rulebox_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-rulebox-ability-lock/go-second --seed {seed} --require-ready-by 3)\n"
    )

    with locked_file(LOCK_PATH):
        text = CMAKE_PATH.read_text(encoding="utf-8")
        if new in text:
            return 0
        if text.count(old) != 1:
            raise RuntimeError(f"Unexpected rulebox trace anchor count: {text.count(old)}")
        atomic_write(CMAKE_PATH, text.replace(old, new, 1))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
