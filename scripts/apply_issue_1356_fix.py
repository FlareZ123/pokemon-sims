from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1356"
FSS_PATH = Path("src/trace_engine_v2/part_010_fss_override.inc")
MAIN_PATH = Path("src/regidrago_sim.cpp")
TREASURE_OVERRIDE_PATH = Path(
    "src/trace_engine_v2/part_issue_1356_fss_treasure_energy_override.inc"
)
LOCK_PATH = FSS_PATH.with_suffix(FSS_PATH.suffix + ".lock")


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


def fss_helper_block() -> str:
    return "\n".join(
        [
            "  bool fss_should_split_treasure_vstar_and_next_turn_energy() const {",
            "    if (scenario_.dci != DciProfile::StrictJit ||",
            "        scenario_.locks != LockMode::None || state_.turn != 2 ||",
            "        scenario_.max_turn < 3 || !deck_seen_ ||",
            "        !state_.supporter_used || state_.vstar_power_used ||",
            "        !state_.manual_energy_used || item_locked() || !state_.active ||",
            "        state_.active->card != Card::RegidragoV ||",
            "        state_.active->entered_turn >= state_.turn ||",
            "        state_.active->tool != Tool::ForestSealStone ||",
            "        hand_count(Card::MysteriousTreasure) < 2 ||",
            "        hand_count(Card::RegidragoVstar) > 0 ||",
            "        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||",
            "        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload)) {",
            "      return false;",
            "    }",
            "",
            "    const bool star_takes_fire =",
            "        state_.active->grass >= 2 && state_.active->fire == 0 &&",
            "        hand_count(Card::Grass) > 0 &&",
            "        deck_count_after_search_started(Card::Fire) > 0;",
            "    const bool star_takes_grass =",
            "        state_.active->grass == 1 && state_.active->fire >= 1 &&",
            "        hand_count(Card::Fire) > 0 &&",
            "        deck_count_after_search_started(Card::Grass) > 0;",
            "",
            "    // Arven has supplied the second Treasure and Forest Seal Stone. The",
            "    // prior-turn Active has exactly one GGF type missing, Star Alchemy can",
            "    // search that type, and the opposite held Energy is surplus for this",
            "    // attacker. The later scoped Treasure policy spends only that surplus",
            "    // Energy, searches Regidrago VSTAR, and preserves the second Treasure",
            "    // plus the held Dragon for the strict-JIT T3 payload:",
            "    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166",
            "    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113",
            "    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156",
            "    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179",
            "    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55",
            "    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136",
            "    // Official Item, VSTAR Power, search, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules",
            "    // Dynamic DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356",
            "    return star_takes_fire || star_takes_grass;",
            "  }",
            "",
        ]
    )


def fss_target_block() -> str:
    return "\n".join(
        [
            "    if (fss_should_take_crispin_over_redundant_blender()) return Card::Crispin;",
            "",
            "    if (fss_should_split_treasure_vstar_and_next_turn_energy()) {",
            "      // Star Alchemy searches any card. Treasure supplies the Dragon VSTAR axis,",
            "      // so preserve Quick Ball, Oricorio, the Bench slot, and the second Treasure",
            "      // by assigning Forest Seal Stone to the missing next-turn Basic Energy:",
            "      // https://api.pokemontcg.io/v2/cards/swsh12-156",
            "      // https://api.pokemontcg.io/v2/cards/sm6-113",
            "      // https://api.pokemontcg.io/v2/cards/sm2-55",
            "      // https://api.pokemontcg.io/v2/cards/swsh1-179",
            "      // https://github.com/FlareZ123/pokemon-sims/issues/1356",
            "      if (grass_needed() == 1) return Card::Grass;",
            "      return Card::Fire;",
            "    }",
            "",
            "    const Card original = fss_target_after_search_started_original();",
        ]
    )


def treasure_override() -> str:
    return """  bool issue_1356_complete_next_turn_route_holds_quick_ball() const {
    if (scenario_.dci != DciProfile::StrictJit || scenario_.locks != LockMode::None ||
        state_.turn != 2 || !deck_seen_ || !state_.supporter_used ||
        !state_.manual_energy_used || !state_.vstar_power_used || item_locked() ||
        !state_.active || state_.active->entered_turn >= state_.turn ||
        (state_.active->card != Card::RegidragoV &&
         state_.active->card != Card::RegidragoVstar) ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const bool vstar_axis_complete = active_is_vstar() ||
        hand_count(Card::RegidragoVstar) > 0;
    const bool fire_is_held_next_attachment =
        state_.active->grass >= 2 && state_.active->fire == 0 &&
        hand_count(Card::Fire) > 0;
    const bool grass_is_held_next_attachment =
        state_.active->grass == 1 && state_.active->fire >= 1 &&
        hand_count(Card::Grass) > 0;
    const bool pending_treasure_vstar_axis =
        hand_count(Card::MysteriousTreasure) >= 2 &&
        hand_count(Card::RegidragoVstar) == 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        ((fire_is_held_next_attachment && hand_count(Card::Grass) > 0) ||
         (grass_is_held_next_attachment && hand_count(Card::Fire) > 0));

    // The public T3 route is complete or has a fully payable Treasure-to-VSTAR
    // bridge: the sole missing Basic Energy is held, the opposite held Energy pays
    // Treasure when VSTAR is still in deck, and another Treasure plus the held Dragon
    // remain for the ready turn. Quick Ball into Oricorio spends an extra Item, an
    // extra discard, a Bench slot, and a support Pokémon without improving T3:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Bench, Ability, evolution, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1335 https://github.com/FlareZ123/pokemon-sims/issues/1356
    return (vstar_axis_complete || pending_treasure_vstar_axis) &&
        (fire_is_held_next_attachment || grass_is_held_next_attachment);
  }

  bool play_quick_ball(const bool permit_payload) {
    if (issue_1356_complete_next_turn_route_holds_quick_ball()) return false;
    return play_quick_ball_issue1356_original(permit_payload);
  }

  bool issue_1356_treasure_vstar_energy_split_available() const {
    if (scenario_.dci != DciProfile::StrictJit || scenario_.locks != LockMode::None ||
        state_.turn != 2 || scenario_.max_turn < 3 || !deck_seen_ ||
        !state_.supporter_used || !state_.manual_energy_used ||
        !state_.vstar_power_used || item_locked() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::MysteriousTreasure) < 2 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const bool fire_is_next_attachment =
        state_.active->grass >= 2 && state_.active->fire == 0 &&
        hand_count(Card::Fire) > 0 && hand_count(Card::Grass) > 0;
    const bool grass_is_next_attachment =
        state_.active->grass == 1 && state_.active->fire >= 1 &&
        hand_count(Card::Grass) > 0 && hand_count(Card::Fire) > 0;

    // Arven has already supplied the second Treasure and Forest Seal Stone. Star
    // Alchemy has supplied the sole missing next-turn Energy. The opposite Energy
    // type is now surplus for this attacker, while preserving the second Treasure
    // retains the strict-JIT Dragon outlet and avoids Quick Ball plus Oricorio:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, search, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356
    return fire_is_next_attachment || grass_is_next_attachment;
  }

  bool play_issue_1356_treasure_vstar_energy_split() {
    if (!issue_1356_treasure_vstar_energy_split_available()) return false;

    const Card surplus_energy = state_.active->fire == 0 ? Card::Grass : Card::Fire;
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(surplus_energy, "Mysterious Treasure split-route cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-1356 surplus Energy cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::RegidragoVstar)) {
      throw std::logic_error("projected issue-1356 Regidrago VSTAR target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          "Discarded the surplus off-type Energy and searched Regidrago VSTAR; preserved the second Treasure for the T3 Dragon payload.");
    return true;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (play_issue_1356_treasure_vstar_energy_split()) return true;
    return play_mysterious_treasure_issue1356_original(permit_payload);
  }
"""


def main() -> int:
    helper_anchor = "\n  Card fss_target_after_search_started() const {"
    target_anchor = "\n".join(
        [
            "    if (fss_should_take_crispin_over_redundant_blender()) return Card::Crispin;",
            "",
            "    const Card original = fss_target_after_search_started_original();",
        ]
    )
    include_anchor = "\n".join(
        [
            '#include "trace_engine_v2/part_issue_1235_oricorio_treasure_tapu_override.inc"',
            '#include "trace_engine_v2/part_issue_1118_secret_box.inc"',
        ]
    )
    include_block = "\n".join(
        [
            '#define play_quick_ball play_quick_ball_issue1356_original',
            '#define play_mysterious_treasure play_mysterious_treasure_issue1356_original',
            '#include "trace_engine_v2/part_issue_1235_oricorio_treasure_tapu_override.inc"',
            '#undef play_mysterious_treasure',
            '#undef play_quick_ball',
            '#include "trace_engine_v2/part_issue_1356_fss_treasure_energy_override.inc"',
            '#include "trace_engine_v2/part_issue_1118_secret_box.inc"',
        ]
    )

    with locked_file(LOCK_PATH):
        fss = FSS_PATH.read_text(encoding="utf-8")
        if ISSUE_URL not in fss:
            if fss.count(helper_anchor) != 1:
                raise RuntimeError("Forest Seal Stone override helper anchor changed unexpectedly")
            if fss.count(target_anchor) != 1:
                raise RuntimeError("Forest Seal Stone target anchor changed unexpectedly")
            fss = fss.replace(helper_anchor, "\n" + fss_helper_block() + helper_anchor.lstrip("\n"), 1)
            fss = fss.replace(target_anchor, fss_target_block(), 1)
            atomic_write(FSS_PATH, fss)

        main_source = MAIN_PATH.read_text(encoding="utf-8")
        if "part_issue_1356_fss_treasure_energy_override.inc" not in main_source:
            if main_source.count(include_anchor) != 1:
                raise RuntimeError("Mysterious Treasure final-wrapper anchor changed unexpectedly")
            main_source = main_source.replace(include_anchor, include_block, 1)
            atomic_write(MAIN_PATH, main_source)

        atomic_write(TREASURE_OVERRIDE_PATH, treasure_override())

    LOCK_PATH.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
