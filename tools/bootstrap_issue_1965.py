from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/trace_engine_v2/part_klara_recovery_override.inc"
TEST = ROOT / "tests/issue_1965_klara_crobat_tests.cpp"
WORKFLOW = ROOT / ".github/workflows/bootstrap-issue-1965.yml"
LOCK = ROOT / ".git/issue-1965-bootstrap.lock"


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    temporary = Path(temporary_name)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(content)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def replace_once(content: str, old: str, new: str) -> str:
    count = content.count(old)
    if count != 1:
        raise RuntimeError(f"Expected one source anchor, found {count}: {old[:100]!r}")
    return content.replace(old, new, 1)


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
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_klara(Engine& engine) { return engine.play_klara_recovery(); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static bool ready(const Engine& engine) {
    return engine.active_is_vstar() && engine.state_.active->grass >= 2 &&
           engine.state_.active->fire >= 1 && engine.payload_ready();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::DeckRecipe crobat_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::CrobatV, 1);
  return recipe;
}

sim::State exact_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Klara};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::Grass,
                   sim::Card::CrobatV, sim::Card::Dragapult};
  state.deck = {
      sim::Card::MegaDragonite, sim::Card::GoodraVstar,
      sim::Card::DialgaGX, sim::Card::QuickBall,
      sim::Card::MysteriousTreasure, sim::Card::Arven,
      sim::Card::Crispin, sim::Card::Fire,
      sim::Card::Grass, sim::Card::BrilliantBlender,
  };
  state.vstar_power_used = true;
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const sim::LockMode lock, const std::uint64_t seed,
          sim::State state)
      : scenario{"issue-1965", sim::DciProfile::StrictJit, lock, false, 4},
        rng(seed),
        engine(scenario, crobat_recipe(), rng) {
    sim::EngineTestAccess::set_state(engine, std::move(state));
  }
};

void expect_dragon_fallback(sim::Engine& engine, const char* message) {
  expect(sim::EngineTestAccess::play_klara(engine), message);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Dragapult) &&
             !contains(after.hand, sim::Card::CrobatV),
         "The existing older-Dragon fallback was not preserved.");
}

void test_live_crobat_connector_completes_the_exact_turn() {
  Fixture fixture(sim::LockMode::None, 1965, exact_state());
  sim::EngineTestAccess::run_turn(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);

  // Klara may recover up to two Pokemon. Once its direct VSTAR and Energy targets
  // earn the Supporter action, Crobat V is the superior free second selection when
  // playing it from hand to the Bench will resolve Dark Asset and expose the live
  // Brilliant Blender route:
  // https://api.pokemontcg.io/v2/cards/swsh6-145
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/1965
  expect(sim::EngineTestAccess::ready(fixture.engine),
         "The exact Klara-Crobat-Blender route did not reach readiness.");
  expect(after.dark_asset_used &&
             std::any_of(after.bench.begin(), after.bench.end(),
                         [](const sim::Pokemon& pokemon) {
                           return pokemon.card == sim::Card::CrobatV;
                         }),
         "Klara did not recover and Bench the live Crobat V connector.");
  expect(!contains(after.hand, sim::Card::Dragapult) &&
             std::any_of(after.discarded_this_turn.begin(),
                         after.discarded_this_turn.end(), sim::is_payload),
         "The stranded Dragon still displaced the current-turn payload route.");
}

void test_used_dark_asset_preserves_dragon_fallback() {
  sim::State state = exact_state();
  state.dark_asset_used = true;
  Fixture fixture(sim::LockMode::None, 1966, std::move(state));
  expect_dragon_fallback(fixture.engine,
      "Klara should remain on the Dragon fallback after Dark Asset is used.");
}

void test_rule_box_lock_preserves_dragon_fallback() {
  Fixture fixture(sim::LockMode::FullRuleBoxAbility, 1967, exact_state());
  expect_dragon_fallback(fixture.engine,
      "Klara should remain on the Dragon fallback under Rule Box Ability lock.");
}

void test_full_bench_preserves_dragon_fallback() {
  sim::State state = exact_state();
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1},
      sim::Pokemon{sim::Card::RegidragoV, 1},
      sim::Pokemon{sim::Card::Oricorio, 1},
      sim::Pokemon{sim::Card::TapuLeleGX, 1},
      sim::Pokemon{sim::Card::LatiasEx, 1},
  };
  Fixture fixture(sim::LockMode::None, 1968, std::move(state));
  expect_dragon_fallback(fixture.engine,
      "Klara should remain on the Dragon fallback with no Bench space.");
}

void test_zero_card_dark_asset_preserves_dragon_fallback() {
  sim::State state = exact_state();
  state.hand.insert(state.hand.end(),
                    {sim::Card::Grant, sim::Card::WishfulBaton,
                     sim::Card::ErikasInvitation, sim::Card::Guzma});
  Fixture fixture(sim::LockMode::None, 1969, std::move(state));
  expect_dragon_fallback(fixture.engine,
      "Klara should not recover Crobat when the projected Dark Asset draws zero.");
}

void test_absent_crobat_preserves_dragon_fallback() {
  sim::State state = exact_state();
  state.discard.erase(std::remove(state.discard.begin(), state.discard.end(),
                                  sim::Card::CrobatV),
                      state.discard.end());
  Fixture fixture(sim::LockMode::None, 1970, std::move(state));
  expect_dragon_fallback(fixture.engine,
      "Klara should remain on the Dragon fallback when Crobat V is absent.");
}
}  // namespace

int main() {
  try {
    test_live_crobat_connector_completes_the_exact_turn();
    test_used_dark_asset_preserves_dragon_fallback();
    test_rule_box_lock_preserves_dragon_fallback();
    test_full_bench_preserves_dragon_fallback();
    test_zero_card_dark_asset_preserves_dragon_fallback();
    test_absent_crobat_preserves_dragon_fallback();
    std::cout << "Issue 1965 Klara Crobat tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
'''


with LOCK.open("w", encoding="utf-8") as lock_handle:
    fcntl.flock(lock_handle.fileno(), fcntl.LOCK_EX)
    source = SOURCE.read_text(encoding="utf-8")
    source = replace_once(
        source,
        """  void klara_fill_discrete_value_targets(std::vector<Card>& pokemon) const {\n    if (pokemon.size() >= 2U) return;\n\n    // Once an advancing target makes Klara worth the Supporter action, use the second""",
        """  bool klara_crobat_connector_is_live(\n      const std::vector<Card>& pokemon,\n      const std::vector<Card>& energy) const {\n    if (pokemon.size() >= 2U || !setup_axis_missing() ||\n        klara_recoverable_copies(Card::CrobatV) <=\n            count_of(pokemon, Card::CrobatV) ||\n        state_.dark_asset_used || bench_space() <= 0 || state_.deck.empty() ||\n        !ability_available_for_pokemon(Card::CrobatV)) {\n      return false;\n    }\n\n    // Paying Klara removes one card. Recovering Crobat and then Benching it cancel\n    // each other, so direct Pokemon and Basic Energy recoveries determine the exact\n    // post-Bench hand size. Admit Crobat only when Dark Asset will draw at least one:\n    // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145\n    // Crobat V and Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104\n    // Connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#crobat-v-draw-connector-policy\n    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1965\n    const std::size_t post_bench_hand_size =\n        state_.hand.size() - 1U + pokemon.size() + energy.size();\n    return post_bench_hand_size < 6U;\n  }\n\n  void klara_fill_discrete_value_targets(\n      std::vector<Card>& pokemon, const std::vector<Card>& energy) const {\n    if (klara_crobat_connector_is_live(pokemon, energy)) {\n      // Klara has already earned the Supporter action through a direct setup target.\n      // A live Dark Asset connector advances the unresolved state, while an older\n      // Dragon without a legal discard outlet would remain stranded in hand:\n      // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145\n      // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104\n      // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1965\n      pokemon.push_back(Card::CrobatV);\n    }\n    if (pokemon.size() >= 2U) return;\n\n    // Once an advancing target makes Klara worth the Supporter action, use the second""",
    )
    source = replace_once(
        source,
        "klara_fill_discrete_value_targets(pokemon);",
        "klara_fill_discrete_value_targets(pokemon, energy);",
    )
    atomic_write(SOURCE, source)
    atomic_write(TEST, TEST_CONTENT)
    Path(__file__).unlink()
    WORKFLOW.unlink()
