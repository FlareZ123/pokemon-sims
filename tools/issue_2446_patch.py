#!/usr/bin/env python3
from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def atomic_locked_write(path: Path, content: str) -> None:
    with path.open("r+", encoding="utf-8") as lock_file:
        fcntl.flock(lock_file.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", newline="\n",
                                         dir=path.parent, delete=False) as temporary:
            temporary.write(content)
            temporary.flush()
            os.fsync(temporary.fileno())
            temporary_path = Path(temporary.name)
        os.replace(temporary_path, path)
        fcntl.flock(lock_file.fileno(), fcntl.LOCK_UN)


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def patch_source() -> None:
    path = ROOT / "src/trace_engine_v2/part_tapu_tate_switch_override.inc"
    text = path.read_text(encoding="utf-8")
    old = r'''    const auto future_crispin_completes_ggf = [&]() {
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
      // https://api.pokemontcg.io/v2/cards/sm2-60
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/809
      return completes_after_current_manual(0) ||
             completes_after_current_manual(1) ||
             completes_after_current_manual(2);
    };
'''
    new = r'''    const auto future_crispin_completes_ggf = [&]() {
      if (!need_energy() || hand_count(Card::Crispin) > 0 ||
          !might_be_unseen(Card::Crispin)) {
        return false;
      }
      const Pokemon* target = target_regi();
      if (target == nullptr) return false;

      const bool grass_searchable = might_be_unseen(Card::Grass);
      const bool fire_searchable = might_be_unseen(Card::Fire);
      if (!grass_searchable && !fire_searchable) return false;

      const auto next_turn_crispin_completes = [&](Pokemon projected) {
        const auto attach_next_manual = [&](Pokemon after_crispin,
                                            const Card basic) {
          return attach_energy_card(after_crispin, basic) &&
              pays_apex_energy_cost(after_crispin);
        };

        if (grass_searchable && fire_searchable) {
          for (const Card crispin_attachment : {Card::Grass, Card::Fire}) {
            Pokemon after_crispin = projected;
            if (!attach_energy_card(after_crispin, crispin_attachment)) continue;
            if (pays_apex_energy_cost(after_crispin)) return true;
            const Card hand_basic = crispin_attachment == Card::Grass
                ? Card::Fire : Card::Grass;
            if (attach_next_manual(after_crispin, hand_basic)) return true;
          }
          return false;
        }

        // With only one Basic type searchable, Crispin can search that single card
        // under "up to 2" and must put it into hand because there is no second
        // searched Energy to attach. The next turn's manual attachment can still
        // complete a DDE-equipped Dragon even when this turn's manual attachment
        // was already used to attach the DDE itself:
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Manual Energy attachment rule: https://www.pokemon.com/us/pokemon-tcg/rules
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2446
        return attach_next_manual(projected,
                                  grass_searchable ? Card::Grass : Card::Fire);
      };

      // If this turn's manual attachment is still unused, include the legal held
      // Basic choices before projecting next turn. If it was already used, the
      // observed target state already contains that attachment and Crispin still
      // has a fresh manual attachment available on the following turn:
      // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Turn and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed route contract: https://github.com/FlareZ123/pokemon-sims/issues/809
      // Confirmed DDE regression: https://github.com/FlareZ123/pokemon-sims/issues/2446
      if (next_turn_crispin_completes(*target)) return true;
      if (state_.manual_energy_used) return false;
      for (const Card current_basic : {Card::Grass, Card::Fire}) {
        if (hand_count(current_basic) == 0) continue;
        Pokemon after_current_manual = *target;
        if (attach_energy_card(after_current_manual, current_basic) &&
            !pays_apex_energy_cost(after_current_manual) &&
            next_turn_crispin_completes(after_current_manual)) {
          return true;
        }
      }
      return false;
    };
'''
    text = replace_once(text, old, new, "future Crispin semantic projection")
    atomic_locked_write(path, text)


def create_test() -> None:
    path = ROOT / "tests/issue_2446_opening_tapu_dde_crispin_tests.cpp"
    if path.exists():
        raise RuntimeError(f"test already exists: {path}")
    content = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool future_wonder_tag_target(Engine& engine) {
    return engine.future_turn_wonder_tag_route_has_live_target();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon vstar(const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoVstar, 1, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2446", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2446};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State opening_state() {
  sim::State state;
  state.turn = 1;
  state.manual_energy_used = true;
  state.hand = {sim::Card::TapuLeleGX};
  return state;
}

void test_dde_with_only_grass_searchable_banks_crispin() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = vstar(0, 0, 1);
  state.deck = {sim::Card::Crispin, sim::Card::Grass};
  state.prizes = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The T1 manual attachment was already spent on DDE. Wonder Tag can still bank
  // Crispin for T2; Crispin searches the sole available Grass into hand, then the
  // fresh T2 manual attachment makes DDE + Grass pay Apex Dragon.
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2446
  expect(sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Wonder Tag rejected DDE plus sole searchable Grass Crispin line.");
}

void test_dde_with_only_fire_searchable_banks_crispin() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = vstar(0, 0, 1);
  state.deck = {sim::Card::Crispin, sim::Card::Fire};
  state.prizes = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Either Basic type is a legal final unit beside DDE on a Dragon.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Wonder Tag rejected DDE plus sole searchable Fire Crispin line.");
}

void test_basic_only_one_type_cannot_complete() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = vstar(1, 0, 0);
  state.deck = {sim::Card::Crispin, sim::Card::Grass};
  state.prizes = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // One additional Grass cannot satisfy Apex Dragon's Fire requirement without DDE.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(!sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Basic-only one-type Crispin line was incorrectly accepted.");
}

void test_basic_only_two_types_still_completes() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = vstar(1, 0, 0);
  state.deck = {sim::Card::Crispin, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // On T2 Crispin can attach one searched type and put the other into hand for the
  // fresh manual attachment, preserving the pre-DDE Basic-only completion route.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Manual Energy attachment rule: https://www.pokemon.com/us/pokemon-tcg/rules
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Basic-only two-type Crispin route regressed.");
}

}  // namespace

int main() {
  try {
    test_dde_with_only_grass_searchable_banks_crispin();
    test_dde_with_only_fire_searchable_banks_crispin();
    test_basic_only_one_type_cannot_complete();
    test_basic_only_two_types_still_completes();
    std::cout << "Issue 2446 opening Tapu DDE Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
'''
    path.write_text(content, encoding="utf-8", newline="\n")


def main() -> int:
    patch_source()
    create_test()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
