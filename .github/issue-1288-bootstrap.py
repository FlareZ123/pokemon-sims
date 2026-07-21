from __future__ import annotations

import os
import tempfile
from pathlib import Path


TARGET = Path("src/trace_engine_v2/part_012_override.inc")
LOCK = TARGET.with_suffix(TARGET.suffix + ".issue-1288.lock")
OLD = """        for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                   Card::GoodraVstar, Card::DialgaGX}) {
"""
NEW = """        // Appletun sv8-140 is a Dragon and a modeled Apex Dragon payload, so
        // every non-Quick-Ball costed-search projection must include it:
        // https://api.pokemontcg.io/v2/cards/sm6-113
        // https://api.pokemontcg.io/v2/cards/sv8-140
        // https://api.pokemontcg.io/v2/cards/sv4-163
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://github.com/FlareZ123/pokemon-sims/issues/1288
        for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                   Card::GoodraVstar, Card::DialgaGX,
                                   Card::Appletun}) {
"""


def lock_file(handle: object) -> None:
    if os.name == "nt":
        import msvcrt

        msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
        return

    import fcntl

    fcntl.flock(handle.fileno(), fcntl.LOCK_EX)


def unlock_file(handle: object) -> None:
    if os.name == "nt":
        import msvcrt

        handle.seek(0)
        msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
        return

    import fcntl

    fcntl.flock(handle.fileno(), fcntl.LOCK_UN)


def main() -> None:
    LOCK.parent.mkdir(parents=True, exist_ok=True)
    with LOCK.open("a+b") as lock_handle:
        lock_handle.seek(0)
        lock_file(lock_handle)
        try:
            text = TARGET.read_text(encoding="utf-8")
            if "every non-Quick-Ball costed-search projection must include it" in text:
                return
            if OLD not in text:
                raise RuntimeError("Issue 1288 source anchor was not found.")
            updated = text.replace(OLD, NEW, 1)
            with tempfile.NamedTemporaryFile(
                mode="w",
                encoding="utf-8",
                newline="\n",
                dir=TARGET.parent,
                delete=False,
            ) as temporary:
                temporary.write(updated)
                temporary.flush()
                os.fsync(temporary.fileno())
                temporary_path = Path(temporary.name)
            os.replace(temporary_path, TARGET)
        finally:
            unlock_file(lock_handle)
            LOCK.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
