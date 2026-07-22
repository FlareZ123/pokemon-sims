from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1339"
SOURCE_PATH = Path("src/trace_engine_v2/part_010_blender_thinning_override.inc")
LOCK_PATH = SOURCE_PATH.with_suffix(SOURCE_PATH.suffix + ".lock")


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
    anchor = "\n".join(
        [
            "    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 ||",
            "        !can_play_payload_this_turn() || !payload_might_be_in_deck() ||",
            "        !blender_energy_axis_can_finish_this_turn() ||",
            "        !blender_active_axis_can_finish_this_turn()) {",
            "      return false;",
            "    }",
        ]
    )
    guard = "\n".join(
        [
            "    // A public Dragon payload plus a legal same-turn hand-discard outlet",
            "    // already completes strict JIT. Preserve the one-copy ACE SPEC and the",
            "    // deck payloads instead of duplicating that axis:",
            "    // https://api.pokemontcg.io/v2/cards/sv8-164",
            "    // https://api.pokemontcg.io/v2/cards/sm6-113",
            "    // https://api.pokemontcg.io/v2/cards/swsh1-179",
            "    // https://api.pokemontcg.io/v2/cards/sv4-163",
            "    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146",
            "    // https://api.pokemontcg.io/v2/cards/swsh12-164",
            "    // https://api.pokemontcg.io/v2/cards/me2pt5-152",
            "    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/",
            "    // https://www.pokemon.com/us/pokemon-tcg/rules",
            "    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/976",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/1339",
            "    const bool payload_already_held =",
            "        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);",
            "    bool live_one_discard_payload_outlet = false;",
            "    if (payload_already_held && deck_search_available()) {",
            "      for (const Card item : {Card::MysteriousTreasure, Card::QuickBall,",
            "                              Card::EarthenVessel}) {",
            "        if (hand_count(item) == 0) continue;",
            "        const bool known_dead_mysterious = item == Card::MysteriousTreasure &&",
            "            deck_seen_ && std::none_of(state_.deck.begin(), state_.deck.end(),",
            "                                      is_dragon_or_psychic);",
            "        const bool known_dead_quick = item == Card::QuickBall && deck_seen_ &&",
            "            std::none_of(state_.deck.begin(), state_.deck.end(), is_basic);",
            "        if (known_dead_mysterious || known_dead_quick ||",
            "            (item == Card::EarthenVessel && !earthen_vessel_has_legal_target())) {",
            "          continue;",
            "        }",
            "        const auto cost = choose_discard(true, true, true, item);",
            "        if (cost && is_payload(*cost)) {",
            "          live_one_discard_payload_outlet = true;",
            "          break;",
            "        }",
            "      }",
            "    }",
            "    bool live_ultra_ball_payload_outlet = false;",
            "    if (payload_already_held && hand_count(Card::UltraBall) > 0 &&",
            "        ultra_ball_has_legal_target()) {",
            "      Engine simulated = *this;",
            "      const auto payload_cost =",
            "          simulated.choose_discard(true, true, true, Card::UltraBall);",
            "      if (payload_cost && is_payload(*payload_cost) &&",
            "          remove_one(simulated.state_.hand, *payload_cost)) {",
            "        live_ultra_ball_payload_outlet =",
            "            simulated.choose_discard(false, true, true, Card::UltraBall).has_value();",
            "      }",
            "    }",
            "    const bool live_serena_payload_outlet = payload_already_held &&",
            "        supporter_allowed() && hand_count(Card::Serena) > 0;",
            "    if (live_one_discard_payload_outlet || live_ultra_ball_payload_outlet ||",
            "        live_serena_payload_outlet) {",
            "      return false;",
            "    }",
        ]
    )

    with locked_file(LOCK_PATH):
        text = SOURCE_PATH.read_text(encoding="utf-8")
        if ISSUE_URL in text:
            return 0
        if text.count(anchor) != 1:
            raise RuntimeError("Active Brilliant Blender selector changed unexpectedly")
        atomic_write(SOURCE_PATH, text.replace(anchor, anchor + "\n\n" + guard, 1))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
