import fcntl
import os
import tempfile
from pathlib import Path


def locked_atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lock_path = path.with_name(path.name + ".lock")
    with lock_path.open("a+", encoding="utf-8") as lock_file:
        fcntl.flock(lock_file.fileno(), fcntl.LOCK_EX)
        file_descriptor, temporary_name = tempfile.mkstemp(
            prefix=path.name + ".", suffix=".tmp", dir=path.parent
        )
        try:
            with os.fdopen(file_descriptor, "w", encoding="utf-8", newline="") as temporary_file:
                temporary_file.write(content)
                temporary_file.flush()
                os.fsync(temporary_file.fileno())
            os.replace(temporary_name, path)
        finally:
            if os.path.exists(temporary_name):
                os.unlink(temporary_name)
            fcntl.flock(lock_file.fileno(), fcntl.LOCK_UN)
    lock_path.unlink(missing_ok=True)


source_path = Path("src/trace_engine_v2/part_forretress_ex_combo.inc")
source = source_path.read_text(encoding="utf-8")
old_block = """bool Engine::use_exploding_energy_for_setup() {
  if (!forretress_combo_enabled() || !need_energy() ||
      in_play_count(Card::ForretressEx) == 0 ||
      !ability_available_for_pokemon(Card::ForretressEx)) {
    return false;
  }
  const auto target_index = exploding_energy_regidrago_target_index();
  if (!target_index || !might_be_unseen(Card::Grass)) return false;

  std::vector<std::size_t> destinations;
"""
new_block = """bool Engine::use_exploding_energy_for_setup() {
  const bool promotion_only_finish = prizes_known() && state_.active &&
      state_.active->card == Card::ForretressEx && need_active_vstar() &&
      benched_vstar_promotion_ready() && !need_payload() && !need_energy();
  if (!forretress_combo_enabled() ||
      (!need_energy() && !promotion_only_finish) ||
      in_play_count(Card::ForretressEx) == 0 ||
      !ability_available_for_pokemon(Card::ForretressEx)) {
    return false;
  }
  const auto target_index = exploding_energy_regidrago_target_index();
  if (!target_index || !might_be_unseen(Card::Grass)) return false;

  if (promotion_only_finish) {
    // This K1-only completion attaches one known deck-resident Basic Grass Energy,
    // then applies Exploding Energy's printed self-Knock-Out and promotion. Ability
    // effects using "up to X" require a selection from 1 through X under the
    // February 2026 rule update, so this route never relies on a zero-card search:
    // Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085
    // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Ability, search, attachment, Knock Out, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, current-turn payload, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1561
    return resolve_exploding_energy({*target_index});
  }

  std::vector<std::size_t> destinations;
"""
if old_block in source:
    if source.count(old_block) != 1:
        raise SystemExit("Expected one Exploding Energy setup entry block")
    source = source.replace(old_block, new_block, 1)
elif new_block not in source:
    raise SystemExit("Issue-1561 production patch is neither original nor applied")
locked_atomic_write(source_path, source)


tests = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {

void expect(const bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

bool contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void test_seed_17_attaches_one_grass_then_promotes() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1561 fixture is unavailable.");

  std::mt19937_64 rng{17};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The source-bound K1 state has a current-turn Dragon payload, a Benched GGF
  // Regidrago VSTAR, Active Forretress ex, and searchable Grass. Exploding Energy
  // must choose at least one Grass because it is an Ability, attach that card, then
  // apply its printed self-Knock-Out so Regidrago VSTAR can be promoted on T3:
  // Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Matchup-flex timing and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1561
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "Seed 17 missed its legal T3 ready state.");
  expect(contains(trace, "Exploding Energy attached 1 Basic Grass Energy"),
         "Seed 17 did not make the required nonzero Ability selection.");
  expect(contains(trace, "T3 | PROMOTE") && contains(trace, "T3 | READY"),
         "Seed 17 did not self-Knock Out and promote Regidrago VSTAR on T3.");
}

}  // namespace

int main() {
  try {
    test_seed_17_attaches_one_grass_then_promotes();
    std::cout << "Issue 1561 tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
'''
locked_atomic_write(
    Path("tests/issue_1561_forretress_promotion_tests.cpp"), tests
)
