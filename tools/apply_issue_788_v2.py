from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-788-v2.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-788-v2.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 788 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))

        source = Path("src/trace_engine_v2/part_005.inc")
        old_route = '''    const bool held_redundant_one_discard_regi_graph =
        !item_locked() && might_be_unseen(Card::RegidragoV) &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::MysteriousTreasure) > 0 &&
        payloads_in_hand > 1 && hand_count(Card::Crispin) == 1 &&
        hand_count(Card::BrilliantBlender) == 0;
'''
        new_route = '''    const bool payable_quick_ball_regi_route =
        hand_count(Card::QuickBall) > 0 &&
        choose_discard(false, true, true, Card::QuickBall).has_value();
    const bool payable_mysterious_treasure_regi_route =
        hand_count(Card::MysteriousTreasure) > 0 &&
        choose_discard(false, true, true, Card::MysteriousTreasure).has_value();
    const bool held_redundant_one_discard_regi_graph =
        !item_locked() && might_be_unseen(Card::RegidragoV) &&
        (payable_quick_ball_regi_route || payable_mysterious_treasure_regi_route) &&
        payloads_in_hand > 1 && hand_count(Card::Crispin) == 1 &&
        hand_count(Card::BrilliantBlender) == 0;
'''
        replace_once(source, old_route, new_route)

        old_comment = '''    // Dialga-GX is a modeled Apex Dragon payload. Under strict current-turn DCI,
    // preserve it when Crispin already covers the Supporter connector and the public
    // hand retains either Tapu recovery or the reviewed redundant Quick Ball plus
    // Mysterious Treasure graph. Exactly one Crispin and no held Blender preserve
    // Wonder Tag whenever a stronger direct payload outlet already solves that axis:
'''
        new_comment = '''    // Quick Ball and Mysterious Treasure expose Regidrago V only when the
    // centralized DCI selector admits their mandatory printed discard. Protected
    // singleton and payload hands must not advertise an impossible connector:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/issues/788
    // Dialga-GX is a modeled Apex Dragon payload. Under strict current-turn DCI,
    // preserve it when Crispin already covers the Supporter connector and the public
    // hand retains either Tapu recovery or a payable one-discard Regidrago V route.
    // Exactly one Crispin and no held Blender preserve Wonder Tag whenever a stronger
    // direct payload outlet already solves that axis:
'''
        replace_once(source, old_comment, new_comment)

        tests = Path("tests/opening_active_dialga_payload_tests.cpp")
        old_test = '''void test_opening_choice_preserves_dialga_with_direct_regi_route() {
  const sim::Scenario scenario{"opening-dialga-direct-regi-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65301};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::MysteriousTreasure,
                sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Setup may choose either Basic. The held one-discard Items already expose
  // Regidrago V, Crispin covers the Supporter route, and Mega Dragonite ex provides
  // a second Dragon payload, so Tapu Lele-GX has lower marginal connector value:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX) ||
      !contains(after.hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Tapu Lele-GX should start when a held one-discard Item exposes Regidrago V.");
  }
}
'''
        new_test = '''void test_unpayable_singleton_search_items_preserve_wonder_tag() {
  const sim::Scenario scenario{"opening-dialga-unpayable-search", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{78801};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::MysteriousTreasure,
                sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Both Items require a different card as their discard cost. Every other card in
  // this strict-DCI hand is protected, so neither Item exposes Regidrago V and
  // Tapu Lele-GX must remain available for a later Wonder Tag:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/788
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Unpayable singleton search Items must preserve Wonder Tag.");
  }
}

void test_duplicate_quick_ball_is_a_payable_regi_route() {
  const sim::Scenario scenario{"opening-dialga-duplicate-quick-ball", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{78802};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::QuickBall,
                sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A second Quick Ball can pay the first copy's printed discard cost:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/issues/788
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("Duplicate Quick Ball should preserve Dialga as a payload.");
  }
}

void test_duplicate_mysterious_treasure_is_a_payable_regi_route() {
  const sim::Scenario scenario{"opening-dialga-duplicate-treasure", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{78803};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure, sim::Card::MegaDragonite,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A second Mysterious Treasure can pay the first copy's printed discard cost:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/788
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("Duplicate Mysterious Treasure should preserve Dialga as a payload.");
  }
}
'''
        replace_once(tests, old_test, new_test)

        replace_once(
            tests,
            "  test_opening_choice_preserves_dialga_with_direct_regi_route();\n",
            "  test_unpayable_singleton_search_items_preserve_wonder_tag();\n  test_duplicate_quick_ball_is_a_payable_regi_route();\n  test_duplicate_mysterious_treasure_is_a_payable_regi_route();\n",
        )
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
