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
anchor = "  Card choose_supporter_after_search_started() const {\n"
helper = r'''  bool k1_prized_latias_gladion_promotion_route() const {
    if (!strict_payload_timing() || !prizes_known() || !supporter_allowed() ||
        item_locked() || state_.manual_energy_used || state_.retreat_used ||
        !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV || bench_space() == 0 ||
        !need_active_vstar() || !need_energy() || !need_payload() ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        prize_count_after_reveal(Card::LatiasEx) == 0 ||
        prize_count_after_reveal(Card::HisuianHeavyBall) == 0 ||
        deck_count_after_search_started(Card::Gladion) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 || hand_count(Card::Fire) == 0 ||
        hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::MysteriousTreasure) < 2 ||
        deck_count_after_search_started(Card::DialgaGX) == 0) {
      return false;
    }

    const bool prior_turn_gg_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.grass >= 2 &&
              pokemon.fire == 0;
        });
    if (!prior_turn_gg_regi) return false;

    // K1 proves Latias ex and Heavy Ball are in Prizes. Gladion therefore restores
    // the missing mobility connector while held Fire plus the unused manual
    // attachment completes GGF and the two one-discard Items preserve an independent
    // Quick Ball -> Dialga-GX -> Mysterious Treasure strict-JIT payload line.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Prize, Supporter, Item, search, attachment, evolution, Bench, Ability and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, Supporter contention, strict-JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2293
    return true;
  }

'''
if helper not in text:
    if text.count(anchor) != 1:
        raise RuntimeError("supporter selector anchor mismatch")
    text = text.replace(anchor, helper + anchor, 1)
old = """  Card choose_supporter_after_search_started() const {\n    if (issue_1797_wonder_tag_steven_route_available()) {\n"""
new = """  Card choose_supporter_after_search_started() const {\n    if (k1_prized_latias_gladion_promotion_route()) {\n      // Gladion recovers the K1-known prized Latias ex that uniquely solves the\n      // unresolved Active-position axis: https://api.pokemontcg.io/v2/cards/sm4-95\n      // https://api.pokemontcg.io/v2/cards/sv8-76\n      // https://github.com/FlareZ123/pokemon-sims/issues/2293\n      return Card::Gladion;\n    }\n    if (issue_1797_wonder_tag_steven_route_available()) {\n"""
if new not in text:
    if text.count(old) != 1:
        raise RuntimeError("supporter selector insertion anchor mismatch")
    text = text.replace(old, new, 1)
atomic_write(part7, text)

latias = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"
ltext = latias.read_text(encoding="utf-8")
run_anchor = "void run_turn() {\n"
late_helper = r'''  bool complete_late_latias_mobility_axis() {
    if (state_.turn_ended || !state_.supporter_used || !prizes_known() ||
        hand_count(Card::LatiasEx) == 0 || in_play(Card::LatiasEx) ||
        bench_space() == 0 || state_.retreat_used || !state_.active ||
        !is_basic(state_.active->card) || state_.active->card == Card::RegidragoV ||
        !need_active_vstar() || need_energy() || need_payload() ||
        !ability_available_for_pokemon(Card::LatiasEx)) {
      return false;
    }

    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || !pays_apex_energy_cost(*target)) return false;

    // A Supporter can add Latias ex to hand after the ordinary Basic-play fixed
    // point. Replaying the legal Bench action is required before Skyliner can remove
    // the Basic Active's Retreat Cost and promote the already-complete VSTAR.
    // Gladion Prize exchange: https://api.pokemontcg.io/v2/cards/sm4-95
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Prize, Bench, Ability and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest complete route and K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed transition bug: https://github.com/FlareZ123/pokemon-sims/issues/2293
    if (!bench_from_hand(Card::LatiasEx, false)) {
      throw std::logic_error("late Latias mobility Bench play disappeared");
    }
    if (!retreat_to_benched_vstar_with_latias()) {
      throw std::logic_error("late Skyliner promotion disappeared");
    }
    return active_is_vstar() && !need_energy() && !need_payload();
  }

'''
if late_helper not in ltext:
    if ltext.count(run_anchor) != 1:
        raise RuntimeError("run_turn anchor mismatch")
    ltext = ltext.replace(run_anchor, late_helper + run_anchor, 1)
old_late = '''    if (late_supporter_was_unused &&
        late_supporter_quick_ball_latias_completion_available()) {
      play_items_until_stable(true);
      play_basics_from_hand();
    }
    if (use_legacy_star()) {
'''
new_late = '''    if (late_supporter_was_unused &&
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
if new_late not in ltext:
    if ltext.count(old_late) != 1:
        raise RuntimeError("late Supporter replay anchor mismatch")
    ltext = ltext.replace(old_late, new_late, 1)
atomic_write(latias, ltext)

cmake = ROOT / "CMakeLists.txt"
ctext = cmake.read_text(encoding="utf-8")
if "trace_issue_2293_strict_first_seed291" not in ctext:
    ctext += r'''

# K1 Wonder Tag must recover the known prized Latias ex through Gladion when the
# remaining Energy, evolution and current-turn payload axes are independently live.
# https://api.pokemontcg.io/v2/cards/sm2-60
# https://api.pokemontcg.io/v2/cards/sm4-95
# https://api.pokemontcg.io/v2/cards/sv8-76
# https://github.com/FlareZ123/pokemon-sims/issues/2293
add_test(NAME trace_issue_2293_strict_first_seed291
  COMMAND regidrago_sim --simulate-this --deck regidrago-shell
    --scenario strict-jit/go-first --seed 291 --require-ready-by 4)
'''
atomic_write(cmake, ctext)
