from pathlib import Path

source = Path("src/trace_engine_v2/part_014a.inc")
text = source.read_text()
old = '''const auto issue_1757_prelock_vessel_available = [this] {
  if (secret_box_combo_enabled() ||
      scenario_.locks != LockMode::TurnTwoItem || scenario_.going_first ||
      state_.turn != 1 || item_locked() || !need_regi() || has_vstar() ||
      hand_count(Card::EarthenVessel) == 0 ||
      hand_count(Card::MegaDragonite) < 2) {
    return false;
  }
'''
new = '''const auto issue_1757_redundant_payload_cost = [this]() -> std::optional<Card> {
  // The scheduled-lock fallback needs one safely redundant physical copy of the
  // same modeled Dragon payload, independent of the historical Mega Dragonite
  // witness. A singleton stays protected for the later strict-JIT payload turn:
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dynamic DCI and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Systemic overfit fix: https://github.com/FlareZ123/pokemon-sims/issues/2901
  for (const Card card : {Card::Appletun, Card::MegaDragonite, Card::Dragapult,
                          Card::GoodraVstar, Card::DialgaGX}) {
    if (hand_count(card) >= 2) return card;
  }
  return std::nullopt;
};

const auto issue_1757_prelock_vessel_available =
    [this, &issue_1757_redundant_payload_cost] {
  if (secret_box_combo_enabled() ||
      scenario_.locks != LockMode::TurnTwoItem || scenario_.going_first ||
      state_.turn != 1 || item_locked() || !need_regi() || has_vstar() ||
      hand_count(Card::EarthenVessel) == 0 ||
      !issue_1757_redundant_payload_cost().has_value()) {
    return false;
  }
'''
if old not in text:
    raise SystemExit("source gate not found")
text = text.replace(old, new, 1)
old_play = '''const auto play_issue_1757_prelock_vessel =
    [this, &issue_1757_prelock_vessel_available] {
  if (!issue_1757_prelock_vessel_available()) return false;
  if (!remove_one(state_.hand, Card::EarthenVessel)) {
    throw std::logic_error("Issue-1757 Earthen Vessel disappeared");
  }
  state_.discard.push_back(Card::EarthenVessel);
  if (!discard_from_hand(Card::MegaDragonite,
                         "Earthen Vessel issue-1757 duplicate payload cost",
                         "R-EV-01; P-DCI-01; P-KNOWLEDGE-01")) {
    throw std::logic_error("Issue-1757 duplicate payload cost disappeared");
  }
'''
new_play = '''const auto play_issue_1757_prelock_vessel =
    [this, &issue_1757_prelock_vessel_available,
     &issue_1757_redundant_payload_cost] {
  if (!issue_1757_prelock_vessel_available()) return false;
  const std::optional<Card> payload_cost = issue_1757_redundant_payload_cost();
  if (!payload_cost) return false;
  if (!remove_one(state_.hand, Card::EarthenVessel)) {
    throw std::logic_error("Issue-1757 Earthen Vessel disappeared");
  }
  state_.discard.push_back(Card::EarthenVessel);
  if (!discard_from_hand(*payload_cost,
                         "Earthen Vessel issue-1757 redundant payload cost",
                         "R-EV-01; P-DCI-01; P-KNOWLEDGE-01")) {
    throw std::logic_error("Issue-1757 redundant payload cost disappeared");
  }
'''
if old_play not in text:
    raise SystemExit("source play block not found")
text = text.replace(old_play, new_play, 1)
text = text.replace("One of two\n  // identical Mega Dragonite ex copies is a realistic DCI cost because the other\n  // copy remains protected", "One of two\n  // identical modeled Dragon payload copies is a realistic DCI cost because the other\n  // copy remains protected", 1)
source.write_text(text)

test = Path("tests/issue_1757_prelock_vessel_tests.cpp")
t = test.read_text()
t = t.replace('''sim::State prelock_state(const int payload_copies = 2,
                         const bool include_energy = true) {''', '''sim::State prelock_state(const int payload_copies = 2,
                         const bool include_energy = true,
                         const sim::Card payload = sim::Card::MegaDragonite) {''', 1)
t = t.replace('''    state.hand.push_back(sim::Card::MegaDragonite);''', '''    state.hand.push_back(payload);''', 1)
anchor = '''void test_singleton_payload_is_protected() {'''
addition = '''void test_equivalent_duplicate_dragapult_uses_same_prelock_route() {
  sim::Scenario scenario{"issue-2901-dragapult", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 5};
  std::mt19937_64 rng{2901};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(
      engine, prelock_state(2, true, sim::Card::Dragapult));

  // Earthen Vessel's discard cost is card-agnostic. Two physical copies of the
  // same modeled Dragon payload let one copy pay the use-or-lose T1 Vessel while
  // the other remains protected for strict-JIT, regardless of payload identity:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Item/discard/search procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Scheduled lock and DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed systemic bug: https://github.com/FlareZ123/pokemon-sims/issues/2901
  expect(sim::EngineTestAccess::run_search_step(engine),
         "The pre-lock Vessel route remained Mega-Dragonite-specific.");
  const auto& state = sim::EngineTestAccess::state(engine);
  expect(std::count(state.hand.begin(), state.hand.end(), sim::Card::Dragapult) == 1 &&
             std::count(state.discard.begin(), state.discard.end(),
                        sim::Card::Dragapult) == 1,
         "The generic route did not spend exactly one redundant Dragapult copy.");
}

'''
if anchor not in t:
    raise SystemExit("test anchor not found")
t = t.replace(anchor, addition + anchor, 1)
t = t.replace('''  test_singleton_payload_is_protected();''', '''  test_equivalent_duplicate_dragapult_uses_same_prelock_route();
  test_singleton_payload_is_protected();''', 1)
test.write_text(t)
