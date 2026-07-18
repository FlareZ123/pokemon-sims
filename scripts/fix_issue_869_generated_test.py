from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

PATH = Path("tests/late_steven_t3_compression_tests.cpp")
READY_OLD = "  static bool ready(const Engine& engine) { return engine.ready(); }\n"
READY_NEW = """  static bool ready(const Engine& engine) {
    const State& state = engine.state_;
    return state.turn >= 2 && engine.active_is_vstar() &&
        state.active->grass >= 2 && state.active->fire >= 1 && engine.payload_ready();
  }
  static std::string state_line(const Engine& engine) {
    return engine.state_line();
  }
"""
INCLUDE_OLD = """#include <algorithm>
#include <stdexcept>
"""
INCLUDE_NEW = """#include <algorithm>
#include <iostream>
#include <stdexcept>
"""
FACTORY_OLD = """sim::Engine make_engine(const std::uint64_t seed) {
  const sim::Scenario scenario{"issue-869-late-steven-t3", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{seed};
  return sim::Engine{scenario, recipe, rng};
}
"""
FACTORY_NEW = """struct EngineFixture {
  sim::Scenario scenario{"issue-869-late-steven-t3", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit EngineFixture(const std::uint64_t seed)
      : rng(seed), engine(scenario, recipe, rng) {}
};

// Engine stores references to Scenario, DeckRecipe, and RNG. Keep those owners alive
// for the entire fixture instead of returning an Engine bound to stack locals:
// https://github.com/FlareZ123/pokemon-sims/issues/869
"""
FIRST_ERROR_OLD = '    throw std::runtime_error("Steven did not preserve the exact T3 route");\n'
FIRST_ERROR_NEW = """    throw std::runtime_error(
        "Steven did not preserve the exact T3 route: " +
        sim::EngineTestAccess::state_line(engine));
"""
READY_ERROR_OLD = '    throw std::runtime_error("confirmed Steven route did not reach readiness on T3");\n'
READY_ERROR_NEW = """    throw std::runtime_error(
        "confirmed Steven route did not reach readiness on T3: " +
        sim::EngineTestAccess::state_line(engine));
"""
MAIN_OLD = """int main() {
  exact_route_reaches_turn_three();
  missing_target_blocks_route();
  full_bench_blocks_route();
  item_lock_blocks_next_turn_treasure();
  return 0;
}
"""
MAIN_NEW = """int main() {
  try {
    exact_route_reaches_turn_three();
    missing_target_blocks_route();
    full_bench_blocks_route();
    item_lock_blocks_next_turn_treasure();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\\n';
    return 1;
  }
}
"""

with PATH.open("r+", encoding="utf-8") as locked:
    fcntl.flock(locked.fileno(), fcntl.LOCK_EX)
    text = locked.read()
    replacements = (
        (READY_OLD, READY_NEW, "readiness helper"),
        (INCLUDE_OLD, INCLUDE_NEW, "iostream include"),
        (FACTORY_OLD, FACTORY_NEW, "owning Engine fixture"),
        (FIRST_ERROR_OLD, FIRST_ERROR_NEW, "Steven route assertion"),
        (READY_ERROR_OLD, READY_ERROR_NEW, "T3 readiness assertion"),
        (MAIN_OLD, MAIN_NEW, "test main"),
    )
    updated = text
    for old, new, label in replacements:
        if updated.count(old) != 1:
            raise SystemExit(f"Unable to find generated {label}")
        updated = updated.replace(old, new, 1)

    for seed in ("869", "8691", "8692"):
        old = f"  sim::Engine engine = make_engine({seed});\n"
        new = f"  EngineFixture fixture({seed});\n  sim::Engine& engine = fixture.engine;\n"
        if updated.count(old) != 1:
            raise SystemExit(f"Unable to find generated Engine construction for seed {seed}")
        updated = updated.replace(old, new, 1)

    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=PATH.parent, delete=False
    ) as temporary:
        temporary.write(updated)
        temporary.flush()
        os.fsync(temporary.fileno())
        temporary_path = Path(temporary.name)
    os.replace(temporary_path, PATH)
