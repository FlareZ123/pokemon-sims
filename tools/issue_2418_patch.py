#!/usr/bin/env python3
from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def atomic_locked_write(path: Path, content: str) -> None:
    with path.open("r+", encoding="utf-8") as lock_file:
        fcntl.flock(lock_file.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile(
            mode="w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
        ) as temporary:
            temporary.write(content)
            temporary.flush()
            os.fsync(temporary.fileno())
            temporary_path = Path(temporary.name)
        os.replace(temporary_path, path)
        fcntl.flock(lock_file.fileno(), fcntl.LOCK_UN)


def patch_turo() -> None:
    path = ROOT / "src/trace_engine_v2/part_turo_oricorio_override.inc"
    text = path.read_text(encoding="utf-8")

    text = replace_once(
        text,
        "  bool turo_oricorio_energy_route_live() const {\n",
        """  std::optional<Card> turo_oricorio_finishing_basic_energy() const {
    if (!state_.active || pays_apex_energy_cost(*state_.active)) return std::nullopt;
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (!might_be_unseen(basic)) continue;
      Pokemon projected = *state_.active;
      if (attach_energy_card(projected, basic) && pays_apex_energy_cost(projected)) {
        // Vital Dance searches Basic Energy only. Project the physical attachment
        // semantically so DDE + either Basic is recognized as Apex Dragon's GGF:
        // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
        // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
        return basic;
      }
    }
    return std::nullopt;
  }

  bool turo_oricorio_energy_route_live() const {
""",
        "insert Turo-Oricorio semantic projection",
    )

    text = replace_once(
        text,
        """    // This route completes the Active attacker, so compute the deficit from that
    // exact Pokémon rather than from the policy's separately preferred Regidrago:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed + active_fire_needed != 1) return false;

    const Card needed_energy = active_grass_needed == 1 ? Card::Grass : Card::Fire;
    const bool needed_energy_might_be_in_deck = might_be_unseen(needed_energy);
    if (!needed_energy_might_be_in_deck) return false;
""",
        """    // Test the actual post-attachment Apex payment state. DDE provides two
    // Energy of every type while attached to a Dragon, so raw Basic counters do
    // not identify the final legal Vital Dance target:
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
    const auto needed_energy = turo_oricorio_finishing_basic_energy();
    if (!needed_energy) return false;
""",
        "replace Turo-Oricorio raw deficit",
    )

    text = replace_once(
        text,
        """    const bool active_regidrago_has_ready_direct_evolution = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0 &&
        state_.active->grass >= 2 && state_.active->fire >= 1;
""",
        """    const bool active_regidrago_has_ready_direct_evolution = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0 &&
        // DDE can satisfy two typed units of Apex Dragon while legally attached:
        // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://github.com/FlareZ123/pokemon-sims/issues/2418
        pays_apex_energy_cost(*state_.active);
""",
        "replace direct-evolution readiness",
    )

    text = replace_once(
        text,
        """    const auto promoted = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoVstar && pokemon.grass >= 2 && pokemon.fire >= 1;
    });
""",
        """    const auto promoted = std::find_if(state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
      // Promote any Regidrago VSTAR that semantically pays Apex Dragon, including
      // DDE + Grass and DDE + Fire:
      // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/issues/2418
      return pokemon.card == Card::RegidragoVstar && pays_apex_energy_cost(pokemon);
    });
""",
        "replace promoted VSTAR readiness",
    )

    text = replace_once(
        text,
        """    for (int energy = 0; energy < returned.grass; ++energy) discard_turo_attachment(Card::Grass);
    for (int energy = 0; energy < returned.fire; ++energy) discard_turo_attachment(Card::Fire);
    if (returned.tool == Tool::ForestSealStone) discard_turo_attachment(Card::ForestSealStone);
""",
        """    for (int energy = 0; energy < returned.grass; ++energy) discard_turo_attachment(Card::Grass);
    for (int energy = 0; energy < returned.fire; ++energy) discard_turo_attachment(Card::Fire);
    // Turo discards every attached card. DDE is a physical Special Energy card,
    // so each attached copy must enter both public discard records when returned:
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
    for (int energy = 0; energy < returned.double_dragon; ++energy) {
      discard_turo_attachment(Card::DoubleDragonEnergy);
    }
    if (returned.tool == Tool::ForestSealStone) discard_turo_attachment(Card::ForestSealStone);
""",
        "record returned DDE in discard",
    )

    text = replace_once(
        text,
        """    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const Card needed_energy = active_grass_needed == 1 ? Card::Grass : Card::Fire;
""",
        """    const auto needed_energy = turo_oricorio_finishing_basic_energy();
    if (!needed_energy) return false;
""",
        "replace play-route raw Basic selection",
    )
    text = replace_once(text, "move_deck_to_hand(needed_energy)", "move_deck_to_hand(*needed_energy)", "dereference Vital Dance search")
    text = replace_once(text, "remove_one(state_.hand, needed_energy)", "remove_one(state_.hand, *needed_energy)", "dereference Vital Dance attachment")
    text = replace_once(text, "if (needed_energy == Card::Grass)", "if (*needed_energy == Card::Grass)", "dereference Vital Dance type")

    atomic_locked_write(path, text)


def patch_issue_991() -> None:
    path = ROOT / "src/trace_engine_v2/part_issue_991_wonder_tag_burnet_legacy_star_override.inc"
    text = path.read_text(encoding="utf-8")
    text = replace_once(
        text,
        """    const bool active_regidrago_has_ready_direct_evolution = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0 &&
        state_.active->grass >= 2 && state_.active->fire >= 1;
""",
        """    const bool active_regidrago_has_ready_direct_evolution = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0 &&
        // Semantic Apex payment includes a legally attached DDE plus either Basic:
        // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://github.com/FlareZ123/pokemon-sims/issues/2418
        pays_apex_energy_cost(*state_.active);
""",
        "replace duplicate direct-evolution readiness",
    )
    text = replace_once(
        text,
        """    return std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoVstar && pokemon.grass >= 2 && pokemon.fire >= 1;
    });
""",
        """    return std::any_of(state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
      // The Turo shortcut owns the Active-position axis when the Benched VSTAR
      // semantically pays Apex Dragon, including DDE + either Basic Energy:
      // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/issues/2418
      return pokemon.card == Card::RegidragoVstar && pays_apex_energy_cost(pokemon);
    });
""",
        "replace duplicate benched readiness",
    )
    atomic_locked_write(path, text)


def patch_issue_1595() -> None:
    path = ROOT / "src/trace_engine_v2/part_issue_1595_preserve_turo_over_quick_ball_override.inc"
    text = path.read_text(encoding="utf-8")
    text = replace_once(
        text,
        """        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [](const Pokemon& pokemon) {
                      return pokemon.card == Card::RegidragoVstar &&
                             pokemon.grass >= 2 && pokemon.fire >= 1;
                    }) &&
""",
        """        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
                      // Held Turo directly promotes any semantically powered VSTAR.
                      // DDE + either Basic Energy already pays Apex Dragon's GGF:
                      // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
                      // https://api.pokemontcg.io/v2/cards/swsh12-136
                      // https://github.com/FlareZ123/pokemon-sims/issues/2418
                      return pokemon.card == Card::RegidragoVstar &&
                             pays_apex_energy_cost(pokemon);
                    }) &&
""",
        "replace issue-1595 benched readiness",
    )
    atomic_locked_write(path, text)


def create_tests() -> None:
    path = ROOT / "tests/issue_2418_turo_dde_tests.cpp"
    if path.exists():
        raise RuntimeError(f"test file already exists: {path}")
    content = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  static bool play_turo_promotion(Engine& engine) {
    return engine.play_turo_active_promotion_route();
  }
  static bool held_turo_promotion(const Engine& engine) {
    return engine.held_turo_direct_active_promotion_route();
  }
  static std::optional<Card> turo_oricorio_basic(const Engine& engine) {
    return engine.turo_oricorio_finishing_basic_energy();
  }
  static bool turo_oricorio_live(const Engine& engine) {
    return engine.turo_oricorio_energy_route_live();
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static bool pays_apex(const Engine& engine, const Pokemon& pokemon) {
    return engine.pays_apex_energy_cost(pokemon);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon pokemon(const sim::Card card, const int grass = 0,
                     const int fire = 0, const int dde = 0,
                     const int entered_turn = 1) {
  sim::Pokemon result{card, entered_turn, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

sim::State payload_ready_state() {
  sim::State state;
  state.turn = 3;
  state.discard = {sim::Card::Dragapult};
  state.discarded_this_turn = {sim::Card::Dragapult};
  return state;
}

struct Fixture {
  sim::Scenario scenario{"issue-2418", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2418};
  sim::Engine engine{scenario, recipe, rng};
};

void test_benched_dde_basic_vstar_promotion(const int grass, const int fire) {
  Fixture fixture;
  sim::State state = payload_ready_state();
  state.active = pokemon(sim::Card::Oricorio);
  state.bench = {pokemon(sim::Card::RegidragoVstar, grass, fire, 1)};
  state.hand = {sim::Card::ProfessorTuro};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // DDE supplies two Energy of every type while attached to a Dragon. One Basic
  // Grass or Fire therefore makes the Benched VSTAR pay Apex Dragon's GGF cost.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Official Supporter and Active-replacement procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(sim::EngineTestAccess::play_turo_promotion(fixture.engine),
         "Turo rejected a DDE-plus-Basic complete Benched VSTAR.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Turo did not promote the DDE-complete Regidrago VSTAR.");
  expect(sim::EngineTestAccess::pays_apex(fixture.engine, *after.active),
         "Promoted DDE VSTAR does not semantically pay Apex Dragon.");
}

void test_direct_dde_evolution_suppresses_turo() {
  Fixture fixture;
  sim::State state = payload_ready_state();
  state.active = pokemon(sim::Card::RegidragoV, 1, 0, 1, 1);
  state.bench = {pokemon(sim::Card::RegidragoVstar, 0, 1, 1)};
  state.hand = {sim::Card::ProfessorTuro, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // A prior-turn Regidrago V with DDE + Grass already pays Apex. The held VSTAR
  // can evolve it in place, so spending Turo and returning attached cards is weaker.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(!sim::EngineTestAccess::play_turo_promotion(fixture.engine),
         "Turo preempted the direct DDE-complete evolution route.");
  expect(!sim::EngineTestAccess::held_turo_promotion(fixture.engine),
         "Duplicate Turo readiness helper ignored the direct DDE evolution.");
}

void test_duplicate_turo_helper_accepts_dde_benched_vstar() {
  Fixture fixture;
  sim::State state = payload_ready_state();
  state.active = pokemon(sim::Card::TapuLeleGX);
  state.bench = {pokemon(sim::Card::RegidragoVstar, 0, 1, 1)};
  state.hand = {sim::Card::ProfessorTuro};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The duplicate Wonder Tag/Turo guard must use the same semantic ready attacker.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(sim::EngineTestAccess::held_turo_promotion(fixture.engine),
         "Duplicate Turo helper rejected DDE + Fire Benched VSTAR.");
}

void test_turo_discards_attached_dde() {
  Fixture fixture;
  sim::State state = payload_ready_state();
  state.active = pokemon(sim::Card::RegidragoV, 1, 0, 1, 1);
  state.active->tool = sim::Tool::ForestSealStone;
  state.bench = {pokemon(sim::Card::RegidragoVstar, 2, 1, 0)};
  state.hand = {sim::Card::ProfessorTuro};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Turo returns the Pokémon and discards every attached card. The physical DDE,
  // Basic Grass, and Tool must all enter public discard rather than disappear.
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Official Supporter/discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(sim::EngineTestAccess::play_turo_promotion(fixture.engine),
         "Turo discard-state fixture did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::DoubleDragonEnergy) == 1,
         "Attached DDE disappeared instead of entering discard.");
  expect(std::count(after.discarded_this_turn.begin(), after.discarded_this_turn.end(),
                    sim::Card::DoubleDragonEnergy) == 1,
         "Attached DDE was not recorded in discarded_this_turn.");
  expect(std::count(after.discard.begin(), after.discard.end(), sim::Card::Grass) == 1,
         "Attached Basic Grass disappeared during Turo return.");
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::ForestSealStone) == 1,
         "Attached Forest Seal Stone disappeared during Turo return.");
}

void test_turo_oricorio_projection_accepts_either_basic(const sim::Card basic) {
  Fixture fixture;
  sim::State state = payload_ready_state();
  state.active = pokemon(sim::Card::RegidragoVstar, 0, 0, 1);
  state.bench = {pokemon(sim::Card::Oricorio)};
  state.hand = {sim::Card::ProfessorTuro};
  state.deck = {basic};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Vital Dance searches Basic Energy. With one DDE already attached, either Basic
  // type is a legal one-attachment Apex completion and must survive route projection.
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(sim::EngineTestAccess::turo_oricorio_basic(fixture.engine) == basic,
         "Turo-Oricorio did not project the DDE one-Basic completion.");
  expect(sim::EngineTestAccess::turo_oricorio_live(fixture.engine),
         "Turo-Oricorio route rejected the DDE one-Basic completion.");
}

void test_issue_1595_preserves_quick_ball_for_dde_turo() {
  sim::Scenario scenario{"issue-2418/1595", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, true, 5};
  std::mt19937_64 rng(24181595);
  sim::Engine engine(scenario, sim::double_dragon_modeling_recipe(), rng);
  sim::State state;
  state.turn = 4;
  state.active = pokemon(sim::Card::TapuLeleGX);
  state.bench = {pokemon(sim::Card::RegidragoVstar, 1, 0, 1)};
  state.hand = {sim::Card::ProfessorTuro, sim::Card::BrilliantBlender,
                sim::Card::QuickBall};
  state.deck = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Held Turo already owns the Active-position axis and Blender owns payload.
  // Spending Quick Ball has no marginal setup value when the Benched DDE VSTAR
  // already pays Apex Dragon's cost.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(!sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball was spent instead of being held for direct Turo promotion.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(std::find(after.hand.begin(), after.hand.end(), sim::Card::QuickBall) !=
             after.hand.end(),
         "Quick Ball left hand in the DDE-complete Turo state.");
}

}  // namespace

int main() {
  try {
    test_benched_dde_basic_vstar_promotion(1, 0);
    test_benched_dde_basic_vstar_promotion(0, 1);
    test_direct_dde_evolution_suppresses_turo();
    test_duplicate_turo_helper_accepts_dde_benched_vstar();
    test_turo_discards_attached_dde();
    test_turo_oricorio_projection_accepts_either_basic(sim::Card::Grass);
    test_turo_oricorio_projection_accepts_either_basic(sim::Card::Fire);
    test_issue_1595_preserves_quick_ball_for_dde_turo();
    std::cout << "Issue 2418 DDE Turo tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
'''
    path.write_text(content, encoding="utf-8", newline="\n")


def main() -> int:
    patch_turo()
    patch_issue_991()
    patch_issue_1595()
    create_tests()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
