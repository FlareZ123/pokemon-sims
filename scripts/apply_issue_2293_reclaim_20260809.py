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
helper = r'''  bool issue_2293_wonder_tag_prized_latias_gladion_route() const {
    if (scenario_.dci != DciProfile::StrictJit || !scenario_.going_first ||
        scenario_.locks != LockMode::None || state_.turn != 4 ||
        !prizes_known() || !supporter_allowed() || item_locked() ||
        state_.manual_energy_used || state_.retreat_used || !state_.active ||
        state_.active->card != Card::Oricorio || bench_space() == 0 ||
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

    // Wonder Tag may search Gladion; K1 proves Latias ex and Heavy Ball are Prizes.
    // Held Fire plus the unused manual attachment finishes GGF, while Quick Ball and
    // Mysterious Treasure preserve the current-turn Dialga-GX payload route.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Prize, Item, Bench, Ability, attachment, evolution and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, DCI/AMR, Supporter contention and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2293
    return true;
  }

'''
if helper not in text:
    if text.count(anchor) != 1:
        raise RuntimeError("#2293 supporter selector anchor mismatch")
    text = text.replace(anchor, helper + anchor, 1)
old = """  Card choose_supporter_after_search_started() const {\n    if (issue_1797_wonder_tag_steven_route_available()) {\n"""
new = """  Card choose_supporter_after_search_started() const {\n    if (issue_2293_wonder_tag_prized_latias_gladion_route()) {\n      // K1-known prized Latias ex is the missing Active-position connector:\n      // https://api.pokemontcg.io/v2/cards/sm4-95\n      // https://api.pokemontcg.io/v2/cards/sv8-76\n      // https://github.com/FlareZ123/pokemon-sims/issues/2293\n      return Card::Gladion;\n    }\n    if (issue_1797_wonder_tag_steven_route_available()) {\n"""
if new not in text:
    if text.count(old) != 1:
        raise RuntimeError("#2293 selector insertion anchor mismatch")
    text = text.replace(old, new, 1)
atomic_write(part7, text)

latias = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"
l = latias.read_text(encoding="utf-8")
run_anchor = "void run_turn() {\n"
late_helper = r'''  bool complete_issue_2293_late_gladion_latias_promotion() {
    if (scenario_.dci != DciProfile::StrictJit || !scenario_.going_first ||
        scenario_.locks != LockMode::None || state_.turn != 4 ||
        state_.turn_ended || !state_.supporter_used || !prizes_known() ||
        hand_count(Card::LatiasEx) == 0 || in_play(Card::LatiasEx) ||
        bench_space() == 0 || state_.retreat_used || !state_.active ||
        state_.active->card != Card::Oricorio ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        // Gladion puts itself into the Prize cards in exchange for the selected Prize;
        // requiring Gladion in discard here is impossible after a successful exchange.
        // https://api.pokemontcg.io/v2/cards/sm4-95
        prize_count_after_reveal(Card::Gladion) == 0 ||
        prize_count_after_reveal(Card::LatiasEx) != 0 ||
        // Strict-JIT requires a Dragon payload to enter discard this same turn.
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
        !std::any_of(state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(),
                     [this](const Card card) { return is_payload(card); })) {
      return false;
    }

    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || target->grass < 2 || target->fire < 1) return false;

    // Gladion has recovered the K1-known prized Latias ex after the ordinary Basic
    // fixed point. A Basic may still be played to the Bench this turn; Skyliner then
    // gives Basic Oricorio no Retreat Cost, enabling promotion of the complete VSTAR.
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Basic Bench-play, Ability, Supporter and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest complete route and K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed transition bug: https://github.com/FlareZ123/pokemon-sims/issues/2293
    if (!bench_from_hand(Card::LatiasEx, false)) {
      throw std::logic_error("Issue-2293 Gladion-fetched Latias Bench play disappeared");
    }
    if (!retreat_to_benched_vstar_with_latias()) {
      throw std::logic_error("Issue-2293 Skyliner promotion disappeared");
    }
    return active_is_vstar();
  }

'''
if late_helper not in l:
    if l.count(run_anchor) != 1:
        raise RuntimeError("#2293 run_turn helper anchor mismatch")
    l = l.replace(run_anchor, late_helper + run_anchor, 1)
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
    if (complete_issue_2293_late_gladion_latias_promotion()) {
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    if (use_legacy_star()) {
'''
if new_late not in l:
    if l.count(old_late) != 1:
        raise RuntimeError("#2293 late Supporter replay anchor mismatch")
    l = l.replace(old_late, new_late, 1)
atomic_write(latias, l)

cmake = ROOT / "CMakeLists.txt"
c = cmake.read_text(encoding="utf-8")
if "trace_issue_2293_strict_first_seed291" not in c:
    c += r'''

# K1 Wonder Tag must bridge through Gladion to the known prized Latias ex and
# replay that same-turn Basic Bench action before Skyliner promotion.
# Tapu Lele-GX / Gladion / Latias ex: https://api.pokemontcg.io/v2/cards/sm2-60 https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/sv8-76
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Core rules: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2293
add_test(NAME trace_issue_2293_strict_first_seed291
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 291 --require-ready-by 4)
'''
    atomic_write(cmake, c)
