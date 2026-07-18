from __future__ import annotations

import os
from pathlib import Path


LOCK_PATH = Path(".issue-823-apply.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-823.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 823 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def append_once(path: Path, marker: str, addition: str) -> None:
    text = path.read_text(encoding="utf-8")
    if marker in text:
        return
    if not text.endswith("\n"):
        text += "\n"
    atomic_replace(path, text + addition)


def main() -> None:
    lock_fd = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(lock_fd, str(os.getpid()).encode("ascii"))

        source = Path("src/trace_engine_v2/part_pokemon_communication.inc")
        old_source = """    Engine simulated = *this;
    if (!remove_one(simulated.state_.hand, supporter_leaving_hand)) return false;
    for (const Card restored : restored_cards) simulated.state_.deck.push_back(restored);

    // Recovery Supporters shuffle their selected Pokémon into the deck before the
"""
        new_source = """    Engine simulated = *this;
    if (!remove_one(simulated.state_.hand, supporter_leaving_hand)) return false;
    for (const Card restored : restored_cards) {
      if (!remove_one(simulated.state_.discard, restored)) return false;
      simulated.state_.deck.push_back(restored);
    }

    // Recovery Supporters move each selected Pokémon from discard into the deck.
    // Preserve that exact zone transition in the preflight so K0 fixed-list
    // accounting does not count one singleton in both public zones:
    // Team Yell's Cheer: https://api.pokemontcg.io/v2/cards/swsh9-149
    // Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
    // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
    // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/823
    // Recovery Supporters shuffle their selected Pokémon into the deck before the
"""
        replace_once(source, old_source, new_source)

        cmake = Path("CMakeLists.txt")
        cmake_addition = """

add_executable(regidrago_pokemon_communication_recovery_zone_tests
  tests/pokemon_communication_recovery_zone_tests.cpp)
target_compile_options(regidrago_pokemon_communication_recovery_zone_tests PRIVATE
  -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
add_test(NAME regidrago_pokemon_communication_recovery_zones
  COMMAND regidrago_pokemon_communication_recovery_zone_tests)
"""
        append_once(
            cmake,
            "regidrago_pokemon_communication_recovery_zone_tests",
            cmake_addition,
        )
    finally:
        os.close(lock_fd)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
