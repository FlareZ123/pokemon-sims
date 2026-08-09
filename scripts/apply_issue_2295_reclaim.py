from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"
CMAKE = ROOT / "CMakeLists.txt"


def atomic_write(path: Path, text: str) -> None:
    lock_path = path.with_name(f"{path.name}.lock")
    with lock_path.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile("w", encoding="utf-8", newline="\n", dir=path.parent, delete=False) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_name = tmp.name
        os.replace(tmp_name, path)
    lock_path.unlink(missing_ok=True)


source = SOURCE.read_text(encoding="utf-8")
helper_anchor = "  bool resolve_final_promotion_and_attachment() {\n"
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
    if (target == nullptr || !pays_apex_energy_cost(*target)) return false;

    const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;
    if (hand_count(payment) == 0 || !remove_one(state_.hand, payment)) return false;

    // Oricorio GRI 55 and Tapu Lele-GX each have a one-Colorless Retreat Cost.
    // A held Basic Energy may pay that mobility axis while the semantically
    // Apex-ready Benched Regidrago VSTAR keeps all of its attached Energy. After
    // promotion, prefer an already-live Professor Burnet payload route so the
    // one-copy ACE SPEC is preserved; otherwise Brilliant Blender completes the
    // same-turn strict-JIT payload axis.
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attachment, retreat, Item, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
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
          "Paid the one-Energy Basic Active Retreat Cost and promoted the "
          "Apex-ready Regidrago VSTAR.");

    if (professor_burnet_has_live_ready_turn_route() && play_professor_burnet()) {
      return active_is_vstar() && !need_energy() && !need_payload();
    }
    if (!play_brilliant_blender()) {
      throw std::logic_error("Issue-2295 held Brilliant Blender completion disappeared");
    }
    return active_is_vstar() && !need_energy() && !need_payload();
  }

'''
if helper not in source:
    if source.count(helper_anchor) != 1:
        raise RuntimeError(f"#2295 helper anchor count is {source.count(helper_anchor)}")
    source = source.replace(helper_anchor, helper + helper_anchor, 1)

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
    // Skyliner remains the zero-Energy preference. When it is unavailable, a
    // held one-Energy paid retreat plus a guaranteed current-turn payload outlet
    // is a complete route rather than a reason to spend a setup-dead Supporter.
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Confirmed ordering boundary: https://github.com/FlareZ123/pokemon-sims/issues/2295
    if (complete_paid_one_cost_basic_retreat_with_held_blender()) return true;
    if (pay_tapu_retreat_to_ready_benched_vstar()) return true;
'''
if new not in source:
    if source.count(old) != 1:
        raise RuntimeError(f"#2295 resolver anchor count is {source.count(old)}")
    source = source.replace(old, new, 1)
atomic_write(SOURCE, source)

cmake = CMAKE.read_text(encoding="utf-8")
registration = r'''
# Exact source-bound witnesses for an ordinary one-Energy Basic retreat plus
# held Brilliant Blender strict-JIT route. The production source carries direct
# card/rule URLs: https://github.com/FlareZ123/pokemon-sims/issues/2295
add_test(NAME trace_issue_2295_oricorio_paid_retreat
  COMMAND regidrago_sim --simulate-this --deck regidrago-shell
          --scenario strict-jit/go-first --seed 29 --require-ready-by 4)
add_test(NAME trace_issue_2295_tapu_paid_retreat
  COMMAND regidrago_sim --simulate-this --deck regidrago-shell
          --scenario strict-jit/go-first --seed 86 --require-ready-by 4)
'''
if "trace_issue_2295_oricorio_paid_retreat" not in cmake:
    cmake = cmake.rstrip() + "\n" + registration
atomic_write(CMAKE, cmake)
