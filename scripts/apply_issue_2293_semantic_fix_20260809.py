from pathlib import Path
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    fd, temp_name = tempfile.mkstemp(prefix=path.name + ".", dir=path.parent, text=True)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp_name, path)
    except BaseException:
        try:
            os.unlink(temp_name)
        except FileNotFoundError:
            pass
        raise


part7 = ROOT / "src/trace_engine_v2/part_007.inc"
text = part7.read_text(encoding="utf-8")
selector_anchor = "  Card choose_supporter_after_search_started() const {\n"
helper = r'''  bool k1_prized_latias_gladion_promotion_route() const {
    if (!strict_payload_timing() || !prizes_known() || !supporter_allowed() ||
        item_locked() || state_.retreat_used || !state_.active ||
        !is_basic(state_.active->card) || state_.active->card == Card::RegidragoV ||
        !need_active_vstar() || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        prize_count_after_reveal(Card::LatiasEx) == 0 ||
        hand_count(Card::HisuianHeavyBall) > 0 ||
        deck_count_after_search_started(Card::Gladion) == 0) {
      return false;
    }

    // Project the selected Benched attacker through only actions that remain legal
    // after Wonder Tag spends the Supporter axis. Existing VSTARs are accepted, or
    // a prior-turn Regidrago V may use a directly held VSTAR card. One unused
    // manual attachment may finish the exact Apex Dragon Energy cost, including a
    // legal Double Dragon Energy configuration through the shared Energy helpers.
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Evolution and one-manual-Energy-per-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    bool attacker_axis_finishes = false;
    for (const Pokemon& pokemon : state_.bench) {
      Pokemon projected = pokemon;
      if (projected.card == Card::RegidragoV) {
        if (projected.entered_turn >= state_.turn ||
            hand_count(Card::RegidragoVstar) == 0) {
          continue;
        }
        projected.card = Card::RegidragoVstar;
      } else if (projected.card != Card::RegidragoVstar) {
        continue;
      }
      if (!pays_apex_energy_cost(projected) && !state_.manual_energy_used) {
        const auto manual = preferred_manual_energy_from_hand(projected);
        if (manual) attach_energy_card(projected, *manual);
      }
      if (pays_apex_energy_cost(projected)) {
        attacker_axis_finishes = true;
        break;
      }
    }
    if (!attacker_axis_finishes) return false;

    const bool current_turn_payload = std::any_of(
        state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), is_payload);
    const bool blender_payload = hand_count(Card::BrilliantBlender) > 0 &&
        payload_might_be_in_deck() && can_play_payload_this_turn();

    const bool treasure_has_target = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic);
    const bool quick_ball_has_target = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_basic);
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const bool direct_item_payload = held_payload &&
        ((hand_count(Card::MysteriousTreasure) > 0 && treasure_has_target) ||
         (hand_count(Card::QuickBall) > 0 && quick_ball_has_target));

    // Quick Ball can create a payload without a Supporter only when K1 proves a
    // Basic Dragon payload in deck and a distinct Mysterious Treasure survives the
    // first mandatory discard. Simulate the first cost, remove the planned Dialga
    // from the known deck logically, and require a legal Psychic/Dragon target for
    // the surviving Treasure. This is a paid two-connector hypergraph route rather
    // than a seed or hand-identity special case.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dialga-GX is the registered Basic Dragon payload: https://api.pokemontcg.io/v2/cards/sm5-100
    // Trainer cost, search, and shuffle procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    bool quick_ball_treasure_payload = false;
    if (hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        deck_count_after_search_started(Card::DialgaGX) > 0) {
      const auto first_cost = choose_discard(false, true, true, Card::QuickBall);
      if (first_cost) {
        const int surviving_treasure = hand_count(Card::MysteriousTreasure) -
            (*first_cost == Card::MysteriousTreasure ? 1 : 0);
        bool treasure_target_after_dialga = false;
        bool removed_dialga = false;
        for (const Card card : state_.deck) {
          if (!removed_dialga && card == Card::DialgaGX) {
            removed_dialga = true;
            continue;
          }
          if (is_dragon_or_psychic(card)) {
            treasure_target_after_dialga = true;
            break;
          }
        }
        quick_ball_treasure_payload =
            surviving_treasure > 0 && treasure_target_after_dialga;
      }
    }

    // Wonder Tag spends the only Supporter play. Gladion is therefore selected only
    // when every other required setup axis has an observable Supporter-independent
    // completion and Latias ex is the known prized mobility connector. A held Heavy
    // Ball remains the cheaper Prize route and suppresses this Supporter exchange.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Core Prize, Supporter, Item, Bench, Ability and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, Supporter contention, strict-JIT, DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2293
    return current_turn_payload || blender_payload || direct_item_payload ||
        quick_ball_treasure_payload;
  }

'''
if helper not in text:
    if text.count(selector_anchor) != 1:
        raise RuntimeError("#2293 supporter selector anchor mismatch")
    text = text.replace(selector_anchor, helper + selector_anchor, 1)

priority_anchor = """    if (crispin_finishes_current_turn_prized_energy_route) return Card::Crispin;\n\n    // Wonder Tag is resolving after legal deck inspection. A known prized\n"""
priority_insert = """    if (crispin_finishes_current_turn_prized_energy_route) return Card::Crispin;\n\n    if (k1_prized_latias_gladion_promotion_route()) {\n      // Gladion recovers the K1-known prized Latias ex that uniquely completes\n      // the unresolved Active-position axis: https://api.pokemontcg.io/v2/cards/sm4-95\n      // https://api.pokemontcg.io/v2/cards/sv8-76\n      // https://github.com/FlareZ123/pokemon-sims/issues/2293\n      return Card::Gladion;\n    }\n\n    // Wonder Tag is resolving after legal deck inspection. A known prized\n"""
if priority_insert not in text:
    if text.count(priority_anchor) != 1:
        raise RuntimeError("#2293 Wonder Tag priority anchor mismatch")
    text = text.replace(priority_anchor, priority_insert, 1)
atomic_write(part7, text)

latias = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"
ltext = latias.read_text(encoding="utf-8")
run_anchor = "void run_turn() {\n"
late_helper = r'''  bool complete_late_latias_mobility_axis() {
    if (state_.turn_ended || hand_count(Card::LatiasEx) == 0 ||
        in_play(Card::LatiasEx) || bench_space() == 0 || state_.retreat_used ||
        !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV ||
        !ability_available_for_pokemon(Card::LatiasEx)) {
      return false;
    }

    Pokemon* target = best_benched_vstar_for_promotion();
    const bool current_turn_payload = std::any_of(
        state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), is_payload);
    if (target == nullptr || !pays_apex_energy_cost(*target) ||
        (strict_payload_timing() ? !current_turn_payload : !payload_ready())) {
      return false;
    }

    // A Supporter such as Gladion can add Latias ex to hand after the ordinary
    // Basic-play fixed point. Re-run only the now-live Bench transition, then use
    // Skyliner's zero-Retreat effect to promote the already Energy/payload-complete
    // Benched Regidrago VSTAR. Every gate is physical state, so this also repairs
    // equivalent late-Latias transitions without encoding a witness seed.
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Prize, Bench, Ability and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest complete route and K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed transition bug: https://github.com/FlareZ123/pokemon-sims/issues/2293
    if (!bench_from_hand(Card::LatiasEx, false)) {
      throw std::logic_error("late Latias mobility Bench play disappeared");
    }
    if (!retreat_to_benched_vstar_with_latias()) {
      throw std::logic_error("late Skyliner promotion disappeared");
    }
    return true;
  }

'''
if late_helper not in ltext:
    if ltext.count(run_anchor) != 1:
        raise RuntimeError("#2293 run_turn anchor mismatch")
    ltext = ltext.replace(run_anchor, late_helper + run_anchor, 1)

late_anchor = '''    if (late_supporter_was_unused &&
        late_supporter_quick_ball_latias_completion_available()) {
      play_items_until_stable(true);
      play_basics_from_hand();
    }
    if (use_legacy_star()) {
'''
late_insert = '''    if (late_supporter_was_unused &&
        late_supporter_quick_ball_latias_completion_available()) {
      play_items_until_stable(true);
      play_basics_from_hand();
    }
    if (complete_late_latias_mobility_axis()) {
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    if (use_legacy_star()) {
'''
if late_insert not in ltext:
    if ltext.count(late_anchor) != 1:
        raise RuntimeError("#2293 late Supporter replay anchor mismatch")
    ltext = ltext.replace(late_anchor, late_insert, 1)
atomic_write(latias, ltext)

cmake = ROOT / "CMakeLists.txt"
ctext = cmake.read_text(encoding="utf-8")
if "trace_issue_2293_strict_first_seed291" not in ctext:
    ctext += r'''

# K1 Wonder Tag must recover the known prized Latias ex through Gladion when the
# remaining Energy/evolution and current-turn payload axes are independently live.
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
# Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
# Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
# Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2293
add_test(NAME trace_issue_2293_strict_first_seed291
  COMMAND regidrago_sim --simulate-this --deck regidrago-shell
    --scenario strict-jit/go-first --seed 291 --require-ready-by 4)
'''
atomic_write(cmake, ctext)
