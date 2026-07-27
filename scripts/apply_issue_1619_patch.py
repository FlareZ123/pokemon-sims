from pathlib import Path
from textwrap import dedent

source_path = Path("src/trace_engine_v2/part_issue_1118_secret_box.inc")
source = source_path.read_text(encoding="utf-8")

visibility_anchor = dedent(
    """\
        const bool treasure_available = hand_count(Card::MysteriousTreasure) > 0 ||
            might_be_unseen(Card::MysteriousTreasure);
        const bool needs_treasure = !vstar_held_or_play || !payload_already;
        if (!vstar_available || !payload_available ||
            (needs_treasure && !treasure_available)) {
    """
)
visibility_insert = dedent(
    """\
        const bool treasure_available = hand_count(Card::MysteriousTreasure) > 0 ||
            might_be_unseen(Card::MysteriousTreasure);
        const bool dawn_payload_line = dawn_line && !payload_already &&
            payload_might_be_in_deck();
        // Dawn can search Forretress ex as its Stage 1 and Dragapult ex as its Stage 2
        // in the same resolution. When VSTAR is already held, that public route makes
        // Mysterious Treasure unnecessary and leaves Secret Box's Item search free for
        // Earthen Vessel, which discards the Dawn-searched payload and finds Fire:
        // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
        // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
        // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Core search and discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1619
        const bool needs_treasure =
            !vstar_held_or_play || (!payload_already && !dawn_payload_line);
        if (!vstar_available || !payload_available ||
            (needs_treasure && !treasure_available)) {
    """
)
if source.count(visibility_anchor) != 1:
    raise SystemExit(
        f"issue-1619 visibility anchor count: {source.count(visibility_anchor)}"
    )
source = source.replace(visibility_anchor, visibility_insert, 1)

selection_anchor = dedent(
    """\
        const bool vstar_held_or_play = has_vstar() ||
            hand_count(Card::RegidragoVstar) > 0;
        const bool needs_treasure = !vstar_held_or_play || !payload_ready();
        const bool needs_fire = fire_needed() > 0 && hand_count(Card::Fire) == 0;
    """
)
selection_insert = dedent(
    """\
        const bool vstar_held_or_play = has_vstar() ||
            hand_count(Card::RegidragoVstar) > 0;
        const bool dawn_will_supply_payload = needs_dawn && supporter_allowed() &&
            (hand_count(Card::Dawn) > 0 || might_be_unseen(Card::Dawn)) &&
            payload_might_be_in_deck();
        // Preserve Secret Box's Item channel for Earthen Vessel when held VSTAR and
        // Dawn's independent Stage 1 plus Stage 2 searches already cover evolution and
        // payload. This is the direct same-turn route, so Treasure would add a redundant
        // connector and strand the Fire axis:
        // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
        // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
        // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1619
        const bool needs_treasure = !vstar_held_or_play ||
            (!payload_ready() && !dawn_will_supply_payload);
        const bool needs_fire = fire_needed() > 0 && hand_count(Card::Fire) == 0;
    """
)
if source.count(selection_anchor) != 1:
    raise SystemExit(
        f"issue-1619 selection anchor count: {source.count(selection_anchor)}"
    )
source = source.replace(selection_anchor, selection_insert, 1)

cost_anchor = "      if (card == Card::QuickBall && has_any_regi()) return 7;\n"
cost_insert = dedent(
    """\
          if (card == Card::QuickBall && has_any_regi()) return 7;
          const bool established_prior_turn_regi =
              (state_.active && state_.active->card == Card::RegidragoV &&
               state_.active->entered_turn < state_.turn) ||
              std::any_of(state_.bench.begin(), state_.bench.end(),
                          [this](const Pokemon& pokemon) {
                return pokemon.card == Card::RegidragoV &&
                       pokemon.entered_turn < state_.turn;
              });
          // A held Regidrago V becomes a route-replaced Secret Box cost only after a
          // separate prior-turn Regidrago V is already established and every same-turn
          // Pineco, Energy, evolution, payload, and search axis is publicly executable.
          // The live attacker remains in play, so discarding the hand copy cannot remove
          // the evolution target or delay the proven completion window:
          // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
          // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
          // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
          // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
          // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
          // Core discard, search, Bench, evolution, Ability, attachment, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // Dynamic DCI and route replacement: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#secret-box-and-pineco-route-policy
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1619
          if (card == Card::RegidragoV && established_prior_turn_regi) return 7;
    """
)
if source.count(cost_anchor) != 1:
    raise SystemExit(f"issue-1619 cost anchor count: {source.count(cost_anchor)}")
source_path.write_text(source.replace(cost_anchor, cost_insert, 1), encoding="utf-8")

test_path = Path("tests/issue_1118_multi_deck_secret_box_tests.cpp")
test_source = test_path.read_text(encoding="utf-8")
test_anchor = "void expect_seeded_route(const std::uint64_t seed,\n"
test_insert = dedent(
    r"""\
    void test_issue_1619_route_replaced_extra_regidrago_cost() {
      Fixture payable;
      sim::State state;
      state.turn = 2;
      state.active = sim::Pokemon{sim::Card::Pineco, 0, 0, 0,
                                  sim::Tool::None};
      state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                                         sim::Tool::ForestSealStone});
      state.hand = {sim::Card::SecretBox, sim::Card::RegidragoVstar,
                    sim::Card::RegidragoVstar,
                    sim::Card::ForestOfVitality, sim::Card::RegidragoV};
      state.deck = {sim::Card::MysteriousTreasure, sim::Card::WishfulBaton,
                    sim::Card::Dawn, sim::Card::ForretressEx,
                    sim::Card::Dragapult, sim::Card::EarthenVessel,
                    sim::Card::Fire, sim::Card::Grass, sim::Card::Grass};
      sim::EngineTestAccess::set_state(payable.engine, state);

      // The established prior-turn attacker reserves the live Regidrago V line. One
      // duplicate VSTAR remains after cost payment, while Dawn can search Forretress ex
      // and Dragapult ex together, leaving the Item channel for Earthen Vessel:
      // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
      // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
      // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Core rules: https://www.pokemon.com/us/pokemon-tcg/rules
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1619
      if (!sim::EngineTestAccess::play_secret_box(payable.engine)) {
        throw std::runtime_error(
            "The issue-1619 complete route did not admit three costs.");
      }
      const sim::State& after = sim::EngineTestAccess::state(payable.engine);
      if (std::count(after.discard.begin(), after.discard.end(),
                     sim::Card::RegidragoVstar) != 1 ||
          !contains(after.discard, sim::Card::ForestOfVitality) ||
          !contains(after.discard, sim::Card::RegidragoV) ||
          std::count(after.hand.begin(), after.hand.end(),
                     sim::Card::RegidragoVstar) != 1 ||
          !contains(after.hand, sim::Card::EarthenVessel) ||
          contains(after.hand, sim::Card::MysteriousTreasure)) {
        throw std::runtime_error(
            "The issue-1619 route did not preserve one VSTAR and select the direct Vessel continuation.");
      }

      Fixture protected_singleton;
      state.hand = {sim::Card::SecretBox, sim::Card::RegidragoVstar,
                    sim::Card::ForestOfVitality, sim::Card::RegidragoV};
      sim::EngineTestAccess::set_state(protected_singleton.engine, state);

      // One VSTAR remains UDP because no replacement exists for the only evolution
      // card. The extra Regidrago V exception therefore supplies only a second cost,
      // and Secret Box must remain blocked rather than consume the singleton VSTAR:
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
      // UDP/DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1619
      if (sim::EngineTestAccess::play_secret_box(protected_singleton.engine) ||
          sim::EngineTestAccess::outcome(protected_singleton.engine)
                  .secret_box_cost_blocked != 1U) {
        throw std::runtime_error(
            "The issue-1619 exception spent a sole protected VSTAR.");
      }
    }

    void expect_seeded_route(const std::uint64_t seed,
    """
)
if test_source.count(test_anchor) != 1:
    raise SystemExit(f"issue-1619 test anchor count: {test_source.count(test_anchor)}")
test_source = test_source.replace(test_anchor, test_insert, 1)

main_anchor = "  test_secret_box_cost_reservation_and_dci();\n"
main_insert = (
    "  test_secret_box_cost_reservation_and_dci();\n"
    "  test_issue_1619_route_replaced_extra_regidrago_cost();\n"
)
if test_source.count(main_anchor) != 1:
    raise SystemExit(f"issue-1619 main anchor count: {test_source.count(main_anchor)}")
test_path.write_text(test_source.replace(main_anchor, main_insert, 1), encoding="utf-8")
