from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


SOURCE_PATH = Path(
    "src/trace_engine_v2/part_issue_1071_fss_oricorio_treasure_decomposition_override.inc"
)
TEST_PATH = Path("tests/issue_1379_fss_turn_three_latias_tests.cpp")
LOCK_PATH = Path(".ci/issue-1379-materialize.lock")

INSERTION = """    if (deck_seen_ && scenario_.dci == DciProfile::StrictJit &&
        state_.turn == 3 && supporter_allowed() &&
        !state_.manual_energy_used && !state_.retreat_used && state_.active &&
        state_.active->card == Card::Oricorio && bench_space() > 0 &&
        !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        latias_ability_available && hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::ProfessorBurnet) > 0 && hand_count(Card::Fire) > 0 &&
        payload_might_be_in_deck()) {
      const bool prior_turn_gg_regi = std::any_of(
          state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
            return pokemon.card == Card::RegidragoV &&
                pokemon.entered_turn < state_.turn && pokemon.grass >= 2 &&
                pokemon.fire == 0;
          });
      if (prior_turn_gg_regi) {
        // Professor Burnet already covers the current-turn Dragon-discard axis.
        // Star Alchemy must search Latias ex, whose Skyliner Ability removes
        // Oricorio's Retreat Cost after the held Fire attachment and evolution:
        // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
        // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
        // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
        // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
        // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Supporter, attachment, evolution, Ability, Bench, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1379
        return true;
      }
    }
"""

TEST_CONTENT = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool latias_route(const Engine& engine) {
    return engine.fss_latias_over_redundant_burnet_route_available();
  }
  static Card fss_target(const Engine& engine) {
    return engine.fss_target_after_search_started();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

const sim::DeckRecipe& recipe() {
  static const sim::DeckRecipe value = sim::baseline_recipe();
  return value;
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1379", sim::DciProfile::StrictJit, lock, false, 3};
}

sim::State turn_three_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1},
      sim::Pokemon{sim::Card::RegidragoV, 2, 2, 0,
                   sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::ProfessorBurnet,
                sim::Card::Fire, sim::Card::TeamYellsCheer,
                sim::Card::Powerglass};
  state.deck = {sim::Card::LatiasEx, sim::Card::BrilliantBlender,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::GoodraVstar, sim::Card::Grass,
                sim::Card::MysteriousTreasure, sim::Card::TateLiza};
  state.prizes = {sim::Card::Arven, sim::Card::QuickBall,
                  sim::Card::DialgaGX, sim::Card::Lusamine,
                  sim::Card::ChaoticSwell, sim::Card::Crispin};
  return state;
}

struct Evaluation {
  bool route_available;
  sim::Card target;
};

Evaluation evaluate(sim::State state,
                    const sim::LockMode lock = sim::LockMode::None) {
  std::mt19937_64 rng{1379};
  const sim::Scenario selected_scenario = scenario(lock);
  sim::Engine engine(selected_scenario, recipe(), rng);
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));
  return {sim::EngineTestAccess::latias_route(engine),
          sim::EngineTestAccess::fss_target(engine)};
}

void test_prefers_latias_for_missing_active_position_axis() {
  const Evaluation result = evaluate(turn_three_state());
  // Burnet already searches and discards the same-turn Dragon payload. Latias ex
  // supplies Skyliner for Oricorio's retreat after Fire attachment and evolution,
  // so Star Alchemy must spend its one-use connector on the missing position axis:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(result.route_available,
         "The public T3 state should admit the Latias route");
  expect(result.target == sim::Card::LatiasEx,
         "Star Alchemy should choose Latias over redundant Blender");
}

void test_requires_burnet_payload_outlet() {
  sim::State state = turn_three_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::ProfessorBurnet), state.hand.end());
  const Evaluation result = evaluate(std::move(state));
  // Without Burnet, Brilliant Blender remains the direct payload connector:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(!result.route_available,
         "Latias must require the independent held Burnet outlet");
}

void test_requires_held_fire_for_current_turn_ggf() {
  sim::State state = turn_three_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::Fire), state.hand.end());
  const Evaluation result = evaluate(std::move(state));
  // Apex Dragon requires GGF and only one manual Energy may be attached per turn:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(!result.route_available,
         "The same-turn Latias route must require held Fire Energy");
}

void test_requires_prior_turn_gg_regidrago() {
  sim::State state = turn_three_state();
  state.bench.back().entered_turn = state.turn;
  const Evaluation result = evaluate(std::move(state));
  // A Pokemon cannot evolve during the turn it entered play:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(!result.route_available,
         "A same-turn Regidrago V must reject the T3 route");
}

void test_requires_exact_attached_energy_axis() {
  sim::State state = turn_three_state();
  state.bench.back().grass = 1;
  const Evaluation result = evaluate(std::move(state));
  // One Fire attachment cannot complete Apex Dragon's missing second Grass:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(!result.route_available,
         "Latias must require the visible GG foundation");
}

void test_rejects_rule_box_ability_lock() {
  const Evaluation result = evaluate(turn_three_state(),
                                     sim::LockMode::FullRuleBoxAbility);
  // Skyliner is a Rule Box Pokemon Ability and is disabled by the modeled lock:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(!result.route_available,
         "Rule Box Ability lock must reject Skyliner");
}

void test_requires_bench_space() {
  sim::State state = turn_three_state();
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1});
  state.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  state.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  const Evaluation result = evaluate(std::move(state));
  // The Bench limit is five Pokemon:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(!result.route_available,
         "A full Bench must reject Latias");
}

void test_requires_deck_resident_latias_and_payload() {
  sim::State no_latias = turn_three_state();
  no_latias.deck.erase(std::remove(no_latias.deck.begin(), no_latias.deck.end(),
                                   sim::Card::LatiasEx), no_latias.deck.end());
  const Evaluation no_latias_result = evaluate(std::move(no_latias));
  // Star Alchemy can select only a card remaining in the inspected deck:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(!no_latias_result.route_available,
         "An unavailable Latias must reject the route");

  sim::State no_payload = turn_three_state();
  no_payload.deck.erase(
      std::remove_if(no_payload.deck.begin(), no_payload.deck.end(),
                     [](const sim::Card card) {
                       return card == sim::Card::MegaDragonite ||
                           card == sim::Card::Dragapult ||
                           card == sim::Card::GoodraVstar;
                     }),
      no_payload.deck.end());
  const Evaluation no_payload_result = evaluate(std::move(no_payload));
  // Burnet cannot cover strict-JIT when no legal Dragon payload remains in deck:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(!no_payload_result.route_available,
         "The route must require a deck-resident payload");
}

}  // namespace

int main() {
  test_prefers_latias_for_missing_active_position_axis();
  test_requires_burnet_payload_outlet();
  test_requires_held_fire_for_current_turn_ggf();
  test_requires_prior_turn_gg_regidrago();
  test_requires_exact_attached_energy_axis();
  test_rejects_rule_box_ability_lock();
  test_requires_bench_space();
  test_requires_deck_resident_latias_and_payload();
  return 0;
}
'''


@contextmanager
def locked_file(path: Path) -> Iterator[TextIO]:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = path.open("a+", encoding="utf-8")
    try:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        yield handle
    finally:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
        handle.close()


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def main() -> int:
    with locked_file(LOCK_PATH):
        source = SOURCE_PATH.read_text(encoding="utf-8")
        if "https://github.com/FlareZ123/pokemon-sims/issues/1379" not in source:
            anchor = """    const bool latias_ability_available =
        scenario_.locks != LockMode::FullRuleBoxAbility &&
        scenario_.locks != LockMode::FullCombined;
"""
            if source.count(anchor) < 1:
                raise RuntimeError("Issue 1379 source anchor was not found")
            source = source.replace(anchor, anchor + INSERTION, 1)
            atomic_write(SOURCE_PATH, source)

        atomic_write(TEST_PATH, TEST_CONTENT)

    LOCK_PATH.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
