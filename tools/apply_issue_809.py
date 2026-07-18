from __future__ import annotations

import os
from pathlib import Path


LOCK_PATH = Path(".issue-809-apply.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-809.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 809 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    lock_fd = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(lock_fd, str(os.getpid()).encode("ascii"))

        source = Path("src/trace_engine_v2/part_tapu_tate_switch_override.inc")
        old_source = '''    // Going first prohibits playing a Supporter on turn one. Wonder Tag may still
    // search a concrete Gladion or Arven and leave it in hand for turn two:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/748
    return live_gladion_axis || live_arven_vstar;
'''
        new_source = '''    const auto future_crispin_completes_ggf = [&]() {
      if (!need_energy() || state_.manual_energy_used ||
          hand_count(Card::Crispin) > 0 || !might_be_unseen(Card::Crispin) ||
          !might_be_unseen(Card::Grass) || !might_be_unseen(Card::Fire)) {
        return false;
      }

      const int initial_grass_needed = grass_needed();
      const int initial_fire_needed = fire_needed();
      const int held_grass = hand_count(Card::Grass);
      const int held_fire = hand_count(Card::Fire);

      const auto completes_after_current_manual = [&](const int current_manual) {
        int remaining_grass = initial_grass_needed;
        int remaining_fire = initial_fire_needed;
        int grass_in_hand = held_grass;
        int fire_in_hand = held_fire;

        if (current_manual == 1) {
          if (remaining_grass <= 0 || grass_in_hand <= 0) return false;
          --remaining_grass;
          --grass_in_hand;
        } else if (current_manual == 2) {
          if (remaining_fire <= 0 || fire_in_hand <= 0) return false;
          --remaining_fire;
          --fire_in_hand;
        }

        const auto completes_after_crispin = [&](const bool attach_grass) {
          int grass_after_crispin = remaining_grass;
          int fire_after_crispin = remaining_fire;
          int grass_available_for_manual = grass_in_hand;
          int fire_available_for_manual = fire_in_hand;

          if (attach_grass) {
            if (grass_after_crispin <= 0) return false;
            --grass_after_crispin;
            ++fire_available_for_manual;
          } else {
            if (fire_after_crispin <= 0) return false;
            --fire_after_crispin;
            ++grass_available_for_manual;
          }

          if (grass_after_crispin == 0 && fire_after_crispin == 0) return true;
          if (grass_after_crispin == 1 && fire_after_crispin == 0) {
            return grass_available_for_manual > 0;
          }
          if (grass_after_crispin == 0 && fire_after_crispin == 1) {
            return fire_available_for_manual > 0;
          }
          return false;
        };

        return completes_after_crispin(true) || completes_after_crispin(false);
      };

      // Wonder Tag may bank Crispin on the first player's opening turn. Enumerate
      // the unused current manual attachment, Crispin's different-type search and
      // attachment, and the next turn's manual attachment. This admits the route
      // only when those public resources complete GGF without Vital Dance or Star
      // Alchemy:
      // https://api.pokemontcg.io/v2/cards/cel25c-60_A
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/809
      return completes_after_current_manual(0) ||
             completes_after_current_manual(1) ||
             completes_after_current_manual(2);
    };
    const bool live_crispin_energy = future_crispin_completes_ggf();

    // Going first prohibits playing a Supporter on turn one. Wonder Tag may still
    // search a concrete Gladion, Arven, or Crispin and leave it in hand for turn two:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/748
    // https://github.com/FlareZ123/pokemon-sims/issues/809
    return live_gladion_axis || live_arven_vstar || live_crispin_energy;
'''
        replace_once(source, old_source, new_source)

        cmake = Path("CMakeLists.txt")
        old_cmake = '''enable_testing()
add_test(NAME regidrago_sim_self_test COMMAND regidrago_sim --self-test)
'''
        new_cmake = '''enable_testing()
add_test(NAME regidrago_future_wonder_tag_crispin
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_future_wonder_tag_crispin.cmake)
add_test(NAME regidrago_sim_self_test COMMAND regidrago_sim --self-test)
'''
        replace_once(cmake, old_cmake, new_cmake)
    finally:
        os.close(lock_fd)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
