from __future__ import annotations

import os
import time
from pathlib import Path


SOURCE = Path("src/trace_engine_v2/part_forretress_ex_combo.inc")
LOCK = SOURCE.with_suffix(SOURCE.suffix + ".issue-1329.lock")

OLD = '''  // Choosing to use the Ability at K0 relies only on fixed-list public-zone
  // accounting. The exact remaining Grass count becomes available only after
  // the printed deck search begins:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/1118
  record_deck_search_knowledge("Exploding Energy");
  const int available = std::min(5, deck_count_after_search_started(Card::Grass));
  if (available <= 0) return false;

  std::vector<std::size_t> destinations;
  const bool source_is_active =
      state_.active && state_.active->card == Card::ForretressEx;
  const bool active_blocks_regi = !source_is_active && state_.active &&
      state_.active->card != Card::RegidragoV &&
      state_.active->card != Card::RegidragoVstar;
  const bool retreat_available = active_blocks_regi && !state_.retreat_used;
  // Skyliner gives every Basic Pokémon in play no Retreat Cost while Latias ex's
  // Rule Box Ability is available. Apply that effective cost before distributing
  // Exploding Energy so a free retreat never consumes a searched Grass Energy:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  const bool skyliner_free_retreat = retreat_available &&
      can_free_retreat_with_latias();
  const int active_retreat_cost = retreat_available && !skyliner_free_retreat
      ? retreat_cost(state_.active->card) : 0;
  const int target_grass_needed = std::max(0, grass_needed());

  // Exploding Energy may distribute its five Grass among any Pokémon. When a
  // non-Regidrago Active blocks a Benched setup target, pay that printed Retreat
  // Cost from the Ability while retaining enough Grass on Regidrago. This models
  // the real T2 Tapu, Pineco, and Dialga recovery line rather than leaving a fully
  // powered VSTAR stranded on the Bench:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1118
  const bool can_complete_retreat = retreat_available &&
      (active_retreat_cost == 0 ||
       available >= active_retreat_cost + target_grass_needed);
  if (can_complete_retreat && active_retreat_cost > 0) {
    destinations.insert(destinations.end(),
                        static_cast<std::size_t>(active_retreat_cost), 0U);
  }
  destinations.insert(destinations.end(),
                      static_cast<std::size_t>(available -
                          (can_complete_retreat ? active_retreat_cost : 0)),
                      *target_index);
'''

NEW = '''  std::vector<std::size_t> destinations;
  const bool source_is_active =
      state_.active && state_.active->card == Card::ForretressEx;
  const bool active_blocks_regi = !source_is_active && state_.active &&
      state_.active->card != Card::RegidragoV &&
      state_.active->card != Card::RegidragoVstar;
  const bool retreat_available = active_blocks_regi && !state_.retreat_used;
  // Skyliner gives every Basic Pokémon in play no Retreat Cost while Latias ex's
  // Rule Box Ability is available. Apply that effective cost before distributing
  // Exploding Energy so a free retreat never consumes a searched Grass Energy:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  const bool skyliner_free_retreat = retreat_available &&
      can_free_retreat_with_latias();
  const int active_retreat_cost = retreat_available && !skyliner_free_retreat
      ? retreat_cost(state_.active->card) : 0;
  const int target_grass_needed = std::max(0, grass_needed());
  const bool paid_retreat_route = retreat_available && active_retreat_cost > 0;
  if (target_grass_needed <= 0 && !paid_retreat_route) return false;

  // Choosing to use the Ability at K0 relies only on fixed-list public-zone
  // accounting. The exact remaining Grass count becomes available only after
  // the printed deck search begins:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/1118
  record_deck_search_knowledge("Exploding Energy");
  const int available = std::min(5, deck_count_after_search_started(Card::Grass));
  if (available <= 0) return false;

  // Exploding Energy searches for up to five Grass Energy, so select only the
  // copies that advance the current Grass and paid-Retreat axes. Extra copies do
  // not improve the earliest ready turn and remain in deck for later attackers:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  const bool can_complete_retreat = retreat_available &&
      (active_retreat_cost == 0 ||
       available >= active_retreat_cost + target_grass_needed);
  const int retreat_grass = can_complete_retreat ? active_retreat_cost : 0;
  const int target_grass = std::min(
      target_grass_needed, available - retreat_grass);
  if (retreat_grass + target_grass <= 0) return false;
  if (retreat_grass > 0) {
    destinations.insert(destinations.end(),
                        static_cast<std::size_t>(retreat_grass), 0U);
  }
  destinations.insert(destinations.end(),
                      static_cast<std::size_t>(target_grass), *target_index);
'''


def acquire_lock() -> int:
    for _ in range(200):
        try:
            return os.open(LOCK, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        except FileExistsError:
            time.sleep(0.05)
    raise RuntimeError(f"could not acquire lock: {LOCK}")


def main() -> int:
    fd = acquire_lock()
    try:
        text = SOURCE.read_text(encoding="utf-8")
        if NEW in text:
            return 0
        if text.count(OLD) != 1:
            raise RuntimeError("issue #1329 source block changed unexpectedly")
        updated = text.replace(OLD, NEW, 1)
        temporary = SOURCE.with_suffix(SOURCE.suffix + ".tmp")
        temporary.write_text(updated, encoding="utf-8")
        os.replace(temporary, SOURCE)
        return 0
    finally:
        os.close(fd)
        LOCK.unlink(missing_ok=True)


if __name__ == "__main__":
    raise SystemExit(main())
