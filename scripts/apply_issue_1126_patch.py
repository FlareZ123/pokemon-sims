from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def replace_once(path: str, old: str, new: str) -> None:
    target = ROOT / path
    text = target.read_text(encoding="utf-8")
    if text.count(old) != 1:
        raise RuntimeError(f"Expected one replacement point in {path}, found {text.count(old)}")
    atomic_write(target, text.replace(old, new, 1))


replace_once(
    "src/trace_engine_v2/part_004.inc",
    """  bool can_free_retreat_with_latias() const {
    return state_.active && is_basic(state_.active->card) && in_play(Card::LatiasEx) &&
           ability_available_for_pokemon(Card::LatiasEx) && !state_.retreat_used;
  }
""",
    """  bool can_free_retreat_with_latias() const {
    return state_.active && is_basic(state_.active->card) && in_play(Card::LatiasEx) &&
           ability_available_for_pokemon(Card::LatiasEx) && !state_.retreat_used;
  }

  bool latias_serena_completion_ready() const {
    // Serena can establish the strict-JIT Dragon payload while Skyliner promotes
    // the already powered Benched Regidrago VSTAR during the same turn. Gladion
    // must preserve the single Supporter play for this complete two-axis route:
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Mega Dragonite ex, a modeled Dragon payload: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Latias ex and Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Regidrago VSTAR and Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Bench, Ability, Supporter, discard, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1126
    const bool latias_available = in_play(Card::LatiasEx) ||
        (hand_count(Card::LatiasEx) > 0 && bench_space() > 0);
    return supporter_allowed() && need_active_vstar() && !need_energy() &&
        need_payload() && can_play_payload_this_turn() && state_.active &&
        is_basic(state_.active->card) && !state_.retreat_used &&
        benched_vstar_promotion_ready() &&
        ability_available_for_pokemon(Card::LatiasEx) && latias_available &&
        hand_count(Card::Serena) > 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
  }
""",
)

replace_once(
    "src/trace_engine_v2/part_012.inc",
    """      const bool direct_supporter = should_play_steven() ||
                                    (need_energy() && hand_count(Card::Crispin) > 0 && crispin_can_advance_energy_axis()) ||
                                    active_burnet_completion ||
                                    latias_burnet_completion ||
                                    (need_payload() && item_locked() && hand_count(Card::ProfessorBurnet) > 0) ||
                                    (!item_locked() && hand_count(Card::Arven) > 0);
""",
    """      const bool direct_supporter = should_play_steven() ||
                                    (need_energy() && hand_count(Card::Crispin) > 0 && crispin_can_advance_energy_axis()) ||
                                    active_burnet_completion ||
                                    latias_burnet_completion ||
                                    latias_serena_completion_ready() ||
                                    (need_payload() && item_locked() && hand_count(Card::ProfessorBurnet) > 0) ||
                                    (!item_locked() && hand_count(Card::Arven) > 0);
""",
)

replace_once(
    "src/trace_engine_v2/part_014b.inc",
    """    if (serena_completes_only_missing_axis && play_serena(true)) return;

    // Roseanne's Backup can restore a known discarded Regidrago VSTAR before a
""",
    """    if (serena_completes_only_missing_axis && play_serena(true)) return;
    // A legal Latias ex promotion lets Serena complete the payload and Active axes
    // before Gladion consumes the only Supporter play:
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/1126
    if (latias_serena_completion_ready() && play_serena(true)) return;

    // Roseanne's Backup can restore a known discarded Regidrago VSTAR before a
""",
)

replace_once(
    "CMakeLists.txt",
    """add_test(NAME trace_issue_1114_latias_burnet_preempts_gladion
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 2932 --require-ready-by 4)

add_test(NAME regidrago_sim_matrix_smoke""",
    """add_test(NAME trace_issue_1114_latias_burnet_preempts_gladion
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 2932 --require-ready-by 4)

# A legal Latias ex promotion plus Serena's mandatory Dragon discard completes
# both remaining axes before Gladion may spend the Supporter play:
# https://api.pokemontcg.io/v2/cards/sv8-76
# https://api.pokemontcg.io/v2/cards/swsh12-164
# https://api.pokemontcg.io/v2/cards/me2pt5-152
# https://api.pokemontcg.io/v2/cards/sm4-95
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://www.pokemon.com/us/pokemon-tcg/rules
# https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# https://github.com/FlareZ123/pokemon-sims/issues/1126
add_test(NAME trace_issue_1126_latias_serena_preempts_gladion
  COMMAND regidrago_sim --simulate-this --scenario strict-jit-full-item-lock/go-first --seed 16189 --require-ready-by 5)

add_test(NAME regidrago_sim_matrix_smoke""",
)

atomic_write(
    ROOT / "tests/issue_1126_latias_serena_preempts_gladion_tests.cpp",
    r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool route_ready(const Engine& engine) {
    return engine.latias_serena_completion_ready();
  }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State exact_state() {
  sim::State state;
  state.turn = 5;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::LatiasEx, sim::Card::Serena,
                sim::Card::MegaDragonite, sim::Card::Gladion};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Dragapult};
  state.prizes = {sim::Card::MegaDragonite, sim::Card::Lusamine,
                  sim::Card::Grass, sim::Card::RegidragoV,
                  sim::Card::QuickBall, sim::Card::MysteriousTreasure};
  state.discard = {sim::Card::Arven};
  return state;
}

sim::Scenario scenario(const sim::LockMode locks = sim::LockMode::FullItem) {
  return sim::Scenario{"issue-1126-latias-serena", sim::DciProfile::StrictJit,
                       locks, true, 5};
}

bool route_ready_for(sim::State state,
                     const sim::LockMode locks = sim::LockMode::FullItem) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1126};
  sim::Engine engine(scenario(locks), recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::route_ready(engine);
}

void test_route_blocks_gladion() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1126};
  sim::Engine engine(scenario(), recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_state());

  // Serena's mandatory Dragon discard establishes the strict-JIT payload, while
  // Latias ex gives the Basic Active Dialga-GX no Retreat Cost and promotes the
  // prepared GGF Regidrago VSTAR. Gladion advances neither remaining axis:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Bench, Ability, Supporter, discard, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest-ready route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1126
  expect(sim::EngineTestAccess::route_ready(engine),
         "The exact Latias-Serena route must be recognized.");
  expect(!sim::EngineTestAccess::play_gladion(engine),
         "Gladion must yield to the current-turn Latias-Serena completion.");
}

void test_turn_policy_completes_both_axes() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1126};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario(), recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, exact_state());

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used, "Serena must consume the Supporter play.");
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Skyliner must promote the prepared Regidrago VSTAR.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Serena must establish the current-turn Dragon payload.");
  expect(trace_contains(trace, "Mega Dragonite ex"),
         "The trace must record Serena's Dragon discard.");
  expect(!trace_contains(trace, "exchanged Gladion"),
         "The trace must not exchange Gladion.");
}

void test_negative_controls() {
  sim::State state = exact_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::MegaDragonite), state.hand.end());
  expect(!route_ready_for(state), "A modeled Dragon payload is required.");

  state = exact_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::LatiasEx), state.hand.end());
  expect(!route_ready_for(state), "Latias ex must be available.");

  state = exact_state();
  state.bench = {state.bench.front(),
                 sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
                 sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None},
                 sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None},
                 sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None}};
  expect(!route_ready_for(state), "A held Latias ex needs an open Bench slot.");

  state = exact_state();
  state.bench.front().grass = 1;
  expect(!route_ready_for(state), "The Benched VSTAR must already pay GGF.");

  state = exact_state();
  state.retreat_used = true;
  expect(!route_ready_for(state), "The retreat action must remain unused.");

  state = exact_state();
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 0,
                              sim::Tool::None};
  expect(!route_ready_for(state), "Skyliner requires a Basic Active.");

  expect(!route_ready_for(exact_state(), sim::LockMode::FullCombined),
         "Rule Box Ability lock must disable Skyliner.");
}
}  // namespace

int main() {
  test_route_blocks_gladion();
  test_turn_policy_completes_both_axes();
  test_negative_controls();
  return 0;
}
''',
)
