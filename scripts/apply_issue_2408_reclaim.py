from pathlib import Path
import fcntl
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    lock_path = path.with_name(path.name + ".lock")
    with lock_path.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile("w", encoding="utf-8", dir=path.parent, delete=False) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_name = tmp.name
        os.replace(tmp_name, path)
        fcntl.flock(lock.fileno(), fcntl.LOCK_UN)
    lock_path.unlink(missing_ok=True)


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    if text.count(old) != 1:
        raise RuntimeError(f"#2408 {label} anchor mismatch: {text.count(old)} matches")
    return text.replace(old, new, 1)


source_path = ROOT / "src/trace_engine_v2/part_issue_1209_treasure_tapu_crispin_override.inc"
source = source_path.read_text(encoding="utf-8")
source_anchor = "    choose_supporter_issue1209_original();\n"
source_insert = r'''    const bool issue_2408_k1_burnet_before_serena =
        scenario_.dci == DciProfile::StrictJit && prizes_known() &&
        need_payload() && !need_regi() && !need_vstar() && !need_energy() &&
        !need_active_vstar() && can_play_payload_this_turn() &&
        hand_count(Card::ProfessorBurnet) > 0 && hand_count(Card::Serena) > 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        !has_live_blender_payload_line() &&
        !has_live_one_discard_hand_payload_line() &&
        !has_live_ultra_ball_hand_payload_line();
    if (issue_2408_k1_burnet_before_serena) {
      // K1 proves deck contents before this Supporter choice. Burnet can discard a
      // known deck-resident Dragon without consuming Serena or the held Dragon.
      // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // One Supporter per turn and deck-search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // K1, strict-JIT, DCI, and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
      if (play_professor_burnet()) return;
    }

'''
if source_insert not in source:
    if source.count(source_anchor) != 1:
        raise RuntimeError("#2408 Supporter wrapper anchor mismatch")
    source = source.replace(source_anchor, source_insert + source_anchor, 1)
atomic_write(source_path, source)


test_path = ROOT / "tests/issue_1341_burnet_held_payload_tests.cpp"
test = test_path.read_text(encoding="utf-8")
test = replace_once(
    test,
    '''  static void set_state(Engine& engine, State state) {\n    engine.state_ = std::move(state);\n    engine.deck_seen_ = true;\n  }\n''',
    '''  static void set_state(Engine& engine, State state, const bool deck_seen = true) {\n    engine.state_ = std::move(state);\n    engine.deck_seen_ = deck_seen;\n  }\n''',
    "EngineTestAccess",
)
test = replace_once(
    test,
    '''  sim::EngineTestAccess::set_state(engine, std::move(state));\n\n  // Serena can use its mandatory discard on the held Dragon and complete strict JIT.\n  // Burnet must remain out of the discard pile while Serena uses the Supporter slot:\n''',
    '''  sim::EngineTestAccess::set_state(engine, std::move(state), false);\n\n  // K0 has not established deck contents, so retain the conservative held-card line.\n  // Serena can use its mandatory discard on the held Dragon and complete strict JIT.\n  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states\n  // Confirmed K1-only correction: https://github.com/FlareZ123/pokemon-sims/issues/2408\n''',
    "K0 Serena control",
)
new_test = r'''void test_k1_burnet_preserves_serena_and_held_payload() {
  const sim::Scenario scenario{"issue-2408-k1-burnet-resource-order",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{2408};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::Serena,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::RegidragoV,
                sim::Card::Dragapult, sim::Card::DialgaGX,
                sim::Card::Fire, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // K1 proves the deck-resident Dragon payloads. Both Supporters complete the sole
  // strict-JIT payload axis, so Burnet preserves Serena and the held Dragon.
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter/search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.supporter_used, "Professor Burnet must use the Supporter play.");
  expect(contains(result.discard, sim::Card::ProfessorBurnet),
         "K1 payload-only state must prefer Professor Burnet.");
  expect(contains(result.hand, sim::Card::Serena) &&
             !contains(result.discard, sim::Card::Serena),
         "Serena must remain held after the K1 Burnet route.");
  expect(contains(result.hand, sim::Card::MegaDragonite) &&
             !contains(result.discard, sim::Card::MegaDragonite),
         "The held Dragon must remain held after the K1 Burnet route.");
  expect(contains(result.discard, sim::Card::Dragapult) &&
             contains(result.discard, sim::Card::DialgaGX),
         "Burnet must discard the known deck payloads.");
}

'''
if new_test not in test:
    test = replace_once(test, "void test_no_held_payload_preserves_burnet_route() {\n",
                        new_test + "void test_no_held_payload_preserves_burnet_route() {\n",
                        "K1 regression insertion")
test = replace_once(
    test,
    '''    test_serena_uses_held_payload_instead_of_burnet();\n    test_no_held_payload_preserves_burnet_route();\n''',
    '''    test_serena_uses_held_payload_instead_of_burnet();\n    test_k1_burnet_preserves_serena_and_held_payload();\n    test_no_held_payload_preserves_burnet_route();\n''',
    "K1 regression invocation",
)
atomic_write(test_path, test)


prize_path = ROOT / "tests/issue_2091_prize_k1_issue_1341_burnet_held_payload_tests.cpp"
prize = prize_path.read_text(encoding="utf-8")
prize = replace_once(prize,
                     "void test_serena_uses_held_payload_instead_of_burnet() {\n",
                     "void test_k1_burnet_preserves_serena_and_held_payload() {\n",
                     "Prize-K1 test name")
prize = replace_once(
    prize,
    '''  // Serena can use its mandatory discard on the held Dragon and complete strict JIT.\n  // Burnet must remain out of the discard pile while Serena uses the Supporter slot:\n  // https://api.pokemontcg.io/v2/cards/swsh12-164\n  // https://api.pokemontcg.io/v2/cards/me2pt5-152\n  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26\n  // https://api.pokemontcg.io/v2/cards/swsh12-136\n  // https://github.com/FlareZ123/pokemon-sims/issues/888\n  // https://github.com/FlareZ123/pokemon-sims/issues/1341\n''',
    '''  // Complete Prize inspection is K1. Burnet can use known deck payloads while\n  // preserving Serena and the held Dragon for later discrete-value routes.\n  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26\n  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164\n  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152\n  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136\n  // Official Supporter/search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf\n  // K1/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n  // Prize-derived K1 regression: https://github.com/FlareZ123/pokemon-sims/issues/2091\n  // Confirmed ordering correction: https://github.com/FlareZ123/pokemon-sims/issues/2408\n''',
    "Prize-K1 comments",
)
prize = replace_once(
    prize,
    '''  expect(result.supporter_used, "Serena must use the Supporter play.");\n  expect(contains(result.discard, sim::Card::Serena),\n         "Serena must be the Supporter placed in discard.");\n  expect(contains(result.discard, sim::Card::MegaDragonite),\n         "Serena must discard the held Dragon payload.");\n  expect(!contains(result.discard, sim::Card::ProfessorBurnet),\n         "Professor Burnet must not be consumed before Serena.");\n''',
    '''  expect(result.supporter_used, "Professor Burnet must use the Supporter play.");\n  expect(contains(result.discard, sim::Card::ProfessorBurnet),\n         "Prize-derived K1 must prefer Professor Burnet.");\n  expect(contains(result.hand, sim::Card::Serena) &&\n             !contains(result.discard, sim::Card::Serena),\n         "Serena must remain held after the K1 Burnet route.");\n  expect(contains(result.hand, sim::Card::MegaDragonite) &&\n             !contains(result.discard, sim::Card::MegaDragonite),\n         "The held Dragon must remain held after the K1 Burnet route.");\n  expect(contains(result.discard, sim::Card::Dragapult) &&\n             contains(result.discard, sim::Card::DialgaGX),\n         "Burnet must discard the known deck payloads.");\n''',
    "Prize-K1 expectations",
)
prize = replace_once(prize,
                     "    test_serena_uses_held_payload_instead_of_burnet();\n",
                     "    test_k1_burnet_preserves_serena_and_held_payload();\n",
                     "Prize-K1 invocation")
atomic_write(prize_path, prize)
