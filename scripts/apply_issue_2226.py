from pathlib import Path
import fcntl
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lock_path = path.with_name(path.name + ".lock")
    with lock_path.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=path.parent, delete=False
        ) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_name = tmp.name
        os.replace(tmp_name, path)
        fcntl.flock(lock.fileno(), fcntl.LOCK_UN)
    lock_path.unlink(missing_ok=True)


source_path = ROOT / "src/trace_engine_v2/part_issue_1876_treasure_payload_cost_override.inc"
source = source_path.read_text(encoding="utf-8")
anchor = "  std::optional<Card> issue_2227_treasure_gg_tapu_crispin_payload_cost() const {\n"
helper = r'''  std::optional<Card> issue_2226_treasure_gf_oricorio_payload_cost() const {
    const bool route_available =
        scenario_.dci == DciProfile::StrictJit && prizes_known() &&
        !item_locked() && state_.supporter_used &&
        !state_.manual_energy_used && state_.vstar_power_used &&
        state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        state_.active->tool == Tool::ForestSealStone && need_payload() &&
        hand_count(Card::RegidragoVstar) > 0 && bench_space() > 0 &&
        hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) &&
        ability_available_for_pokemon(Card::Oricorio) &&
        deck_count_after_search_started(Card::Oricorio) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0;
    if (!route_available) return std::nullopt;

    // K1 was established before this cost. Arven has already found Forest Seal
    // Stone and this Treasure, Star Alchemy has already supplied Regidrago VSTAR,
    // and the remaining public route is Treasure -> Oricorio -> Vital Dance ->
    // manual Grass -> evolve. Spending a held Dragon as Treasure's printed cost
    // creates the strict current-turn payload while preserving that exact finish.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Oricorio GRI 55 / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Dragapult ex payload witness: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Ability, VSTAR Power, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2226
    for (const Card card : {Card::Dragapult, Card::Appletun,
                            Card::MegaDragonite, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

'''
if helper not in source:
    if source.count(anchor) != 1:
        raise RuntimeError("#2226 helper anchor mismatch on current main")
    source = source.replace(anchor, helper + anchor, 1)
old = "  std::optional<Card> issue_1876_treasure_payload_cost() const {\n"
new = """  std::optional<Card> issue_1876_treasure_payload_cost() const {\n    if (const auto cost = issue_2226_treasure_gf_oricorio_payload_cost()) {\n      return cost;\n    }\n"""
if new not in source:
    if source.count(old) != 1:
        raise RuntimeError("#2226 selector insertion mismatch on current main")
    source = source.replace(old, new, 1)
atomic_write(source_path, source)

test_path = ROOT / "tests/issue_2226_treasure_gf_oricorio_payload_tests.cpp"
test = r'''#define REGIDRAGO_SIM_NO_MAIN
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
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

void test_strict_seed_475_reaches_t4() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2226 strict fixture is unavailable.");

  std::mt19937_64 rng(475);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // T1 Quick Ball already established K1. T4 Arven finds this Treasure plus
  // Forest Seal Stone, Star Alchemy finds Regidrago VSTAR, and Treasure searches
  // Oricorio. Vital Dance supplies Grass for the unused manual attachment. The
  // Treasure cost can therefore spend Dragapult ex for the same-turn payload.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Oricorio GRI 55: https://api.pokemontcg.io/v2/cards/sm2-55
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Ability, VSTAR Power, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2226
  expect(outcome.first_ready_turn == 4,
         "Strict seed 475 did not reach readiness on T4.");
  expect(trace_contains(trace, "Dragapult ex (Mysterious Treasure cost)"),
         "Strict seed 475 did not spend Dragapult ex as Treasure's cost.");
  expect(!trace_contains(trace,
                         "Mysterious Treasure (Mysterious Treasure cost)"),
         "Strict seed 475 still spent the duplicate Treasure.");
}

void test_matchup_flex_seed_475_keeps_t4() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2226 matchup-flex fixture is unavailable.");
  std::mt19937_64 rng(475);
  sim::Engine engine(*scenario, deck->recipe, rng);
  expect(engine.run().first_ready_turn == 4,
         "Issue 2226 regressed the existing matchup-flex T4 finish.");
}
}  // namespace

int main() {
  try {
    test_strict_seed_475_reaches_t4();
    test_matchup_flex_seed_475_keeps_t4();
    std::cout << "Issue 2226 GF Treasure Oricorio payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
'''
atomic_write(test_path, test)
