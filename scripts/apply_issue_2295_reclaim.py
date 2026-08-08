from pathlib import Path
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    fd, temporary = tempfile.mkstemp(prefix=path.name + ".", dir=path.parent, text=True)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary, path)
    except BaseException:
        try:
            os.unlink(temporary)
        except FileNotFoundError:
            pass
        raise


path = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"
text = path.read_text(encoding="utf-8")
anchor = "  bool resolve_final_promotion_and_attachment() {\n"
helper = r'''  bool complete_paid_one_cost_basic_retreat_with_held_blender() {
    if (scenario_.dci != DciProfile::StrictJit || item_locked() ||
        !prizes_known() || !need_active_vstar() || need_energy() ||
        !need_payload() || state_.retreat_used || state_.manual_energy_used ||
        !state_.active || hand_count(Card::BrilliantBlender) == 0 ||
        !payload_might_be_in_deck() || !can_play_payload_this_turn()) {
      return false;
    }
    if (state_.active->card != Card::Oricorio &&
        state_.active->card != Card::TapuLeleGX) {
      return false;
    }

    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || target->grass < 2 || target->fire < 1) return false;

    const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;
    if (hand_count(payment) == 0 || !remove_one(state_.hand, payment)) return false;

    // Oricorio GRI 55 and Tapu Lele-GX each have a one-Colorless Retreat Cost.
    // The spare manual attachment may therefore pay the mobility axis without
    // altering the already-complete GGF VSTAR. Held Brilliant Blender is part of
    // this same atomic route: after promotion it must resolve immediately, because
    // strict-JIT requires a Dragon payload to enter discard during this very turn.
    // Leaving Blender for a later generic Item pass strands seed 86 until T5.
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attachment, Retreat Cost, and Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, DCI/AMR, and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Preserved route boundaries: https://github.com/FlareZ123/pokemon-sims/issues/802 https://github.com/FlareZ123/pokemon-sims/issues/905 https://github.com/FlareZ123/pokemon-sims/issues/1022 https://github.com/FlareZ123/pokemon-sims/issues/1845
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
    if (payment == Card::Grass) ++state_.active->grass;
    else ++state_.active->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          std::string(name(payment)) + " manually to " +
              std::string(name(state_.active->card)) + " for its Retreat Cost.");

    if (payment == Card::Grass) --state_.active->grass;
    else --state_.active->fire;
    state_.discard.push_back(payment);
    std::swap(*state_.active, *target);
    state_.retreat_used = true;
    trace("RETREAT", "R-GAME-RETREAT",
          "Paid the one-Energy Basic Active Retreat Cost and promoted the GGF "
          "Regidrago VSTAR; resolving held Brilliant Blender now for strict-JIT.");

    if (!play_brilliant_blender()) {
      throw std::logic_error("Issue-2295 held Brilliant Blender completion disappeared");
    }
    return active_is_vstar() && !need_energy() && !need_payload();
  }

'''
if helper not in text:
    if text.count(anchor) != 1:
        raise RuntimeError("#2295 resolver anchor mismatch")
    text = text.replace(anchor, helper + anchor, 1)
old = '''    if (benched_vstar_promotion_ready() &&
        retreat_to_benched_vstar_with_latias()) {
      return true;
    }
    if (pay_tapu_retreat_to_ready_benched_vstar()) return true;
'''
new = '''    if (benched_vstar_promotion_ready() &&
        retreat_to_benched_vstar_with_latias()) {
      return true;
    }
    // Skyliner remains preferred because it spends no Energy. When that free
    // route is unavailable, #2295 proves the one-Energy Basic retreat plus held
    // Blender is itself a complete strict-JIT readiness route.
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Confirmed ordering boundary: https://github.com/FlareZ123/pokemon-sims/issues/2295
    if (complete_paid_one_cost_basic_retreat_with_held_blender()) return true;
    if (pay_tapu_retreat_to_ready_benched_vstar()) return true;
'''
if new not in text:
    if text.count(old) != 1:
        raise RuntimeError("#2295 promotion-order anchor mismatch")
    text = text.replace(old, new, 1)
atomic_write(path, text)

cmake = ROOT / "CMakeLists.txt"
c = cmake.read_text(encoding="utf-8")
if "trace_issue_2295_strict_first_seed29" not in c:
    c += r'''

# Paid one-cost Basic retreat plus held Blender is a complete strict-JIT T4 route.
# Oricorio / Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-55 https://api.pokemontcg.io/v2/cards/sm2-60
# Brilliant Blender / Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/swsh12-136
# Retreat/attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
add_test(NAME trace_issue_2295_strict_first_seed29
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 29 --require-ready-by 4)
add_test(NAME trace_issue_2295_strict_first_seed86
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 86 --require-ready-by 4)
'''
    atomic_write(cmake, c)
