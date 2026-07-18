from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path


def atomic_replace(path: Path, old: str, new: str) -> None:
    with path.open("r+", encoding="utf-8") as locked:
        fcntl.flock(locked.fileno(), fcntl.LOCK_EX)
        text = locked.read()
        if new in text:
            return
        if text.count(old) != 1:
            raise SystemExit(f"Unable to find unique issue 869 anchor in {path}")
        updated = text.replace(old, new, 1)
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=path.parent, delete=False
        ) as temporary:
            temporary.write(updated)
            temporary.flush()
            os.fsync(temporary.fileno())
            temporary_path = Path(temporary.name)
        os.replace(temporary_path, path)


HELPER_PATH = Path("src/trace_engine_v2/part_010_late_steven_override.inc")
HELPER_ANCHOR = "  bool late_steven_is_only_basic_connector() const {\n"
HELPER = r'''  bool play_known_steven_t3_payload_outlet() {
    if (state_.turn != 3 || item_locked() || hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::MegaDragonite) == 0 || hand_count(Card::Grass) == 0 ||
        state_.manual_energy_used || !need_payload() || !in_play(Card::LatiasEx) ||
        !ability_available_for_pokemon(Card::LatiasEx)) {
      return false;
    }
    const bool prepared_vstar = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoVstar && pokemon.grass == 1 &&
              pokemon.fire == 1 && pokemon.entered_turn < state_.turn;
        });
    const bool legal_search_target = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic);
    if (!prepared_vstar || !legal_search_target) return false;

    // This exact K1 continuation uses Mysterious Treasure before Tate & Liza can
    // shuffle away the held outlet and payload. The printed Item discards one card,
    // then searches a Psychic or Dragon Pokémon; Mega Dragonite ex is the deliberate
    // Apex Dragon payload and the held Grass plus Latias ex prove the T3 finish:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(Card::MegaDragonite, "Known Steven Treasure payload cost", "R-MT-01");
    record_deck_search_knowledge("Mysterious Treasure");
    const Card target = fallback_mysterious_target_after_search_started();
    const bool found = move_deck_to_hand(target);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-COMPRESS-01",
          found ? "Preserved the known Steven T3 payload route and searched " +
                      std::string(name(target)) + "."
                : "Preserved the known Steven T3 payload route; no legal target remained after search.");
    return true;
  }

'''
atomic_replace(HELPER_PATH, HELPER_ANCHOR, HELPER + HELPER_ANCHOR)

TURN_PATH = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
TURN_OLD = r'''    bench_latias_for_pending_vstar_promotion();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());
'''
TURN_NEW = r'''    bench_latias_for_pending_vstar_promotion();
    play_basics_from_hand();
    // Preserve the exact #869 Mysterious Treasure payload outlet before a generic
    // Supporter route can shuffle it away:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    play_known_steven_t3_payload_outlet();
    play_items_until_stable(!strict_payload_timing());
'''
atomic_replace(TURN_PATH, TURN_OLD, TURN_NEW)
