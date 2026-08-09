from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"
CMAKE = ROOT / "CMakeLists.txt"


def atomic_write_locked(path: Path, text: str) -> None:
    lock_path = path.with_name(f"{path.name}.lock")
    lock_fd = os.open(lock_path, os.O_CREAT | os.O_EXCL | os.O_RDWR)
    try:
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
        ) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_path = Path(tmp.name)
        os.replace(tmp_path, path)
    finally:
        os.close(lock_fd)
        lock_path.unlink(missing_ok=True)


source = SOURCE.read_text(encoding="utf-8")
helper_name = "complete_paid_one_cost_basic_retreat_with_held_blender"
if helper_name not in source:
    marker = "  bool resolve_final_promotion_and_attachment() {\n"
    if source.count(marker) != 1:
        raise RuntimeError(f"resolve marker count {source.count(marker)}")

    helper = '''  bool complete_paid_one_cost_basic_retreat_with_held_blender() {
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
    const std::size_t target_index =
        static_cast<std::size_t>(target - state_.bench.data());

    const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;
    if (hand_count(payment) == 0) return false;

    // Preflight the full route on an isolated copy before spending a real Energy or
    // Retreat action. Brilliant Blender can establish the strict-JIT payload after
    // promotion only when its own current policy gates remain legal in that exact
    // post-retreat state. This keeps the mobility decision systemic rather than
    // guessing from a single visible-card condition:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Official attachment, retreat, and Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Dynamic DCI and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
    Engine projected = *this;
    if (!remove_one(projected.state_.hand, payment)) return false;
    if (payment == Card::Grass) {
      ++projected.state_.active->grass;
    } else {
      ++projected.state_.active->fire;
    }
    projected.state_.manual_energy_used = true;
    if (payment == Card::Grass) {
      --projected.state_.active->grass;
    } else {
      --projected.state_.active->fire;
    }
    projected.state_.discard.push_back(payment);
    std::swap(*projected.state_.active, projected.state_.bench[target_index]);
    projected.state_.retreat_used = true;
    if (!projected.play_brilliant_blender()) return false;

    if (!remove_one(state_.hand, payment)) return false;
    // Oricorio GRI 55 and Tapu Lele-GX each have a one-Colorless Retreat Cost.
    // Either modeled Basic Energy can pay that cost while the Apex-ready Benched
    // Regidrago VSTAR keeps GGF. The preflight above proves Brilliant Blender can
    // then establish a legal current-turn Dragon payload before readiness is checked:
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attachment, retreat, and Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
    if (payment == Card::Grass) {
      ++state_.active->grass;
    } else {
      ++state_.active->fire;
    }
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          std::string(name(payment)) + " manually to " +
              std::string(name(state_.active->card)) + " for its Retreat Cost.");

    if (payment == Card::Grass) {
      --state_.active->grass;
    } else {
      --state_.active->fire;
    }
    state_.discard.push_back(payment);
    std::swap(*state_.active, state_.bench[target_index]);
    state_.retreat_used = true;
    trace("RETREAT", "R-GAME-RETREAT",
          "Paid the one-Energy Basic Active Retreat Cost and promoted the "
          "Apex-ready Regidrago VSTAR.");

    if (!play_brilliant_blender()) {
      throw std::logic_error("Issue-2295 Blender preflight diverged during execution");
    }
    return active_is_vstar() && !need_energy() && !need_payload();
  }

'''
    source = source.replace(marker, helper + marker, 1)

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
    // Preserve zero-Energy Skyliner first. When that route is unavailable, only
    // spend a paid Basic retreat when the complete same-turn Blender continuation
    // has already succeeded on an isolated policy projection:
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Confirmed ordering boundary: https://github.com/FlareZ123/pokemon-sims/issues/2295
    if (complete_paid_one_cost_basic_retreat_with_held_blender()) return true;
    if (pay_tapu_retreat_to_ready_benched_vstar()) return true;
'''
    if source.count(old) != 1:
        raise RuntimeError(f"promotion anchor count {source.count(old)}")
    source = source.replace(old, new, 1)
    atomic_write_locked(SOURCE, source)

cmake = CMAKE.read_text(encoding="utf-8")
if "trace_issue_2295_oricorio_paid_retreat" not in cmake:
    tests = '''

# Exact source-bound witnesses for an ordinary one-Energy Basic retreat followed
# by the held Brilliant Blender strict-JIT payload route:
# Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
# Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Core procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
add_test(NAME trace_issue_2295_oricorio_paid_retreat
  COMMAND regidrago_sim --simulate-this --deck regidrago-shell
          --scenario strict-jit/go-first --seed 29 --require-ready-by 4)
add_test(NAME trace_issue_2295_tapu_paid_retreat
  COMMAND regidrago_sim --simulate-this --deck regidrago-shell
          --scenario strict-jit/go-first --seed 86 --require-ready-by 4)
'''
    atomic_write_locked(CMAKE, cmake.rstrip() + tests + "\n")
