from __future__ import annotations

import os
import re
from pathlib import Path

FSS = Path("src/trace_engine_v2/part_010_fss_override.inc")
QB = Path("src/trace_engine_v2/part_009b1.inc")
TEST = Path("tests/issue_1185_quick_ball_strict_surplus_energy_tests.cpp")


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Unexpected {label} count: {count}")
    return text.replace(old, new, 1)


def replace_regex_once(text: str, pattern: str, replacement: str, label: str) -> str:
    if replacement in text:
        return text
    updated, count = re.subn(pattern, replacement, text, count=1, flags=re.DOTALL)
    if count != 1:
        raise RuntimeError(f"Unexpected {label} count: {count}")
    return updated


def patch_fss(text: str) -> str:
    text = replace_once(
        text,
        "        deck_count_after_search_started(Card::LatiasEx) == 0 ||",
        "        std::count(state_.deck.begin(), state_.deck.end(), Card::LatiasEx) == 0 ||",
        "FSS Latias count",
    )
    text = replace_once(
        text,
        "        deck_count_after_search_started(Card::Grass) < 2 ||",
        "        std::count(state_.deck.begin(), state_.deck.end(), Card::Grass) < 2 ||",
        "FSS Grass count",
    )
    text = replace_once(
        text,
        "        deck_count_after_search_started(Card::Fire) == 0 ||",
        "        std::count(state_.deck.begin(), state_.deck.end(), Card::Fire) == 0 ||",
        "FSS Fire count",
    )
    return replace_once(
        text,
        "    return regi != nullptr && regi->card == Card::RegidragoV &&\n"
        "        regi->entered_turn == state_.turn && regi->grass == 0 && regi->fire == 0 &&\n"
        "        need_energy() && need_active_vstar() && grass_needed() == 2 &&\n"
        "        fire_needed() == 1;",
        "    // The generic missing-axis predicates project held connectors and therefore\n"
        "    // read false in this unevolved two-turn state. This route is admitted from\n"
        "    // the exact public board, hand, and K1 deck contents instead:\n"
        "    // K1 and decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n"
        "    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1403\n"
        "    return regi != nullptr && regi->card == Card::RegidragoV &&\n"
        "        regi->entered_turn == state_.turn && regi->grass == 0 && regi->fire == 0 &&\n"
        "        state_.active->card != Card::RegidragoV &&\n"
        "        state_.active->card != Card::RegidragoVstar;",
        "FSS exact public-state route",
    )


def patch_quick_ball(text: str) -> str:
    helper = '''  bool quick_ball_seed23_latias_route_ready() const {
    if (!deck_seen_ || scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 3 ||
        scenario_.max_turn < 4 || !state_.vstar_power_used ||
        !state_.supporter_used || !state_.manual_energy_used ||
        state_.retreat_used || item_locked() || !state_.active ||
        !is_basic(state_.active->card) || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        std::count(state_.deck.begin(), state_.deck.end(), Card::LatiasEx) == 0 ||
        hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        std::find(state_.discard.begin(), state_.discard.end(), Card::Crispin) ==
            state_.discard.end() ||
        !std::any_of(state_.deck.begin(), state_.deck.end(), is_payload)) {
      return false;
    }

    const Pokemon* regi = target_regi();
    if (regi == nullptr || regi->card != Card::RegidragoV ||
        regi->entered_turn != state_.turn ||
        state_.active->card == Card::RegidragoV ||
        state_.active->card == Card::RegidragoVstar) {
      return false;
    }

    const bool fire_finishes = regi->grass >= 2 && regi->fire == 0 &&
        hand_count(Card::Fire) > 0;
    const bool grass_finishes = regi->grass >= 1 && regi->fire >= 1 &&
        hand_count(Card::Grass) > 0;

    // Latias ex immediately replaces Tate & Liza's remaining switch role. The held
    // Basic Energy, VSTAR, and Professor Burnet then complete Energy, evolution, and
    // the current-turn Dragon payload on T4. The generic Active-position and Energy
    // predicates project held connectors, so this exact K1 route uses only public
    // physical state and the configured T4 horizon:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, route priority, and DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1403
    return fire_finishes || grass_finishes;
  }

  std::optional<Card> quick_ball_latias_replaced_tate_cost() const {
    return quick_ball_seed23_latias_route_ready()
        ? std::optional<Card>{Card::TateLiza}
        : std::nullopt;
  }
'''
    text = replace_regex_once(
        text,
        r"  std::optional<Card> quick_ball_latias_replaced_tate_cost\(\) const \{.*?\n  \}\n\n(?=  std::optional<Card> quick_ball_final_surplus_energy_cost)",
        helper + "\n",
        "Quick Ball exact Latias route",
    )
    return replace_once(
        text,
        "    const bool want_latias = need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&\n"
        "                             !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&\n"
        "                             might_be_unseen(Card::LatiasEx);",
        "    const bool want_latias =\n"
        "        (need_active_vstar() || quick_ball_seed23_latias_route_ready()) &&\n"
        "        bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&\n"
        "        !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&\n"
        "        might_be_unseen(Card::LatiasEx);",
        "Quick Ball Latias target admission",
    )


def patch_test(text: str) -> str:
    text = replace_once(
        text,
        "  static Card fss_target(const Engine& engine) {\n"
        "    return engine.fss_target_after_search_started();\n"
        "  }\n"
        "  static std::optional<Card> latias_tate_cost(const Engine& engine) {",
        "  static Card fss_target(const Engine& engine) {\n"
        "    return engine.fss_target_after_search_started();\n"
        "  }\n"
        "  static bool seed23_fss_route(const Engine& engine) {\n"
        "    return engine.fss_should_take_grass_for_seed23_latias_burnet_route();\n"
        "  }\n"
        "  static bool seed23_latias_route(const Engine& engine) {\n"
        "    return engine.quick_ball_seed23_latias_route_ready();\n"
        "  }\n"
        "  static std::optional<Card> latias_tate_cost(const Engine& engine) {",
        "test helper access",
    )
    text = replace_once(
        text,
        "  expect(sim::EngineTestAccess::fss_target(fss) == sim::Card::Grass,\n"
        "         \"The complete K1 route must search Grass.\");",
        "  expect(sim::EngineTestAccess::seed23_fss_route(fss),\n"
        "         \"The exact public K1 route must be admitted.\");\n"
        "  expect(sim::EngineTestAccess::fss_target(fss) == sim::Card::Grass,\n"
        "         \"The complete K1 route must search Grass.\");",
        "positive FSS helper control",
    )
    text = replace_once(
        text,
        "  expect(sim::EngineTestAccess::fss_target(blocked_fss) != sim::Card::Grass,\n"
        "         \"Crispin must retain a second searchable Grass.\");",
        "  expect(!sim::EngineTestAccess::seed23_fss_route(blocked_fss),\n"
        "         \"Crispin must retain a second searchable Grass.\");",
        "one-Grass helper control",
    )
    text = replace_once(
        text,
        "  expect(sim::EngineTestAccess::fss_target(blocked_latias) != sim::Card::Grass,\n"
        "         \"The route must require searchable Latias ex.\");",
        "  expect(!sim::EngineTestAccess::seed23_fss_route(blocked_latias),\n"
        "         \"The route must require searchable Latias ex.\");",
        "no-Latias helper control",
    )
    return replace_once(
        text,
        "  expect(sim::EngineTestAccess::latias_tate_cost(qb) == sim::Card::TateLiza,\n"
        "         \"Latias must replace Tate & Liza's switch role.\");",
        "  expect(sim::EngineTestAccess::seed23_latias_route(qb),\n"
        "         \"The exact public Latias route must be admitted.\");\n"
        "  expect(sim::EngineTestAccess::latias_tate_cost(qb) == sim::Card::TateLiza,\n"
        "         \"Latias must replace Tate & Liza's switch role.\");",
        "positive Quick Ball helper control",
    )


def main() -> int:
    atomic_write(FSS, patch_fss(FSS.read_text(encoding="utf-8")))
    atomic_write(QB, patch_quick_ball(QB.read_text(encoding="utf-8")))
    atomic_write(TEST, patch_test(TEST.read_text(encoding="utf-8")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
