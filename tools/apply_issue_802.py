from __future__ import annotations

import os
from pathlib import Path


LOCK_PATH = Path(".issue-802-apply.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-802.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 802 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    lock_fd = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(lock_fd, str(os.getpid()).encode("ascii"))

        source = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
        old_source = '''  void run_turn() {
'''
        new_source = '''  bool pay_tapu_retreat_to_ready_benched_vstar() {
    if (!need_active_vstar() || need_energy() || need_payload() ||
        state_.retreat_used || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX) {
      return false;
    }

    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || target->grass < 2 || target->fire < 1) return false;

    const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;
    if (hand_count(payment) == 0 || !remove_one(state_.hand, payment)) return false;

    // Tapu Lele-GX has a one-Colorless Retreat Cost, so either modeled Basic
    // Energy may be attached for the turn and immediately discarded to promote an
    // already-GGF Regidrago VSTAR. Restrict this route to states where Energy and
    // payload axes are already complete, preserving the attachment for setup when
    // the Benched VSTAR still needs it:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/802
    if (payment == Card::Grass) {
      ++state_.active->grass;
      --state_.active->grass;
    } else {
      ++state_.active->fire;
      --state_.active->fire;
    }
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          std::string(name(payment)) +
              " manually to Tapu Lele-GX for its Retreat Cost.");

    state_.discard.push_back(payment);
    std::swap(*state_.active, *target);
    state_.retreat_used = true;
    trace("RETREAT", "R-GAME-RETREAT",
          "Paid Tapu Lele-GX's one-Energy Retreat Cost and promoted the ready "
          "Benched Regidrago VSTAR.");
    return true;
  }

  void run_turn() {
'''
        replace_once(source, old_source, new_source)

        old_call = '''    attach_manual();
    retreat_to_benched_vstar_with_latias();
    if (need_active_vstar() && !state_.supporter_used) play_tate_switch();
'''
        new_call = '''    if (!pay_tapu_retreat_to_ready_benched_vstar()) attach_manual();
    retreat_to_benched_vstar_with_latias();
    if (need_active_vstar() && !state_.supporter_used) play_tate_switch();
'''
        replace_once(source, old_call, new_call)

        cmake = Path("CMakeLists.txt")
        old_cmake = '''enable_testing()
add_test(NAME regidrago_sim_self_test COMMAND regidrago_sim --self-test)
'''
        new_cmake = '''enable_testing()
add_test(NAME regidrago_active_tapu_paid_retreat
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_active_tapu_paid_retreat.cmake)
add_test(NAME regidrago_sim_self_test COMMAND regidrago_sim --self-test)
'''
        replace_once(cmake, old_cmake, new_cmake)
    finally:
        os.close(lock_fd)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
