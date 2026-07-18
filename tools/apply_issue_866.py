from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-866.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-866.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_exact(path: Path, old: str, new: str, expected_count: int = 1) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text and old not in text:
        return
    actual_count = text.count(old)
    if actual_count != expected_count:
        raise SystemExit(
            f"issue 866 expected {expected_count} anchor(s) in {path}, found {actual_count}"
        )
    atomic_replace(path, text.replace(old, new))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))

        replace_exact(
            Path("tests/evolution_incense_payload_tests.cpp"),
            "  state.deck = {sim::Card::MegaDragonite, sim::Card::MawileGX};\n",
            """  // Regidrago V is a current modeled Pokémon target, so Ultra Ball still has
  // a legal post-cost search effect in both duplicate- and single-Incense controls:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://github.com/FlareZ123/pokemon-sims/issues/866
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV};
""",
            expected_count=2,
        )

        replace_exact(
            Path("tests/ultra_ball_redundant_cost_tests.cpp"),
            """  state.deck = {Card::MawileGX};
  EngineTestAccess::set_deck_seen(engine);

  // Mawile-GX is a Pokémon, so the known Ultra Ball search remains legal after its
  // two-card requirement is paid: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  assert(EngineTestAccess::play_ultra_ball(engine, true));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MawileGX) == 1);
""",
            """  // Regidrago V is a current modeled Basic Pokémon, so the known Ultra Ball
  // search remains legal after its two-card requirement is paid:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://github.com/FlareZ123/pokemon-sims/issues/866
  state.deck = {Card::RegidragoV};
  EngineTestAccess::set_deck_seen(engine);

  assert(EngineTestAccess::play_ultra_ball(engine, true));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::RegidragoV) == 1);
""",
        )
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
