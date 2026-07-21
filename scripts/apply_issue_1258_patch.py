from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "trace_engine_v2" / "part_010_steven_crispin_override.inc"
TEST = ROOT / "tests" / "issue_1258_late_steven_appletun_tests.cpp"
WORKFLOW = ROOT / ".github" / "workflows" / "apply-issue-1258.yml"
SCRIPT = Path(__file__).resolve()

OLD = """    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1023
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::DialgaGX, Card::GoodraVstar}) {
"""

NEW = """    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1023
    // Appletun is a Dragon Pokémon and a modeled Apex Dragon payload, so it is a
    // legal Steven target for the next-turn Mysterious Treasure discard bridge:
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1258
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::DialgaGX, Card::GoodraVstar,
                               Card::Appletun}) {
"""

TEST_CONTENT = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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

  static const State& state(const Engine& engine) { return engine.state_; }

  static std::optional<Card> late_steven_payload(const Engine& engine) {
    return engine.late_steven_payload_treasure_bridge_payload();
  }

  static bool play_late_steven_payload_bridge(Engine& engine) {
    return engine.play_late_steven_payload_treasure_bridge();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"issue-1258/late-steven-appletun",
                         sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::pineco_recipe()};
  std::mt19937_64 rng{1258};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State bridge_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Appletun, sim::Card::RegidragoV, sim::Card::Grass};
  return state;
}

void expect_no_payload(sim::Engine& engine, sim::State state,
                       const char* message) {
  sim::EngineTestAccess::set_state(engine, std::move(state));
  if (sim::EngineTestAccess::late_steven_payload(engine).has_value()) {
    throw std::runtime_error(message);
  }
}

void test_appletun_is_selected_and_searched() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, bridge_state());

  // Steven's Resolve can search Appletun, and the held Mysterious Treasure can
  // discard it next turn while searching the remaining Dragon target. Appletun's
  // attack is then a legal Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1258
  const std::optional<sim::Card> selected =
      sim::EngineTestAccess::late_steven_payload(fixture.engine);
  if (selected != sim::Card::Appletun) {
    throw std::runtime_error("Late Steven bridge did not select Appletun.");
  }

  if (!sim::EngineTestAccess::play_late_steven_payload_bridge(fixture.engine)) {
    throw std::runtime_error("Late Steven bridge rejected the Appletun route.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.supporter_used || !after.turn_ended ||
      !contains(after.discard, sim::Card::StevensResolve) ||
      !contains(after.hand, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun)) {
    throw std::runtime_error("Late Steven bridge resolved without searching Appletun.");
  }
}

void test_one_treasure_target_is_rejected() {
  Fixture fixture;
  sim::State state = bridge_state();
  state.deck = {sim::Card::Appletun, sim::Card::Grass};
  expect_no_payload(fixture.engine, std::move(state),
                    "Bridge accepted Appletun without a surviving Treasure target.");
}

void test_item_lock_is_rejected() {
  Fixture fixture;
  fixture.scenario.locks = sim::LockMode::FullItem;
  std::mt19937_64 rng{1259};
  sim::Engine engine{fixture.scenario, fixture.recipe, rng};
  expect_no_payload(engine, bridge_state(),
                    "Bridge projected Mysterious Treasure through Item lock.");
}

void test_faster_current_turn_routes_are_preserved() {
  Fixture burnet_fixture;
  sim::State burnet_state = bridge_state();
  burnet_state.hand.push_back(sim::Card::ProfessorBurnet);
  expect_no_payload(burnet_fixture.engine, std::move(burnet_state),
                    "Bridge displaced the faster Professor Burnet route.");

  Fixture blender_fixture;
  sim::State blender_state = bridge_state();
  blender_state.hand.push_back(sim::Card::BrilliantBlender);
  expect_no_payload(blender_fixture.engine, std::move(blender_state),
                    "Bridge displaced the faster Brilliant Blender route.");
}

void test_held_payload_is_preserved() {
  Fixture fixture;
  sim::State state = bridge_state();
  state.hand.push_back(sim::Card::DialgaGX);
  expect_no_payload(fixture.engine, std::move(state),
                    "Bridge searched Appletun while a current payload was held.");
}

}  // namespace

int main() {
  test_appletun_is_selected_and_searched();
  test_one_treasure_target_is_rejected();
  test_item_lock_is_rejected();
  test_faster_current_turn_routes_are_preserved();
  test_held_payload_is_preserved();
  std::cout << "Issue 1258 late Steven Appletun tests passed.\n";
  return 0;
}
'''


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temp_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp_name, path)
    except BaseException:
        try:
            os.unlink(temp_name)
        except FileNotFoundError:
            pass
        raise


def main() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    if source.count(OLD) != 1:
        raise RuntimeError("Issue 1258 source anchor was not unique.")
    atomic_write(SOURCE, source.replace(OLD, NEW, 1))
    atomic_write(TEST, TEST_CONTENT)
    SCRIPT.unlink()
    WORKFLOW.unlink()


if __name__ == "__main__":
    main()
