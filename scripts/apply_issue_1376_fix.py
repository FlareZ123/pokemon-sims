from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


SOURCE_PATH = Path("src/trace_engine_v2/part_issue_1118_secret_box.inc")
LEGACY_TEST_PATH = Path("tests/issue_1118_multi_deck_secret_box_tests.cpp")
NEW_TEST_PATH = Path("tests/issue_1376_t1_pineco_before_steven_tests.cpp")
LOCK_PATH = Path(".issue-1376-fix.lock")


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


def replace_once(text: str, old: str, new: str, description: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Unexpected {description} anchor count: {count}")
    return text.replace(old, new, 1)


NEW_TEST = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool bench_pineco(Engine& engine) {
    return engine.bench_pineco_if_useful();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

std::size_t trace_index(const sim::TraceLog& trace, const std::string& text) {
  const auto it = std::find_if(trace.lines.begin(), trace.lines.end(),
                               [&text](const std::string& line) {
                                 return line.find(text) != std::string::npos;
                               });
  return it == trace.lines.end()
      ? trace.lines.size()
      : static_cast<std::size_t>(std::distance(trace.lines.begin(), it));
}

void test_seed35_benches_pineco_before_turn_ending_steven() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered seed-35 fixture is unavailable.");

  std::mt19937_64 rng(35);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Pineco is a Basic already held before Steven's Resolve ends turn one.
  // Benching it then permits ordinary prior-turn evolution on turn two and
  // preserves the singleton Forest of Vitality plus the Stadium action:
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Core Bench, Supporter, Stadium, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Repository priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1376
  expect(outcome.first_ready_turn == 2,
         "The corrected seed-35 route lost strict-JIT T2 readiness.");
  const std::size_t pineco =
      trace_index(trace, "T1 | BENCH | rules: R-GAME-BENCH | Pineco");
  const std::size_t steven =
      trace_index(trace, "T1 | PLAY SUPPORTER | rules: R-STEVEN-01");
  expect(pineco < steven,
         "Pineco was not Benched before Steven's Resolve ended turn one.");
  expect(trace_contains(trace, "under normal prior-turn timing"),
         "Pineco did not evolve through ordinary prior-turn timing.");
  expect(!trace_contains(
             trace, "T2 | PLAY STADIUM | rules: R-FOREST-VITALITY-01"),
         "The preserved prior-turn route still played Forest of Vitality.");
}

sim::State helper_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Pineco};
  return state;
}

void test_helper_preserves_bench_capacity_gate() {
  const sim::Scenario scenario{"issue-1376-full-bench",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng(137601);
  sim::Engine engine(scenario, sim::pineco_recipe(), rng);
  sim::State state = helper_state();
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 0},
      sim::Pokemon{sim::Card::LatiasEx, 0},
      sim::Pokemon{sim::Card::Oricorio, 0},
      sim::Pokemon{sim::Card::RegidragoV, 0},
      sim::Pokemon{sim::Card::MawileGX, 0},
  };
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The new call site delegates to the existing legal Bench helper, so a full
  // five-Pokémon Bench still blocks Pineco:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1376
  expect(!sim::EngineTestAccess::bench_pineco(engine),
         "Pineco bypassed the five-Pokémon Bench limit.");
  expect(sim::EngineTestAccess::state(engine).hand.size() == 1U,
         "A blocked Pineco Bench action changed the hand.");
}

void test_helper_preserves_energy_need_and_recipe_gates() {
  const sim::Scenario scenario{"issue-1376-controls",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};

  std::mt19937_64 complete_rng(137602);
  sim::Engine complete_engine(scenario, sim::pineco_recipe(), complete_rng);
  sim::State complete = helper_state();
  complete.active->grass = 2;
  complete.active->fire = 1;
  sim::EngineTestAccess::set_state(complete_engine, std::move(complete));
  expect(!sim::EngineTestAccess::bench_pineco(complete_engine),
         "Pineco was Benched after the Energy axis was complete.");

  std::mt19937_64 shell_rng(137603);
  sim::Engine shell_engine(scenario, sim::baseline_recipe(), shell_rng);
  sim::EngineTestAccess::set_state(shell_engine, helper_state());
  // Pineco and Forretress are recipe-gated and absent from the shell:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#named-deck-ownership
  // https://github.com/FlareZ123/pokemon-sims/issues/1376
  expect(!sim::EngineTestAccess::bench_pineco(shell_engine),
         "The Pineco call-site behavior leaked into the shell recipe.");
}

}  // namespace

int main() {
  try {
    test_seed35_benches_pineco_before_turn_ending_steven();
    test_helper_preserves_bench_capacity_gate();
    test_helper_preserves_energy_need_and_recipe_gates();
    std::cout << "Issue 1376 T1 Pineco-before-Steven tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
'''


def main() -> int:
    with locked_file(LOCK_PATH):
        source = SOURCE_PATH.read_text(encoding="utf-8")
        source = replace_once(
            source,
            "      proactive_tapu_retreat_for_pineco();\n"
            "      attach_manual();\n\n"
            "      if (!state_.supporter_used && !play_secret_box_planned_supporter()) {",
            "      proactive_tapu_retreat_for_pineco();\n"
            "      attach_manual();\n\n"
            "      // A held Pineco is a public, costless Basic Bench action. Benching it before\n"
            "      // Steven's Resolve ends turn one preserves ordinary turn-two evolution and\n"
            "      // avoids spending Secret Box's Stadium channel on Forest of Vitality:\n"
            "      // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1\n"
            "      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145\n"
            "      // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117\n"
            "      // Core Bench, Supporter, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules\n"
            "      // Resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n"
            "      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1376\n"
            "      bench_pineco_if_useful();\n"
            "      if (!state_.supporter_used && !play_secret_box_planned_supporter()) {",
            "turn-one Pineco Bench call",
        )
        atomic_write(SOURCE_PATH, source)

        legacy = LEGACY_TEST_PATH.read_text(encoding="utf-8")
        legacy = replace_once(
            legacy,
            "  // Seed 35 proves strict DCI may spend Grant, Wishful Baton, and a\n"
            "  // dynamically replaceable search Item, then use Dawn, Forest, Treasure,\n"
            "  // and Forretress. The exact third cost is intentionally policy-flexible.\n"
            "  expect_seeded_route(35, {\n"
            "      \"Grant (Secret Box cost)\",\n"
            "      \"Wishful Baton (Secret Box cost)\",\n"
            "      \"Secret Box discarded three other cards\",\n"
            "      \"Dawn searched and revealed: Dragapult ex\",\n"
            "      \"Forest of Vitality allowed the newly played Pineco\",\n"
            "      \"Mysterious Treasure cost\",\n"
            "      \"T2 | READY\",\n"
            "  });\n",
            "  // Seed 35 proves strict DCI may spend Grant, Wishful Baton, and a\n"
            "  // dynamically replaceable search Item, then Bench the held Pineco before\n"
            "  // Steven ends turn one. Dawn, ordinary evolution, Treasure, and Forretress\n"
            "  // preserve the T2 route without consuming Forest of Vitality:\n"
            "  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1\n"
            "  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145\n"
            "  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117\n"
            "  // Core Bench, Supporter, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules\n"
            "  // Original route contract: https://github.com/FlareZ123/pokemon-sims/issues/1118\n"
            "  // Stronger resource-preserving route: https://github.com/FlareZ123/pokemon-sims/issues/1376\n"
            "  expect_seeded_route(35, {\n"
            "      \"Grant (Secret Box cost)\",\n"
            "      \"Wishful Baton (Secret Box cost)\",\n"
            "      \"Secret Box discarded three other cards\",\n"
            "      \"T1 | BENCH | rules: R-GAME-BENCH | Pineco from hand\",\n"
            "      \"Dawn searched and revealed: Dragapult ex\",\n"
            "      \"Pineco evolved into Forretress ex under normal prior-turn timing\",\n"
            "      \"Mysterious Treasure cost\",\n"
            "      \"T2 | READY\",\n"
            "  });\n",
            "issue-1118 seed-35 route contract",
        )
        atomic_write(LEGACY_TEST_PATH, legacy)
        atomic_write(NEW_TEST_PATH, NEW_TEST)
    LOCK_PATH.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
