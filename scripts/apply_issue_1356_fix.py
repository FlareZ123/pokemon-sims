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
            "    if (!deck_seen_ || state_.vstar_power_used || item_locked() ||",
            "        !strict_payload_timing() || state_.turn >= scenario_.max_turn ||",
            "        !state_.manual_energy_used || !need_vstar() || !need_energy() ||",
            "        grass_needed() + fire_needed() != 1 || !state_.active ||",
            "        state_.active->card != Card::RegidragoV ||",
            "        state_.active->entered_turn >= state_.turn ||",
            "        hand_count(Card::MysteriousTreasure) < 2 ||",
            "        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||",
            "        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload)) {",
            "      return false;",
            "    }",
            "",
            "    const Card missing_energy = grass_needed() == 1 ? Card::Grass : Card::Fire;",
            "    if (deck_count_after_search_started(missing_energy) == 0) return false;",
            "",
            "    // Star Alchemy resolves before the later Item sequence. Project that exact",
            "    // public order by moving the missing Basic Energy to hand first, then run the",
            "    // final Mysterious Treasure policy. The required T3 Energy stays protected,",
            "    // and the projected DCI must preserve one Treasure plus the held Dragon:",
            "    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166",
            "    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113",
            "    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156",
            "    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136",
            "    // Official turn, Item, search, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules",
            "    // Route priority and strict-JIT DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356",
            "    const std::size_t held_payloads_before = static_cast<std::size_t>(std::count_if(",
            "        state_.hand.begin(), state_.hand.end(), is_payload));",
            "    std::mt19937_64 shadow_rng = rng_;",
            "    Engine projected(scenario_, recipe_, shadow_rng);",
            "    projected.state_ = state_;",
            "    projected.deck_seen_ = deck_seen_;",
            "    projected.prizes_revealed_ = prizes_revealed_;",
            "    projected.state_.vstar_power_used = true;",
            "    if (!projected.move_deck_to_hand(missing_energy) ||",
            "        !projected.play_mysterious_treasure(false)) {",
            "      return false;",
            "    }",
            "    const std::size_t held_payloads_after = static_cast<std::size_t>(std::count_if(",
            "        projected.state_.hand.begin(), projected.state_.hand.end(), is_payload));",
            "    return projected.hand_count(Card::RegidragoVstar) > 0 &&",
            "        projected.hand_count(Card::MysteriousTreasure) > 0 &&",
            "        projected.hand_count(missing_energy) > 0 &&",
            "        held_payloads_after == held_payloads_before;",
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
    return """  bool issue_1356_treasure_vstar_energy_split_available() const {
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
            '#include "trace_engine_v2/part_issue_1235_oricorio_treasure_tapu_override.inc"',
            '#define play_mysterious_treasure play_mysterious_treasure_issue1356_original',
            '#include "trace_engine_v2/part_issue_1356_fss_treasure_energy_override.inc"',
            '#undef play_mysterious_treasure',
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
