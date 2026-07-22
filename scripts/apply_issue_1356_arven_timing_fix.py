from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


FSS_PATH = Path("src/trace_engine_v2/part_010_fss_override.inc")
TEST_PATH = Path("tests/issue_1356_fss_treasure_energy_tests.cpp")
LOCK_PATH = FSS_PATH.with_suffix(FSS_PATH.suffix + ".issue1356-timing.lock")


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


def replace_once(text: str, old: str, new: str, description: str) -> str:
    if new in text:
        return text
    if text.count(old) != 1:
        raise RuntimeError(f"Unexpected {description} anchor count: {text.count(old)}")
    return text.replace(old, new, 1)


def main() -> int:
    with locked_file(LOCK_PATH):
        fss = FSS_PATH.read_text(encoding="utf-8")
        fss = replace_once(
            fss,
            "        scenario_.max_turn < 3 || !deck_seen_ ||\n"
            "        !state_.supporter_used || state_.vstar_power_used ||",
            "        scenario_.max_turn < 3 || !deck_seen_ ||\n"
            "        state_.vstar_power_used ||",
            "Forest Seal Stone Arven-resolution timing",
        )
        atomic_write(FSS_PATH, fss)

        tests = TEST_PATH.read_text(encoding="utf-8")
        tests = replace_once(
            tests,
            "  state.supporter_used = true;\n  state.manual_energy_used = true;",
            "  // Forest Seal Stone is evaluated during Arven's resolution, before the\n"
            "  // engine commits the Supporter-slot flag. The public route must be live at\n"
            "  // this exact timing boundary: https://github.com/FlareZ123/pokemon-sims/issues/1356\n"
            "  state.supporter_used = false;\n  state.manual_energy_used = true;",
            "focused Arven timing fixture",
        )
        tests = replace_once(
            tests,
            "void resolve_star_alchemy_for_fire(sim::State& state) {\n"
            "  state.vstar_power_used = true;",
            "void resolve_star_alchemy_for_fire(sim::State& state) {\n"
            "  // Arven has completed before the later Item sequence begins.\n"
            "  // https://api.pokemontcg.io/v2/cards/sv1-166\n"
            "  // https://www.pokemon.com/us/pokemon-tcg/rules\n"
            "  state.supporter_used = true;\n"
            "  state.vstar_power_used = true;",
            "post-Arven fixture transition",
        )
        atomic_write(TEST_PATH, tests)

    LOCK_PATH.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
