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
            mode="w", encoding="utf-8", newline="", dir=path.parent,
            prefix=f".{path.name}.", delete=False
        ) as handle:
            handle.write(content)
            temporary = Path(handle.name)
        os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if text.count(old) != 1:
        raise RuntimeError(f"Expected exactly one marker in {path}: {old!r}")
    atomic_write(path, text.replace(old, new, 1))


source = Path("src/trace_engine_v2/part_forretress_ex_combo.inc")
replace_once(
    source,
    """  const bool dawn_refills_secret_box_costs = dawn_advances_combo &&
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
""",
    """  const Pokemon* dawn_refill_target = target_regi();
  const bool dawn_refills_secret_box_costs = dawn_advances_combo &&
      prizes_known() && !item_locked() && hand_count(Card::SecretBox) > 0 &&
      need_payload() && dawn_refill_target != nullptr &&
      dawn_refill_target->card == Card::RegidragoV &&
      dawn_refill_target->entered_turn < state_.turn &&
      hand_count(Card::RegidragoVstar) > 0 && !state_.manual_energy_used &&
      hand_count(Card::Fire) > 0 && grass_needed() > 0 && fire_needed() > 0 &&
      in_play_count(Card::Pineco) > 0 && hand_count(Card::ForretressEx) > 0 &&
      deck_count_after_search_started(Card::Grass) >= grass_needed() &&
      deck_count_after_search_started(Card::RegidragoV) > 0 &&
      deck_count_after_search_started(Card::Appletun) > 0 &&
      (deck_count_after_search_started(Card::Dragapult) > 0 ||
       deck_count_after_search_started(Card::MegaDragonite) > 0);
  // A K1 Dawn search may refill Secret Box's three independent discard slots with
  // one Basic, one Stage 1, and one Stage 2 only when the complete same-turn route
  // is already proven. A prior-turn Regidrago V, held VSTAR, held Fire, unused
  // manual attachment, prior-turn Pineco, held Forretress ex, and known deck Grass
  // preserve every board and Energy axis. Regidrago V, Appletun, and the Stage 2
  // Dragon are replaced setup resources whose dynamic DCI becomes positive:
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
                       sim::DciProfile::MatchupFlexJit, lock, false, 4};
}

sim::State refill_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::Pineco, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::Dawn,
      sim::Card::SecretBox,
      sim::Card::ForretressEx,
      sim::Card::RegidragoVstar,
      sim::Card::Fire,
      sim::Card::Dragapult,
  };
  state.deck = {
      sim::Card::RegidragoV,
      sim::Card::Appletun,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::MysteriousTreasure,
      sim::Card::Crispin,
      sim::Card::ForestOfVitality,
  };
  state.discard = {
      sim::Card::MysteriousTreasure,
      sim::Card::Grant,
      sim::Card::EarthenVessel,
      sim::Card::ProfessorTuro,
      sim::Card::Dawn,
      sim::Card::HisuianHeavyBall,
      sim::Card::Dragapult,
      sim::Card::RegidragoV,
  };
  state.prizes = {
      sim::Card::Grass,
      sim::Card::RegidragoVstar,
      sim::Card::EarthenVessel,
      sim::Card::ForestSealStone,
      sim::Card::GoodraVstar,
      sim::Card::Pineco,
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

  // K1 proves the redundant Basic, Stage 1 Dragon, Stage 2 Dragon, and final Grass.
  // The held VSTAR, Fire, Forretress ex, and prior-turn Pineco preserve every axis,
  // so all three Dawn targets become legal same-turn Secret Box fuel:
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
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

void missing_held_fire_rejects_the_refill() {
  Fixture fixture;
  sim::State state = refill_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Fire));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::advance_forretress_combo(fixture.engine),
         "Dawn did not advance the ordinary Grass line");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(!contains(after.hand, sim::Card::RegidragoV),
         "Dawn funded Secret Box before preserving the Fire axis");
  expect(!contains(after.hand, sim::Card::Appletun),
         "Dawn funded Secret Box before preserving the Fire axis");
}

void spent_attachment_rejects_the_refill() {
  Fixture fixture;
  sim::State state = refill_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::advance_forretress_combo(fixture.engine),
         "Dawn did not advance the ordinary Forretress line");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(!contains(after.hand, sim::Card::RegidragoV),
         "Dawn ignored the spent manual attachment");
  expect(!contains(after.hand, sim::Card::Appletun),
         "Dawn ignored the spent manual attachment");
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
  const auto selected_scenario =
      sim::scenario_by_label("matchup-flex-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(selected_scenario.has_value(), "Missing matchup-flex going-second scenario");
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

  // Source-bound regression for the filed public K1 state. Dawn refills the three
  // Secret Box costs, Secret Box discards a current-turn Dragon, Forretress supplies
  // the final Grass, and the held Fire plus VSTAR complete T2 without Legacy Star:
  // https://api.pokemontcg.io/v2/cards/me2-87
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/issues/1839
  expect(outcome.first_ready_turn == 2,
         "Seed 1618033 did not reach T2 readiness");
  expect(trace_contains("Dawn searched and revealed: Regidrago V, Appletun, Mega Dragonite ex"),
         "Dawn did not expose all three Secret Box cost categories");
  expect(trace_contains("PLAY ITEM") && trace_contains("Secret Box discarded three"),
         "Seed 1618033 did not retry Secret Box after Dawn");
  expect(trace_contains("T2 | READY"),
         "Seed 1618033 did not become ready on T2");
  expect(!trace_contains("LEGACY STAR"),
         "Seed 1618033 still spent Legacy Star");
}

}  // namespace

int main() {
  try {
    complete_k1_route_refills_all_three_categories();
    k0_does_not_invent_refill_targets();
    missing_secret_box_keeps_existing_dawn_selection();
    item_lock_rejects_the_secret_box_refill();
    missing_held_fire_rejects_the_refill();
    spent_attachment_rejects_the_refill();
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
