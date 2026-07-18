from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-854.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-854.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 854 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))

        replace_once(
            Path("src/trace_engine_v2/part_014b.inc"),
            """    for (const Card fallback : {Card::ChaoticSwell, Card::PathToPeak, Card::TeamYellsCheer,
                                Card::RoseannesBackup, Card::ProfessorTuro, Card::Channeler,
                                Card::Guzma, Card::Serena, Card::TateLiza, Card::StevensResolve,
                                Card::Arven, Card::Crispin, Card::ProfessorBurnet, Card::Gladion}) {
""",
            """    // Erika's Invitation is a Supporter and the canonical low-impact replacement
    // for retired Mawile-GX. Once an advancing target makes Lusamine worth playing,
    // Erika may satisfy the printed mandatory second Supporter/Stadium recovery:
    // https://api.pokemontcg.io/v2/cards/sm4-96
    // https://api.pokemontcg.io/v2/cards/sv3pt5-160
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
    // https://github.com/FlareZ123/pokemon-sims/issues/854
    for (const Card fallback : {Card::ErikasInvitation, Card::ChaoticSwell, Card::PathToPeak,
                                Card::TeamYellsCheer, Card::RoseannesBackup, Card::ProfessorTuro,
                                Card::Channeler, Card::Guzma, Card::Serena, Card::TateLiza,
                                Card::StevensResolve, Card::Arven, Card::Crispin,
                                Card::ProfessorBurnet, Card::Gladion}) {
""",
        )

        replace_once(
            Path("CMakeLists.txt"),
            """add_executable(regidrago_lusamine_arven_item_lock_tests tests/lusamine_arven_item_lock_tests.cpp)
target_compile_options(regidrago_lusamine_arven_item_lock_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
""",
            """add_executable(regidrago_lusamine_arven_item_lock_tests tests/lusamine_arven_item_lock_tests.cpp)
target_compile_options(regidrago_lusamine_arven_item_lock_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_lusamine_erika_fallback_tests tests/lusamine_erika_fallback_tests.cpp)
target_compile_options(regidrago_lusamine_erika_fallback_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
""",
        )

        replace_once(
            Path("CMakeLists.txt"),
            "add_test(NAME regidrago_lusamine_arven_item_lock COMMAND regidrago_lusamine_arven_item_lock_tests)\n",
            """add_test(NAME regidrago_lusamine_arven_item_lock COMMAND regidrago_lusamine_arven_item_lock_tests)
add_test(NAME regidrago_lusamine_erika_fallback COMMAND regidrago_lusamine_erika_fallback_tests)
""",
        )
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
