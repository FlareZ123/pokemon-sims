from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

PATH = Path("tests/late_steven_t3_compression_tests.cpp")
OLD = "  static bool ready(const Engine& engine) { return engine.ready(); }\n"
NEW = """  static bool ready(const Engine& engine) {
    const State& state = engine.state_;
    return state.turn >= 2 && engine.active_is_vstar() &&
        state.active->grass >= 2 && state.active->fire >= 1 && engine.payload_ready();
  }
  static std::string state_line(const Engine& engine) {
    return engine.state_line();
  }
"""
ERROR_OLD = '    throw std::runtime_error("Steven did not preserve the exact T3 route");\n'
ERROR_NEW = """    throw std::runtime_error(
        "Steven did not preserve the exact T3 route: " +
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
    if OLD not in text:
        raise SystemExit("Unable to find generated readiness helper")
    updated = text.replace(OLD, NEW, 1)
    if ERROR_OLD not in updated:
        raise SystemExit("Unable to find generated Steven route assertion")
    updated = updated.replace(ERROR_OLD, ERROR_NEW, 1)
    if MAIN_OLD not in updated:
        raise SystemExit("Unable to find generated Steven test main")
    updated = updated.replace(MAIN_OLD, MAIN_NEW, 1)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=PATH.parent, delete=False
    ) as temporary:
        temporary.write(updated)
        temporary.flush()
        os.fsync(temporary.fileno())
        temporary_path = Path(temporary.name)
    os.replace(temporary_path, PATH)
