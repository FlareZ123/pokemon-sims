from __future__ import annotations

import fcntl
import os
import tempfile
from contextlib import contextmanager
from pathlib import Path


@contextmanager
def locked_path(path: Path):
    lock_path = path.with_suffix(path.suffix + ".lock")
    descriptor = os.open(lock_path, os.O_CREAT | os.O_RDWR, 0o600)
    try:
        fcntl.flock(descriptor, fcntl.LOCK_EX)
        yield
    finally:
        fcntl.flock(descriptor, fcntl.LOCK_UN)
        os.close(descriptor)
        lock_path.unlink(missing_ok=True)


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with locked_path(path):
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            newline="",
            dir=path.parent,
            prefix=f".{path.name}.",
            delete=False,
        ) as handle:
            handle.write(content)
            temporary = Path(handle.name)
        os.replace(temporary, path)


def replace_once(path_text: str, old: str, new: str) -> None:
    path = Path(path_text)
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if text.count(old) != 1:
        raise RuntimeError(f"Expected exactly one marker in {path}: {old!r}")
    atomic_write(path, text.replace(old, new, 1))


source_path = "src/trace_engine_v2/part_forretress_ex_combo.inc"

replace_once(
    source_path,
    """  const bool dawn_advances_combo = supporter_allowed() && need_energy() &&
      ability_available_for_pokemon(Card::ForretressEx) &&
      hand_count(Card::Dawn) > 0 && pineco_searchable && forretress_searchable &&
      (pineco_present || bench_space() > 0);
  if (dawn_advances_combo && remove_one(state_.hand, Card::Dawn)) {
""",
    """  const bool dawn_advances_combo = supporter_allowed() && need_energy() &&
      ability_available_for_pokemon(Card::ForretressEx) &&
      hand_count(Card::Dawn) > 0 && pineco_searchable && forretress_searchable &&
      (pineco_present || bench_space() > 0);
  const bool dawn_refills_secret_box_costs = dawn_advances_combo &&
      prizes_known() && !item_locked() && hand_count(Card::SecretBox) > 0 &&
      active_is_vstar() && need_payload() && grass_needed() > 0 &&
      fire_needed() <= 0 && in_play_count(Card::Pineco) > 0 &&
      hand_count(Card::ForretressEx) > 0 &&
      deck_count_after_search_started(Card::RegidragoV) > 0 &&
      deck_count_after_search_started(Card::Appletun) > 0 &&
      (deck_count_after_search_started(Card::Dragapult) > 0 ||
       deck_count_after_search_started(Card::MegaDragonite) > 0);
  // A K1 Dawn search may refill Secret Box's three independent discard slots with
  // one Basic, one Stage 1, and one Stage 2 only when the complete same-turn route
  // is already proven. The held Forretress ex and prior-turn Pineco preserve the
  // Grass axis, while Regidrago V, Appletun, and the Stage 2 Dragon are replaced
  // setup resources whose dynamic DCI becomes positive for this exact completion:
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1, dynamic DCI, resource preservation, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1839
  if (dawn_advances_combo && remove_one(state_.hand, Card::Dawn)) {
""",
)

replace_once(
    source_path,
    """    } else if (need_regi() && move_deck_to_hand(Card::RegidragoV)) {
      selected.push_back(Card::RegidragoV);
    }
""",
    """    } else if ((need_regi() || dawn_refills_secret_box_costs) &&
               move_deck_to_hand(Card::RegidragoV)) {
      // Dawn's Basic category becomes legal Secret Box fuel only under the complete
      // K1 refill predicate above. The redundant Regidrago V is never spent merely
      // to increase hand size:
      // https://api.pokemontcg.io/v2/cards/me2-87
      // https://api.pokemontcg.io/v2/cards/swsh12-135
      // https://api.pokemontcg.io/v2/cards/sv6-163
      // https://github.com/FlareZ123/pokemon-sims/issues/1839
      selected.push_back(Card::RegidragoV);
    }
""",
)

replace_once(
    source_path,
    """    if (in_play_count(Card::ForretressEx) == 0 &&
        hand_count(Card::ForretressEx) == 0 &&
        move_deck_to_hand(Card::ForretressEx)) {
      selected.push_back(Card::ForretressEx);
    }
""",
    """    if (in_play_count(Card::ForretressEx) == 0 &&
        hand_count(Card::ForretressEx) == 0 &&
        move_deck_to_hand(Card::ForretressEx)) {
      selected.push_back(Card::ForretressEx);
    } else if (dawn_refills_secret_box_costs &&
               move_deck_to_hand(Card::Appletun)) {
      // Held Forretress ex already satisfies Dawn's Stage 1 setup role. In the
      // proven Secret Box continuation, the deck-resident Appletun is a legal
      // Stage 1 search and a same-turn Dragon payload cost:
      // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
      // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
      // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
      // https://github.com/FlareZ123/pokemon-sims/issues/1839
      selected.push_back(Card::Appletun);
    }
""",
)


test_content = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool advance_forretress_combo(Engine& engine) {
    return engine.advance_forretress_combo();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1839-dawn-secret-box-refill",
                       sim::DciProfile::StrictJit, lock, true, 4};
}

sim::State refill_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::Pineco, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::Dawn,
      sim::Card::SecretBox,
      sim::Card::ForestSealStone,
      sim::Card::ForretressEx,
      sim::Card::RegidragoVstar,
      sim::Card::Fire,
  };
  state.deck = {
      sim::Card::RegidragoV,
      sim::Card::Appletun,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::MysteriousTreasure,
      sim::Card::Crispin,
      sim::Card::ForestOfVitality,
  };
  state.prizes = {
      sim::Card::Dragapult,
      sim::Card::RegidragoV,
      sim::Card::Dawn,
      sim::Card::Grass,
      sim::Card::QuickBall,
      sim::Card::EarthenVessel,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode lock = sim::LockMode::None)
      : scenario_value(scenario(lock)),
        recipe(sim::deck_by_id("regidrago-pineco")->recipe),
        rng(1839),
        trace{true, {}},
        engine(scenario_value, recipe, rng, &trace) {}
};

void complete_k1_route_refills_all_three_categories() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, refill_state());

  // K1 proves that Dawn's Basic, Stage 1, and Stage 2 targets all remain in deck.
  // They are route-replaced Secret Box fuel because the held Forretress ex and
  // prior-turn Pineco already preserve the Energy line, while Secret Box discards
  // a Dragon during the ready turn:
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1839
  expect(sim::EngineTestAccess::advance_forretress_combo(fixture.engine),
         "Dawn did not advance the exact Forretress route");
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(state.hand, sim::Card::RegidragoV),
         "Dawn did not refill Secret Box's Basic cost slot");
  expect(contains(state.hand, sim::Card::Appletun),
         "Dawn did not refill Secret Box's Stage 1 cost slot");
  expect(contains(state.hand, sim::Card::MegaDragonite),
         "Dawn did not refill Secret Box's Stage 2 cost slot");
}

void k0_does_not_invent_refill_targets() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, refill_state(), false);
  expect(sim::EngineTestAccess::advance_forretress_combo(fixture.engine),
         "Dawn should still resolve its ordinary public route at K0");
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(!contains(state.hand, sim::Card::RegidragoV),
         "The refill route read the deck before K1");
  expect(!contains(state.hand, sim::Card::Appletun),
         "The refill route invented the Stage 1 cost at K0");
}

void missing_secret_box_keeps_existing_dawn_selection() {
  Fixture fixture;
  sim::State state = refill_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::SecretBox));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::advance_forretress_combo(fixture.engine),
         "Dawn did not resolve without Secret Box");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(!contains(after.hand, sim::Card::RegidragoV),
         "Dawn fetched a redundant Basic without Secret Box");
  expect(!contains(after.hand, sim::Card::Appletun),
         "Dawn fetched discard fuel without Secret Box");
}

void item_lock_rejects_the_secret_box_refill() {
  Fixture fixture{sim::LockMode::FullItem};
  sim::EngineTestAccess::set_state(fixture.engine, refill_state());
  expect(sim::EngineTestAccess::advance_forretress_combo(fixture.engine),
         "Dawn should remain legal through Item lock");
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(!contains(state.hand, sim::Card::RegidragoV),
         "Dawn banked an unusable Item-locked Secret Box cost");
  expect(!contains(state.hand, sim::Card::Appletun),
         "Dawn banked an unusable Item-locked Stage 1 cost");
}

void unresolved_fire_axis_rejects_the_refill() {
  Fixture fixture;
  sim::State state = refill_state();
  state.active->fire = 0;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::advance_forretress_combo(fixture.engine),
         "Dawn did not advance the ordinary Energy line");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(!contains(after.hand, sim::Card::RegidragoV),
         "Dawn spent its Basic category before the Fire axis was solved");
  expect(!contains(after.hand, sim::Card::Appletun),
         "Dawn spent its Stage 1 category before the Fire axis was solved");
}

void absent_stage_one_cost_rejects_the_refill() {
  Fixture fixture;
  sim::State state = refill_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::Appletun));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::advance_forretress_combo(fixture.engine),
         "Dawn did not resolve when Appletun was absent");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(!contains(after.hand, sim::Card::RegidragoV),
         "Dawn partially funded a three-cost route with no Stage 1 cost");
}

void exact_seed_reaches_turn_two_without_legacy_star() {
  const auto selected_scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(selected_scenario.has_value(), "Missing strict-JIT going-first scenario");
  expect(deck != nullptr, "Missing registered Pineco deck");

  std::mt19937_64 rng{1618033};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  // Source-bound regression for the public K1 route. Dawn refills the three
  // Secret Box costs, Secret Box discards a current-turn Dragon, and Exploding
  // Energy supplies the final Grass without spending Legacy Star:
  // https://api.pokemontcg.io/v2/cards/me2-87
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/issues/1839
  expect(outcome.first_ready_turn == 2,
         "Seed 1618033 did not reach deterministic T2 readiness");
  expect(trace_contains("Dawn searched and revealed"),
         "Seed 1618033 did not resolve Dawn");
  expect(trace_contains("Regidrago V") && trace_contains("Appletun") &&
             trace_contains("Mega Dragonite ex"),
         "Dawn did not expose all three Secret Box cost categories");
  expect(trace_contains("PLAY ITEM | Secret Box"),
         "Seed 1618033 did not retry Secret Box after Dawn");
  expect(trace_contains("T2 | READY"),
         "Seed 1618033 did not become ready on T2");
  expect(!trace_contains("LEGACY STAR"),
         "Seed 1618033 spent Legacy Star despite the direct T2 route");
}

}  // namespace

int main() {
  try {
    complete_k1_route_refills_all_three_categories();
    k0_does_not_invent_refill_targets();
    missing_secret_box_keeps_existing_dawn_selection();
    item_lock_rejects_the_secret_box_refill();
    unresolved_fire_axis_rejects_the_refill();
    absent_stage_one_cost_rejects_the_refill();
    exact_seed_reaches_turn_two_without_legacy_star();
  } catch (const std::exception& error) {
    std::cerr << "issue-1839 Dawn Secret Box refill test failure: "
              << error.what() << '\n';
    return 1;
  }
  return 0;
}
'''

atomic_write(Path("tests/issue_1839_dawn_secret_box_refill_tests.cpp"), test_content)
