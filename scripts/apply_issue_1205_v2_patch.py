from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    descriptor, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def main() -> None:
    source = ROOT / "src/trace_engine_v2/part_006.inc"
    text = source.read_text(encoding="utf-8")
    old = """    if (hand_count(Card::Grass) > grass_needed() + 1) return Card::Grass;
    if (hand_count(Card::Fire) > fire_needed() + 1) return Card::Fire;

    if (flex_fodder && scenario_.dci != DciProfile::StrictJit) {
"""
    new = """    if (hand_count(Card::Grass) > grass_needed() + 1) return Card::Grass;
    if (hand_count(Card::Fire) > fire_needed() + 1) return Card::Fire;

    const int held_payloads_for_turo = static_cast<int>(std::count_if(
        state_.hand.begin(), state_.hand.end(), is_payload));
    const bool latias_is_live_active_connector =
        ability_available_for_pokemon(Card::LatiasEx) &&
        (hand_count(Card::LatiasEx) > 0 || in_play(Card::LatiasEx));
    const bool active_has_retreat_energy = state_.active &&
        state_.active->grass + state_.active->fire > 0;
    const bool turo_is_only_observable_active_connector = state_.active &&
        state_.active->card == Card::TapuLeleGX &&
        !latias_is_live_active_connector && hand_count(Card::TateLiza) == 0 &&
        !active_has_retreat_energy;
    const bool preserve_turo_for_stranded_active =
        scenario_.dci == DciProfile::MatchupFlexJit && flex_fodder &&
        excluded_from_cost && *excluded_from_cost == Card::MysteriousTreasure &&
        !item_locked() && state_.turn == 1 && scenario_.going_first &&
        need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV) &&
        hand_count(Card::ProfessorTuro) > 0 && held_payloads_for_turo >= 2 &&
        turo_is_only_observable_active_connector;
    if (preserve_turo_for_stranded_active) {
      // One of two held Dragon payloads retains attack value after becoming the
      // Mysterious Treasure cost, while singleton Professor Turo remains the only
      // public route that can clear the stranded Active Tapu Lele-GX:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/sv4-171
      // https://api.pokemontcg.io/v2/cards/cel25c-60_A
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed stale-claim bug: https://github.com/FlareZ123/pokemon-sims/issues/1205
      for (const Card card : {Card::Appletun, Card::MegaDragonite,
                              Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX}) {
        if (hand_count(card) > 0) return card;
      }
    }

    if (flex_fodder && scenario_.dci != DciProfile::StrictJit) {
"""
    if text.count(old) != 1:
        raise RuntimeError("Expected one matchup-flex discard anchor")
    atomic_write(source, text.replace(old, new, 1))

    test = ROOT / "tests/issue_1205_preserve_turo_tests.cpp"
    atomic_write(test, r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }

  static std::optional<Card> treasure_cost(const Engine& engine) {
    return engine.choose_discard(false, true, true,
                                 Card::MysteriousTreasure, false);
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"issue-1205/preserve-turo",
                         sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, true, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1205};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State exact_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::DialgaGX,
                sim::Card::Dragapult, sim::Card::ProfessorTuro,
                sim::Card::ForestSealStone, sim::Card::Crispin,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::RegidragoV, sim::Card::LatiasEx,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void expect_cost(sim::Engine& engine, sim::State state,
                 const sim::Card expected, const char* message) {
  sim::EngineTestAccess::set_state(engine, std::move(state));
  if (sim::EngineTestAccess::treasure_cost(engine) != expected) {
    throw std::runtime_error(message);
  }
}

void test_redundant_payload_precedes_only_turo_connector() {
  Fixture fixture;
  // Mysterious Treasure may discard any other held card. A discarded Dragon
  // remains an Apex Dragon attack source, while Professor Turo is the only public
  // Active-position connector in this exact state:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1205
  expect_cost(fixture.engine, exact_state(), sim::Card::Dragapult,
              "Redundant Dragon did not precede the sole Turo connector.");
}

void test_single_payload_stays_protected() {
  Fixture fixture;
  sim::State state = exact_state();
  state.hand.erase(state.hand.begin() + 2);
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "A lone payload was spent to preserve Turo.");
}

void test_active_regidrago_keeps_turo_expendable() {
  Fixture fixture;
  sim::State state = exact_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "Turo was protected without a stranded Active.");
}

void test_latias_connector_keeps_turo_expendable() {
  Fixture fixture;
  sim::State state = exact_state();
  state.hand.push_back(sim::Card::LatiasEx);
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "Turo was protected despite a live Latias connector.");
}

void test_tate_connector_keeps_turo_expendable() {
  Fixture fixture;
  sim::State state = exact_state();
  state.hand.push_back(sim::Card::TateLiza);
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "Turo was protected despite a held Tate & Liza connector.");
}

void test_retreat_energy_keeps_turo_expendable() {
  Fixture fixture;
  sim::State state = exact_state();
  state.active->grass = 1;
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "Turo was protected despite a paid retreat route.");
}

void test_full_bench_keeps_turo_expendable() {
  Fixture fixture;
  sim::State state = exact_state();
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::Oricorio, 0, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::MawileGX, 0, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::Pineco, 0, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::ForretressEx, 0, 0, 0, sim::Tool::None}};
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "Turo was protected when no Regidrago Bench slot existed.");
}

void test_item_lock_keeps_turo_expendable() {
  Fixture fixture;
  fixture.scenario.locks = sim::LockMode::FullItem;
  std::mt19937_64 rng{1206};
  sim::Engine engine{fixture.scenario, fixture.recipe, rng};
  expect_cost(engine, exact_state(), sim::Card::ProfessorTuro,
              "Treasure-specific protection activated through Item lock.");
}

}  // namespace

int main() {
  test_redundant_payload_precedes_only_turo_connector();
  test_single_payload_stays_protected();
  test_active_regidrago_keeps_turo_expendable();
  test_latias_connector_keeps_turo_expendable();
  test_tate_connector_keeps_turo_expendable();
  test_retreat_energy_keeps_turo_expendable();
  test_full_bench_keeps_turo_expendable();
  test_item_lock_keeps_turo_expendable();
  std::cout << "Issue 1205 Turo-preservation tests passed.\n";
  return 0;
}
''')

    (ROOT / ".github/workflows/apply-issue-1205-v2.yml").unlink(missing_ok=True)
    Path(__file__).unlink(missing_ok=True)


if __name__ == "__main__":
    main()
