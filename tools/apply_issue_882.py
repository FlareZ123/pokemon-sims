from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-882.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-882.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text and old not in text:
        return
    if text.count(old) != 1:
        raise SystemExit(f"issue 882 anchor missing or duplicated in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))

        replace_once(
            Path("src/trace_engine_v2/part_012.inc"),
            """  bool play_serena() {
    if (!supporter_allowed() || hand_count(Card::Serena) == 0) return false;
    const bool permit_payload = can_play_payload_this_turn() && std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const auto first = choose_discard(permit_payload, true);
    if (!first) return false;
    remove_one(state_.hand, Card::Serena);
    state_.discard.push_back(Card::Serena);
    discard_from_hand(*first, "Serena chosen discard", "R-SERENA-01");
    if (scenario_.dci != DciProfile::StrictJit) {
      for (int number = 0; number < 2; ++number) {
        const auto extra = choose_discard(false, true);
        if (!extra || is_payload(*extra)) break;
        const bool can_cycle = *extra == Card::Dipplin || *extra == Card::MawileGX || *extra == Card::Guzma ||
                               *extra == Card::Channeler || *extra == Card::FieldBlower || *extra == Card::ChaoticSwell || *extra == Card::PathToPeak;
        if (!can_cycle) break;
""",
            """  bool play_serena(const bool allow_zero_draw_payload_completion = false) {
    if (!supporter_allowed() || hand_count(Card::Serena) == 0) return false;
    const bool permit_payload = can_play_payload_this_turn() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const auto payload_it = std::find_if(state_.hand.begin(), state_.hand.end(), is_payload);
    // The dedicated completion branch exists to put a Dragon payload in discard now.
    // Force that printed Serena discard instead of allowing generic low-DCI fodder to
    // consume the mandatory slot:
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://api.pokemontcg.io/v2/cards/sv6-127
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/882
    const std::optional<Card> first =
        allow_zero_draw_payload_completion && payload_it != state_.hand.end()
            ? std::optional<Card>{*payload_it}
            : choose_discard(permit_payload, true);
    if (!first || *first == Card::Serena) return false;

    // Build the same deterministic discard plan without committing it. Serena draws
    // until five only after Serena itself and at least one chosen card leave the hand:
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/882
    const std::vector<Card> original_hand = state_.hand;
    std::vector<Card> planned_discards{*first};
    remove_one(state_.hand, Card::Serena);
    remove_one(state_.hand, *first);
    if (scenario_.dci != DciProfile::StrictJit) {
      for (int number = 0; number < 2; ++number) {
        const auto extra = choose_discard(false, true);
        if (!extra || is_payload(*extra)) break;
        const bool can_cycle = *extra == Card::Dipplin || *extra == Card::MawileGX || *extra == Card::Guzma ||
                               *extra == Card::Channeler || *extra == Card::FieldBlower || *extra == Card::ChaoticSwell || *extra == Card::PathToPeak;
        if (!can_cycle || !remove_one(state_.hand, *extra)) break;
        planned_discards.push_back(*extra);
""",
        )

        replace_once(
            Path("src/trace_engine_v2/part_013.inc"),
            """        discard_from_hand(*extra, "Serena optional discard", "R-SERENA-01");
      }
    }
    // Serena permits up to three discards in every DCI profile. In strict JIT,
    // choose_discard(false, true) offers only already-safe non-payload fodder.
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh12-164
    if (scenario_.dci == DciProfile::StrictJit) {
      for (int number = 0; number < 2; ++number) {
        const auto extra = choose_discard(false, true);
        if (!extra || is_payload(*extra)) break;
        discard_from_hand(*extra, "Serena optional discard", "R-SERENA-01");
      }
    }
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) draw_card("Serena");
    trace("PLAY SUPPORTER", "R-SERENA-01; R-GAME-SUPPORTER", "Used Serena draw mode after discarding at least one card.");
    return true;
  }
""",
            """      }
    }
    // Serena permits up to three discards in every DCI profile. In strict JIT,
    // choose_discard(false, true) offers only already-safe non-payload fodder.
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh12-164
    if (scenario_.dci == DciProfile::StrictJit) {
      for (int number = 0; number < 2; ++number) {
        const auto extra = choose_discard(false, true);
        if (!extra || is_payload(*extra) || !remove_one(state_.hand, *extra)) break;
        planned_discards.push_back(*extra);
      }
    }

    const bool would_draw = state_.hand.size() < 5U && !state_.deck.empty();
    const bool completes_payload_now = allow_zero_draw_payload_completion &&
        is_payload(*first) && can_play_payload_this_turn();
    state_.hand = original_hand;
    if (!would_draw && !completes_payload_now) {
      // A legal zero-draw resolution is strategically dominated when its discard
      // does not complete the current-turn strict-JIT payload axis:
      // https://api.pokemontcg.io/v2/cards/swsh12-164
      // https://api.pokemontcg.io/v2/cards/sv6-127
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#discard-capability-index-dci
      // https://github.com/FlareZ123/pokemon-sims/issues/882
      return false;
    }

    remove_one(state_.hand, Card::Serena);
    state_.discard.push_back(Card::Serena);
    for (std::size_t index = 0; index < planned_discards.size(); ++index) {
      discard_from_hand(planned_discards[index],
                        index == 0U ? "Serena chosen discard" : "Serena optional discard",
                        "R-SERENA-01");
    }
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) draw_card("Serena");
    trace("PLAY SUPPORTER", "R-SERENA-01; R-GAME-SUPPORTER", "Used Serena draw mode after discarding at least one card.");
    return true;
  }
""",
        )

        replace_once(
            Path("src/trace_engine_v2/part_014b.inc"),
            "    if (serena_completes_only_missing_axis && play_serena()) return;\n",
            """    // The dedicated route may accept zero draw because the mandatory Dragon
    // discard itself completes the final current-turn strict-JIT axis:
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://github.com/FlareZ123/pokemon-sims/issues/882
    if (serena_completes_only_missing_axis && play_serena(true)) return;
""",
        )

        replace_once(
            Path("CMakeLists.txt"),
            """add_executable(regidrago_serena_mandatory_discard_tests tests/serena_mandatory_discard_tests.cpp)
target_compile_options(regidrago_serena_mandatory_discard_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
""",
            """add_executable(regidrago_serena_mandatory_discard_tests tests/serena_mandatory_discard_tests.cpp)
target_compile_options(regidrago_serena_mandatory_discard_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_serena_zero_draw_tests tests/serena_zero_draw_tests.cpp)
target_compile_options(regidrago_serena_zero_draw_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
""",
        )

        replace_once(
            Path("CMakeLists.txt"),
            "add_test(NAME regidrago_serena_mandatory_discard COMMAND regidrago_serena_mandatory_discard_tests)\n",
            """add_test(NAME regidrago_serena_mandatory_discard COMMAND regidrago_serena_mandatory_discard_tests)
add_test(NAME regidrago_serena_zero_draw COMMAND regidrago_serena_zero_draw_tests)
""",
        )
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
