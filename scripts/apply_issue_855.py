from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOCK_PATH = ROOT / ".issue-855.lock"


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent, text=True)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
            handle.write(content)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def replace_once(path: Path, old: str, new: str) -> None:
    current = path.read_text(encoding="utf-8")
    if new in current:
        return
    count = current.count(old)
    if count != 1:
        raise RuntimeError(f"Expected one replacement anchor in {path}, found {count}.")
    atomic_write(path, current.replace(old, new, 1))


def insert_after_once(path: Path, anchor: str, addition: str) -> None:
    current = path.read_text(encoding="utf-8")
    if addition.strip() in current:
        return
    count = current.count(anchor)
    if count != 1:
        raise RuntimeError(f"Expected one insertion anchor in {path}, found {count}.")
    atomic_write(path, current.replace(anchor, anchor + addition, 1))


TEST_SOURCE = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const std::string& label, const std::uint64_t seed)
      : scenario{label, sim::DciProfile::StrictJit, sim::LockMode::None, false, 4},
        recipe(sim::baseline_recipe()),
        rng(seed),
        engine(scenario, recipe, rng) {}
};

sim::State fallback_state(std::vector<sim::Card> prizes) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Gladion};
  state.deck = {sim::Card::RegidragoVstar};
  state.prizes = std::move(prizes);
  return state;
}

void test_erika_is_selected_before_an_unrelated_live_resource() {
  Fixture fixture{"issue-855-erika-low-impact", 85501};
  sim::EngineTestAccess::set_state(
      fixture.engine,
      fallback_state({sim::Card::ForestSealStone, sim::Card::Arven,
                      sim::Card::ErikasInvitation, sim::Card::QuickBall,
                      sim::Card::EarthenVessel, sim::Card::TapuLeleGX}));

  // Gladion must exchange itself for one revealed Prize. Erika's Invitation is the
  // modeled opponent-dependent inert singleton, so the deterministic low-impact
  // fallback selects it before an unrelated live Forest Seal Stone:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/855
  if (!sim::EngineTestAccess::play_gladion(fixture.engine)) {
    throw std::runtime_error("Gladion should perform its mandatory Prize exchange.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::ErikasInvitation) ||
      contains(after.hand, sim::Card::ForestSealStone) ||
      !contains(after.prizes, sim::Card::ForestSealStone) ||
      !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error(
        "Gladion should choose Erika's Invitation before the unrelated live first Prize.");
  }
}

void test_explicit_missing_axis_target_still_precedes_erika() {
  Fixture fixture{"issue-855-explicit-vstar", 85502};
  sim::EngineTestAccess::set_state(
      fixture.engine,
      fallback_state({sim::Card::ErikasInvitation, sim::Card::RegidragoVstar,
                      sim::Card::ForestSealStone, sim::Card::Arven,
                      sim::Card::QuickBall, sim::Card::EarthenVessel}));

  // A revealed Regidrago VSTAR is the exact missing evolution axis and retains
  // priority over the low-impact Erika fallback:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/855
  if (!sim::EngineTestAccess::play_gladion(fixture.engine)) {
    throw std::runtime_error("Gladion should recover the revealed missing VSTAR axis.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) ||
      contains(after.hand, sim::Card::ErikasInvitation) ||
      !contains(after.prizes, sim::Card::ErikasInvitation) ||
      !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error(
        "The exact missing Regidrago VSTAR target should remain ahead of Erika.");
  }
}
}  // namespace

int main() {
  try {
    test_erika_is_selected_before_an_unrelated_live_resource();
    test_explicit_missing_axis_target_still_precedes_erika();
    std::cout << "Gladion Erika low-impact tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
'''


def apply() -> None:
    source_path = ROOT / "src/trace_engine_v2/part_012.inc"
    old_fallback = (
        "    const std::array<Card, 8> lowest_impact{Card::Dipplin, Card::Grass, Card::Fire, "
        "Card::Powerglass, Card::FieldBlower, Card::ChaoticSwell, Card::PathToPeak, Card::MawileGX};"
    )
    new_fallback = """    // Gladion must exchange itself for one revealed Prize. Erika's Invitation is\n    // the current opponent-dependent inert singleton and belongs in the deterministic\n    // low-impact fallback before an arbitrary live setup resource:\n    // https://api.pokemontcg.io/v2/cards/sm4-95\n    // https://api.pokemontcg.io/v2/cards/sv3pt5-160\n    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n    // https://github.com/FlareZ123/pokemon-sims/issues/855\n    const std::array<Card, 8> lowest_impact{Card::Dipplin, Card::Grass, Card::Fire,\n        Card::Powerglass, Card::FieldBlower, Card::ChaoticSwell, Card::PathToPeak,\n        Card::ErikasInvitation};"""
    replace_once(source_path, old_fallback, new_fallback)

    test_path = ROOT / "tests/gladion_erika_low_impact_tests.cpp"
    if not test_path.exists() or test_path.read_text(encoding="utf-8") != TEST_SOURCE:
        atomic_write(test_path, TEST_SOURCE)

    cmake_path = ROOT / "CMakeLists.txt"
    target_anchor = (
        "add_executable(regidrago_tapu_gladion_prize_tests tests/tapu_gladion_prize_fallback_tests.cpp)\n"
        "target_compile_options(regidrago_tapu_gladion_prize_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)\n"
    )
    target_addition = (
        "\n# Gladion low-impact Prize fallback regression: https://github.com/FlareZ123/pokemon-sims/issues/855\n"
        "add_executable(regidrago_gladion_erika_low_impact_tests tests/gladion_erika_low_impact_tests.cpp)\n"
        "target_compile_options(regidrago_gladion_erika_low_impact_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)\n"
    )
    insert_after_once(cmake_path, target_anchor, target_addition)

    test_anchor = (
        "add_test(NAME regidrago_tapu_gladion_prize COMMAND regidrago_tapu_gladion_prize_tests)\n"
    )
    test_addition = (
        "add_test(NAME regidrago_gladion_erika_low_impact COMMAND regidrago_gladion_erika_low_impact_tests)\n"
    )
    insert_after_once(cmake_path, test_anchor, test_addition)


if __name__ == "__main__":
    LOCK_PATH.touch(exist_ok=True)
    with LOCK_PATH.open("r+", encoding="utf-8") as lock_handle:
        fcntl.flock(lock_handle.fileno(), fcntl.LOCK_EX)
        apply()
