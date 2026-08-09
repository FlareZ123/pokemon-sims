from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"


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


text = PATH.read_text(encoding="utf-8")
old = '''    Pokemon* target = best_benched_vstar_for_promotion();
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
'''
new = '''    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || !pays_apex_energy_cost(*target)) return false;
    const std::size_t target_index =
        static_cast<std::size_t>(target - state_.bench.data());

    const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;
    if (hand_count(payment) == 0) return false;

    // Preflight the complete post-retreat Blender line on a copy before paying any
    // real costs. Blender intentionally declines when a public held payload already
    // has a cheaper live discard outlet, so the retreat route must stay untouched in
    // those states instead of spending mobility first and discovering that policy later.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Official attachment, retreat, and Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Resource-priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug and counterexample discovered in CI: https://github.com/FlareZ123/pokemon-sims/issues/2295
    Engine projected = *this;
    if (!remove_one(projected.state_.hand, payment)) return false;
    if (payment == Card::Grass) ++projected.state_.active->grass;
    else ++projected.state_.active->fire;
    projected.state_.manual_energy_used = true;
    if (payment == Card::Grass) --projected.state_.active->grass;
    else --projected.state_.active->fire;
    projected.state_.discard.push_back(payment);
    std::swap(*projected.state_.active, projected.state_.bench[target_index]);
    projected.state_.retreat_used = true;
    if (!projected.play_brilliant_blender()) return false;

    if (!remove_one(state_.hand, payment)) return false;
    // Oricorio GRI 55 and Tapu Lele-GX each have a one-Colorless Retreat Cost.
    // A held Basic Energy may pay that mobility axis while the semantically
    // Apex-ready Benched Regidrago VSTAR keeps all of its attached Energy, then the
    // already-preflighted Blender route completes the current strict-JIT payload axis.
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attachment, retreat, and Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
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
    std::swap(*state_.active, state_.bench[target_index]);
    state_.retreat_used = true;
    trace("RETREAT", "R-GAME-RETREAT",
          "Paid the one-Energy Basic Active Retreat Cost and promoted the "
          "Apex-ready Regidrago VSTAR.");

    if (!play_brilliant_blender()) {
      throw std::logic_error("Issue-2295 Blender preflight diverged during execution");
    }
    return active_is_vstar() && !need_energy() && !need_payload();
'''
if text.count(old) != 1:
    raise RuntimeError(f"#2295 refinement anchor count is {text.count(old)}")
atomic_write(PATH, text.replace(old, new, 1))
