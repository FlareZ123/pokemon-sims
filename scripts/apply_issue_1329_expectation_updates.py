from __future__ import annotations

import os
import time
from pathlib import Path


UPDATES = {
    Path("tests/forretress_ex_combo_tests.cpp"): (
        (
            '''  const sim::State& unlocked = sim::EngineTestAccess::state(locked.engine);
  if (!unlocked.path_lock_removed || unlocked.stadium != sim::Stadium::ForestOfVitality ||
      !unlocked.active || unlocked.active->grass != 5 || unlocked.active->fire != 1 ||
      count(unlocked.discard, sim::Card::ForretressEx) != 1) {
    throw std::runtime_error("Forest replacement did not unlock and complete Exploding Energy correctly.");
  }
''',
            '''  const sim::State& unlocked = sim::EngineTestAccess::state(locked.engine);
  // Forest removes the modeled lock, then Exploding Energy selects only the two
  // Grass needed beside held Fire and leaves the other three in deck:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  if (!unlocked.path_lock_removed || unlocked.stadium != sim::Stadium::ForestOfVitality ||
      !unlocked.active || unlocked.active->grass != 2 || unlocked.active->fire != 1 ||
      count(unlocked.deck, sim::Card::Grass) != 3 ||
      count(unlocked.discard, sim::Card::ForretressEx) != 1) {
    throw std::runtime_error("Forest replacement did not unlock the minimal Exploding Energy route.");
  }
''',
        ),
        (
            '''  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.supporter_used || after.stadium != sim::Stadium::ForestOfVitality ||
      !after.active || after.active->grass != 5 || after.active->fire != 1 ||
      !contains(after.hand, sim::Card::MegaDragonite) ||
      count(after.discard, sim::Card::Dawn) != 1 ||
      count(after.discard, sim::Card::Pineco) != 1 ||
      count(after.discard, sim::Card::ForretressEx) != 1) {
    throw std::runtime_error("Dawn, Forest, or Exploding Energy produced the wrong exact state.");
  }
''',
            '''  const sim::State& after = sim::EngineTestAccess::state(engine);
  // Dawn and Forest still complete the route while Exploding Energy selects the
  // two missing Grass and preserves the other three for later attackers:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  if (!after.supporter_used || after.stadium != sim::Stadium::ForestOfVitality ||
      !after.active || after.active->grass != 2 || after.active->fire != 1 ||
      count(after.deck, sim::Card::Grass) != 3 ||
      !contains(after.hand, sim::Card::MegaDragonite) ||
      count(after.discard, sim::Card::Dawn) != 1 ||
      count(after.discard, sim::Card::Pineco) != 1 ||
      count(after.discard, sim::Card::ForretressEx) != 1) {
    throw std::runtime_error("Dawn, Forest, or minimal Exploding Energy produced the wrong exact state.");
  }
''',
        ),
    ),
    Path("tests/issue_1248_latias_skyliner_forretress_tests.cpp"): (
        (
            '''  // Latias ex sv8-76 is a Basic Pokémon with printed Retreat Cost 2, while
  // Skyliner gives the player's Basic Pokémon in play no Retreat Cost. Exploding
  // Energy may therefore attach all three Grass to Regidrago and retreat for free:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
''',
            '''  // Latias ex sv8-76 is a Basic Pokémon with printed Retreat Cost 2, while
  // Skyliner gives the player's Basic Pokémon in play no Retreat Cost. Exploding
  // Energy therefore selects only Regidrago's two missing Grass and preserves one:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
''',
        ),
        (
            '''  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.active->grass != 3 || after.active->fire != 1 ||
      count(after.discard, sim::Card::Grass) != 0 || !after.retreat_used) {
    throw std::runtime_error("Skyliner did not preserve all three searched Grass during the free retreat.");
  }
''',
            '''  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.active->grass != 2 || after.active->fire != 1 ||
      count(after.deck, sim::Card::Grass) != 1 ||
      count(after.discard, sim::Card::Grass) != 0 || !after.retreat_used) {
    throw std::runtime_error("Skyliner did not preserve the unneeded searched Grass during the free retreat.");
  }
''',
        ),
    ),
}


def acquire_lock(path: Path) -> tuple[int, Path]:
    lock = path.with_suffix(path.suffix + ".issue-1329.lock")
    for _ in range(200):
        try:
            return os.open(lock, os.O_CREAT | os.O_EXCL | os.O_WRONLY), lock
        except FileExistsError:
            time.sleep(0.05)
    raise RuntimeError(f"could not acquire lock: {lock}")


def update(path: Path, replacements: tuple[tuple[str, str], ...]) -> None:
    descriptor, lock = acquire_lock(path)
    try:
        text = path.read_text(encoding="utf-8")
        for old, new in replacements:
            if new in text:
                continue
            if text.count(old) != 1:
                raise RuntimeError(f"issue #1329 expectation changed unexpectedly: {path}")
            text = text.replace(old, new, 1)
        temporary = path.with_suffix(path.suffix + ".tmp")
        temporary.write_text(text, encoding="utf-8")
        os.replace(temporary, path)
    finally:
        os.close(descriptor)
        lock.unlink(missing_ok=True)


def main() -> int:
    for path, replacements in UPDATES.items():
        update(path, replacements)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
