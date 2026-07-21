from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "trace_engine_v2" / "part_012_gladion_override.inc"
TEST = ROOT / "tests" / "issue_1259_gladion_appletun_tests.cpp"
WORKFLOW = ROOT / ".github" / "workflows" / "apply-issue-1259.yml"
SCRIPT = Path(__file__).resolve()

OLD = """      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::DialgaGX}) {
"""

NEW = """      // Appletun is a modeled Dragon payload and Gladion may exchange itself
      // for that known Prize before a live one-discard Item puts it into discard:
      // https://api.pokemontcg.io/v2/cards/sm4-95
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/sv8-140
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1259
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::DialgaGX,
                                 Card::Appletun}) {
"""

TEST_CONTENT = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static const State& state(const Engine& engine) { return engine.state_; }

  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"issue-1259/gladion-appletun",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 3};
  sim::DeckRecipe recipe{sim::pineco_recipe()};
  std::mt19937_64 rng{1259};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State payload_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::RegidragoV, sim::Card::Grass};
  state.prizes = {sim::Card::Appletun, sim::Card::Fire, sim::Card::Grass,
                  sim::Card::Crispin, sim::Card::Arven,
                  sim::Card::PokemonCommunication};
  return state;
}

void test_known_prized_appletun_completes_payload_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, payload_state());

  // Gladion may exchange itself for known prized Appletun. Mysterious Treasure
  // then discards Appletun while searching the remaining Dragon target, placing a
  // legal attack in discard for Apex Dragon on this strict-JIT turn:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1259
  if (!sim::EngineTestAccess::play_gladion(fixture.engine)) {
    throw std::runtime_error("Gladion rejected the known prized Appletun route.");
  }

  const sim::State& after_gladion = sim::EngineTestAccess::state(fixture.engine);
  if (!after_gladion.supporter_used ||
      !contains(after_gladion.hand, sim::Card::Appletun) ||
      contains(after_gladion.prizes, sim::Card::Appletun) ||
      !contains(after_gladion.prizes, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion did not exchange itself for Appletun.");
  }

  if (!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine)) {
    throw std::runtime_error("Recovered Appletun lacked its live Treasure outlet.");
  }

  const sim::State& after_treasure = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after_treasure.discard, sim::Card::Appletun) ||
      !contains(after_treasure.discard, sim::Card::MysteriousTreasure) ||
      !contains(after_treasure.hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("Treasure failed to discard Appletun and search its target.");
  }
}

void test_no_legal_outlet_preserves_gladion() {
  Fixture fixture;
  sim::State state = payload_state();
  state.hand = {sim::Card::Gladion};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::play_gladion(fixture.engine)) {
    throw std::runtime_error("Gladion recovered Appletun without a discard outlet.");
  }
}

void test_no_remaining_treasure_target_preserves_gladion() {
  Fixture fixture;
  sim::State state = payload_state();
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::play_gladion(fixture.engine)) {
    throw std::runtime_error("Gladion recovered Appletun without a Treasure target.");
  }
}

void test_unknown_prizes_preserve_gladion() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, payload_state(), true, false);
  if (sim::EngineTestAccess::play_gladion(fixture.engine)) {
    throw std::runtime_error("Gladion selected Appletun from unknown Prizes.");
  }
}

void test_item_lock_preserves_gladion() {
  Fixture fixture;
  fixture.scenario.locks = sim::LockMode::FullItem;
  std::mt19937_64 rng{1260};
  sim::Engine engine{fixture.scenario, fixture.recipe, rng};
  sim::EngineTestAccess::set_state(engine, payload_state());
  if (sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion projected the Treasure route through Item lock.");
  }
}

void test_prized_vstar_keeps_higher_priority() {
  Fixture fixture;
  sim::State state = payload_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None};
  state.prizes[1] = sim::Card::RegidragoVstar;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (!sim::EngineTestAccess::play_gladion(fixture.engine)) {
    throw std::runtime_error("Gladion rejected the higher-priority prized VSTAR.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) ||
      !contains(after.prizes, sim::Card::Appletun)) {
    throw std::runtime_error("Appletun displaced the missing VSTAR Prize recovery.");
  }
}

}  // namespace

int main() {
  test_known_prized_appletun_completes_payload_route();
  test_no_legal_outlet_preserves_gladion();
  test_no_remaining_treasure_target_preserves_gladion();
  test_unknown_prizes_preserve_gladion();
  test_item_lock_preserves_gladion();
  test_prized_vstar_keeps_higher_priority();
  std::cout << "Issue 1259 Gladion Appletun tests passed.\n";
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
        raise RuntimeError("Issue 1259 source anchor was not unique.")
    atomic_write(SOURCE, source.replace(OLD, NEW, 1))
    atomic_write(TEST, TEST_CONTENT)
    SCRIPT.unlink()
    WORKFLOW.unlink()


if __name__ == "__main__":
    main()
