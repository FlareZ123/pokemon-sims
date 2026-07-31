from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
QUICK = ROOT / "src/trace_engine_v2/part_009b1.inc"
SCHEDULER = ROOT / "src/trace_engine_v2/part_014c_field_blower_override.inc"
ROUTE = ROOT / "src/trace_engine_v2/part_issue_1875_quick_ball_tapu_crispin_route.inc"
TEST = ROOT / "tests/issue_1895_quick_ball_payload_cost_tests.cpp"
WORKFLOW = ROOT / ".github/workflows/bootstrap-issue-1895.yml"
LOCK = ROOT / ".git/issue-1895-bootstrap.lock"


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    temporary = Path(name)
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
        raise RuntimeError(f"Expected one anchor, found {count}: {old[:100]!r}")
    return content.replace(old, new, 1)


FUNCTION = r'''  std::optional<Card> issue_1895_quick_ball_payload_cost() const {
    // Preserve the established one-cost Quick Ball plus prior-turn Forretress route.
    // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Confirmed route specification: https://github.com/FlareZ123/pokemon-sims/issues/1744
    const bool complete_public_route =
        strict_payload_timing() && prizes_known() && !item_locked() &&
        !issue_1744_quick_ball_forretress_route_available() &&
        supporter_allowed() && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        need_payload() && hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0 &&
        std::any_of(state_.deck.begin(), state_.deck.end(), is_basic);
    if (!complete_public_route) return std::nullopt;

    // Quick Ball may discard any other card from hand. In this public K1 GF
    // state, held Crispin attaches the missing Grass, so a held Dragon is DCI 1
    // because Quick Ball's printed cost establishes the current-turn Apex Dragon
    // payload while its legal Basic target may remain in hand:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supplied card-data corpus: https://github.com/PokemonTCG/pokemon-tcg-data/tree/master/cards/en
    // Official Item, discard-cost, search, Supporter, Energy-attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // K1, dynamic DCI/JIT, Supporter contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1895
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

'''

ROUTE_HELPERS = r'''
  bool issue_1895_held_crispin_quick_ball_route_available() const {
    return issue_1895_quick_ball_payload_cost().has_value();
  }

  bool complete_issue_1895_held_crispin_quick_ball_route() {
    if (!issue_1895_held_crispin_quick_ball_route_available()) return false;

    // Start the one-discard Item before any lower-value Supporter action. Quick
    // Ball's Dragon cost supplies the current-turn payload, and the already-held
    // Crispin attaches the final Grass to the Active GF Regidrago VSTAR:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard-cost, search, Supporter, and Energy-attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Supporter contention and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1895
    if (!play_quick_ball(false)) {
      throw std::logic_error("Issue-1895 Quick Ball route disappeared");
    }
    if (!play_crispin()) {
      throw std::logic_error("Issue-1895 Crispin finish disappeared");
    }
    return active_is_vstar() && !need_energy() && !need_payload();
  }

'''

TEST_CONTENT = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static std::optional<Card> payload_cost(const Engine& engine) {
    return engine.issue_1895_quick_ball_payload_cost();
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1895_held_crispin_quick_ball_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1895_held_crispin_quick_ball_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario strict(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1895", sim::DciProfile::StrictJit, lock, true, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::Crispin,
                sim::Card::Crispin, sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoV};
  return state;
}

sim::Engine make_engine(const sim::Scenario& selected, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true,
                        const bool prizes_revealed = false) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(selected, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, std::move(state), deck_seen, prizes_revealed);
  return engine;
}

void test_exact_route_and_public_k1_boundaries() {
  std::mt19937_64 rng(1895);
  const sim::Scenario strict_scenario = strict();
  const sim::Scenario item_locked_scenario = strict(sim::LockMode::FullItem);
  sim::Engine engine = make_engine(strict_scenario, rng, route_state());

  // Quick Ball's Dragon cost is the current-turn Apex Dragon payload, while
  // held Crispin attaches the missing Grass from a two-type public-K1 deck.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1895
  expect(sim::EngineTestAccess::payload_cost(engine) == sim::Card::Dragapult,
         "Deck-search K1 did not select the held Dragon payload.");
  expect(sim::EngineTestAccess::route_available(engine),
         "The complete held-Crispin Quick Ball route was unavailable.");
  expect(sim::EngineTestAccess::complete_route(engine),
         "The complete held-Crispin Quick Ball route failed.");

  sim::Engine prize_k1 =
      make_engine(strict_scenario, rng, route_state(), false, true);
  expect(sim::EngineTestAccess::payload_cost(prize_k1) == sim::Card::Dragapult,
         "Prize-inspection K1 did not admit the Quick Ball payload cost.");

  sim::Engine k0 = make_engine(strict_scenario, rng, route_state(), false, false);
  expect(!sim::EngineTestAccess::payload_cost(k0).has_value(),
         "True K0 admitted the public-K1-only payload cost.");

  sim::Engine item_locked =
      make_engine(item_locked_scenario, rng, route_state());
  expect(!sim::EngineTestAccess::payload_cost(item_locked).has_value(),
         "Item lock admitted the Quick Ball route.");

  sim::State supporter_spent = route_state();
  supporter_spent.supporter_used = true;
  sim::Engine spent = make_engine(strict_scenario, rng, std::move(supporter_spent));
  expect(!sim::EngineTestAccess::payload_cost(spent).has_value(),
         "A spent Supporter action admitted the Crispin route.");

  sim::State one_type = route_state();
  one_type.deck.erase(std::find(one_type.deck.begin(), one_type.deck.end(),
                                sim::Card::Fire));
  sim::Engine missing_type = make_engine(strict_scenario, rng, std::move(one_type));
  expect(!sim::EngineTestAccess::payload_cost(missing_type).has_value(),
         "One searchable Energy type admitted the two-type Crispin finish.");

  sim::State no_target = route_state();
  no_target.deck = {sim::Card::Grass, sim::Card::Fire};
  sim::Engine targetless = make_engine(strict_scenario, rng, std::move(no_target));
  expect(!sim::EngineTestAccess::payload_cost(targetless).has_value(),
         "Quick Ball without a legal Basic target admitted the cost.");

  sim::State payload_done = route_state();
  payload_done.discard = {sim::Card::Dragapult};
  payload_done.discarded_this_turn = {sim::Card::Dragapult};
  sim::Engine done = make_engine(strict_scenario, rng, std::move(payload_done));
  expect(!sim::EngineTestAccess::payload_cost(done).has_value(),
         "An already-satisfied current-turn payload spent another Dragon.");
}

void test_registered_witnesses_reach_t3() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto strict_scenario = sim::scenario_by_label("strict-jit/go-first");
  const auto flex_scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  expect(deck != nullptr && strict_scenario.has_value() && flex_scenario.has_value(),
         "The registered issue-1895 fixtures are unavailable.");

  std::mt19937_64 strict_rng(951);
  sim::Engine strict_engine(*strict_scenario, deck->recipe, strict_rng);
  expect(strict_engine.run().first_ready_turn == 3,
         "Strict-JIT seed 951 did not reach readiness on T3.");

  std::mt19937_64 flex_rng(489);
  sim::Engine flex_engine(*flex_scenario, deck->recipe, flex_rng);
  expect(flex_engine.run().first_ready_turn == 3,
         "Matchup-flex seed 489 did not reach readiness on T3.");
}
}  // namespace

int main() {
  try {
    test_exact_route_and_public_k1_boundaries();
    test_registered_witnesses_reach_t3();
    std::cout << "Issue 1895 Quick Ball payload-cost tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
'''

with LOCK.open("w", encoding="utf-8") as lock_handle:
    fcntl.flock(lock_handle.fileno(), fcntl.LOCK_EX)

    quick = QUICK.read_text(encoding="utf-8")
    quick = replace_once(
        quick,
        "  bool play_quick_ball(const bool permit_payload) {",
        FUNCTION + "  bool play_quick_ball(const bool permit_payload) {",
    )
    quick = replace_once(
        quick,
        """    const bool has_preferred_search_target =
        want_regi || want_latias || want_tapu || want_oricorio || want_crobat || want_payload;
    const bool can_use_payload_only_outlet =
        can_pay_payload_cost && blender_energy_axis_can_finish_this_turn();""",
        """    const bool has_preferred_search_target =
        want_regi || want_latias || want_tapu || want_oricorio || want_crobat || want_payload;
    const std::optional<Card> issue_1895_payload_cost =
        issue_1895_quick_ball_payload_cost();
    const bool can_use_payload_only_outlet =
        (can_pay_payload_cost && blender_energy_axis_can_finish_this_turn()) ||
        issue_1895_payload_cost.has_value();""",
    )
    quick = replace_once(
        quick,
        """    std::optional<Card> cost;
    if (issue_1797_tapu_route) {""",
        """    std::optional<Card> cost = issue_1895_payload_cost;
    if (!cost && issue_1797_tapu_route) {""",
    )
    atomic_write(QUICK, quick)

    route = ROUTE.read_text(encoding="utf-8")
    route = replace_once(
        route,
        "  bool issue_1875_quick_ball_tapu_crispin_route_available() const {",
        ROUTE_HELPERS + "  bool issue_1875_quick_ball_tapu_crispin_route_available() const {",
    )
    atomic_write(ROUTE, route)

    scheduler = SCHEDULER.read_text(encoding="utf-8")
    scheduler = replace_once(
        scheduler,
        """  bool secret_box_combo_enabled_issue1875() const {
    return issue_1874_duplicate_treasure_payload_route_available() ||""",
        """  bool secret_box_combo_enabled_issue1875() const {
    return issue_1895_held_crispin_quick_ball_route_available() ||
        issue_1874_duplicate_treasure_payload_route_available() ||""",
    )
    scheduler = replace_once(
        scheduler,
        """  void run_secret_box_turn_issue1875() {
    // Preserve the duplicate Treasures""",
        """  void run_secret_box_turn_issue1875() {
    if (issue_1895_held_crispin_quick_ball_route_available()) {
      trace("POLICY", "P-AXIS-01", "Start: " + state_line());
      if (!complete_issue_1895_held_crispin_quick_ball_route()) {
        throw std::logic_error("Issue-1895 projected route failed");
      }
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    // Preserve the duplicate Treasures""",
    )
    atomic_write(SCHEDULER, scheduler)

    atomic_write(TEST, TEST_CONTENT)
    Path(__file__).unlink()
    WORKFLOW.unlink()
