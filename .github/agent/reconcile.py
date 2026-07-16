import os
from pathlib import Path


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_name(path.name + ".tmp-653")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


source = Path("src/trace_engine_v2/part_005.inc")
text = source.read_text(encoding="utf-8")
old_predicate = '''    const bool held_redundant_one_discard_regi_graph =
        !item_locked() && might_be_unseen(Card::RegidragoV) &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::MysteriousTreasure) > 0 &&
        payloads_in_hand > 1 && hand_count(Card::Crispin) == 1 &&
        hand_count(Card::BrilliantBlender) == 0;
'''
new_predicate = '''    const auto payable_one_discard_regi_route = [this](const Card item) {
      // At setup, strict-JIT does not admit a Dragon payload as discard fuel because
      // no same-turn VSTAR-ready payload window exists. Mirror the real Item preflight:
      // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_009a.inc
      // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_009b1.inc
      return hand_count(item) > 0 &&
             choose_discard(false, true, true, item).has_value();
    };
    const bool held_payable_one_discard_regi_graph =
        !item_locked() && might_be_unseen(Card::RegidragoV) &&
        payloads_in_hand > 1 && hand_count(Card::Crispin) == 1 &&
        hand_count(Card::BrilliantBlender) == 0 &&
        (payable_one_discard_regi_route(Card::QuickBall) ||
         payable_one_discard_regi_route(Card::MysteriousTreasure));
'''
if old_predicate not in text:
    if "held_payable_one_discard_regi_graph" not in text:
        raise SystemExit("issue 653 predicate anchor missing")
else:
    text = text.replace(old_predicate, new_predicate, 1)
text = text.replace(
    "         held_redundant_one_discard_regi_graph);",
    "         held_payable_one_discard_regi_graph);",
    1,
)
text = text.replace(
    '''    // hand retains either Tapu recovery or the reviewed redundant Quick Ball plus
    // Mysterious Treasure graph. Exactly one Crispin and no held Blender preserve
    // Wonder Tag whenever a stronger direct payload outlet already solves that axis:
''',
    '''    // hand retains either Tapu recovery or a DCI-payable Quick Ball or Mysterious
    // Treasure route to Regidrago V. Exactly one Crispin and no held Blender preserve
    // Wonder Tag whenever a stronger direct payload outlet already solves that axis.
    // The centralized discard selector proves the printed one-card cost is payable:
''',
    1,
)
text = text.replace(
    '''    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/653
''',
    '''    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/653
''',
    1,
)
atomic_write(source, text)


tests = Path("tests/opening_active_dialga_payload_tests.cpp")
text = tests.read_text(encoding="utf-8")
start = text.find("void test_opening_choice_preserves_dialga_with_direct_regi_route() {")
end = text.find("void test_full_item_lock_rejects_direct_regi_item_signal() {")
if start == -1 or end == -1 or end <= start:
    if "test_quick_ball_only_payable_route_preserves_dialga" not in text:
        raise SystemExit("issue 653 positive-test anchor missing")
else:
    replacement = r'''void test_quick_ball_only_payable_route_preserves_dialga() {
  const sim::Scenario scenario{"opening-dialga-quick-ball-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65301};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::QuickBall,
                sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::Powerglass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A second Quick Ball is a legal low-DCI cost for the first copy. The paid Item
  // therefore exposes Regidrago V while Dialga-GX remains the protected payload:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("A payable Quick Ball route should preserve Dialga-GX.");
  }
}

void test_mysterious_treasure_only_payable_route_preserves_dialga() {
  const sim::Scenario scenario{"opening-dialga-treasure-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65302};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure, sim::Card::MegaDragonite,
                sim::Card::Powerglass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A second Mysterious Treasure legally pays the first copy's discard cost, and
  // Regidrago V is a legal Dragon search target:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("A payable Mysterious Treasure route should preserve Dialga-GX.");
  }
}

void test_unpayable_cross_item_graph_preserves_wonder_tag() {
  const sim::Scenario scenario{"opening-dialga-unpayable-items", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65303};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::QuickBall,
                sim::Card::MysteriousTreasure, sim::Card::MegaDragonite,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Before a same-turn VSTAR payload window exists, strict-JIT protects Mega Dragonite
  // ex and Chaotic Swell. The centralized selector does not admit either singleton
  // search Item as the other Item's cost, so Wonder Tag remains the live connector:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/788
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("An unpayable cross-Item graph should preserve Wonder Tag.");
  }
}

'''
    text = text[:start] + replacement + text[end:]
text = text.replace(
    "  test_opening_choice_preserves_dialga_with_direct_regi_route();\n",
    "  test_quick_ball_only_payable_route_preserves_dialga();\n"
    "  test_mysterious_treasure_only_payable_route_preserves_dialga();\n"
    "  test_unpayable_cross_item_graph_preserves_wonder_tag();\n",
    1,
)
atomic_write(tests, text)

Path(__file__).unlink()
