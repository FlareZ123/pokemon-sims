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


replace_once(
    "src/trace_engine_v2/part_issue_1118_secret_box.inc",
    """      // Exploding Energy is the route's Grass source. A hand Grass is surplus
      // once the full line, Bench access, and Ability legality are proven.
      if (card == Card::Grass) return 7;

      // With VSTAR already held, the Box's independent Item category can take
""",
    """      // Exploding Energy is the route's Grass source. A hand Grass is surplus
      // once the full line, Bench access, and Ability legality are proven.
      if (card == Card::Grass) return 7;

      // A sole Fire is route-replaced only when this same public full-combo proof
      // already has the Regidrago VSTAR Active and its Fire requirement complete.
      // The Active gate preserves Fire for retreat or promotion states; the
      // fire_needed gate preserves manual-attachment and search continuations:
      // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
      // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
      // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2-132
      // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Official Item, Supporter, discard, search, evolution, Ability, Energy, Knock Out, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Dynamic DCI and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1840
      if (card == Card::Fire && active_is_vstar() && fire_needed() <= 0) return 7;

      // With VSTAR already held, the Box's independent Item category can take
""",
)


test_content = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static std::optional<std::array<Card, 3>> cost_plan(const Engine& engine) {
    return engine.secret_box_cost_plan();
  }
  static bool play_secret_box(Engine& engine) { return engine.play_secret_box(); }
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

bool plan_contains(const std::array<sim::Card, 3>& plan,
                   const sim::Card card) {
  return std::find(plan.begin(), plan.end(), card) != plan.end();
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1840-secret-box-surplus-fire",
                       sim::DciProfile::MatchupFlexJit, lock, false, 4};
}

sim::State surplus_fire_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 3, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::Pineco, 3, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::MegaDragonite,
      sim::Card::Fire,
      sim::Card::SecretBox,
      sim::Card::StevensResolve,
  };
  state.deck = {
      sim::Card::Dawn,
      sim::Card::ForretressEx,
      sim::Card::Grass,
      sim::Card::ForestOfVitality,
      sim::Card::Dragapult,
      sim::Card::MysteriousTreasure,
      sim::Card::WishfulBaton,
      sim::Card::Grant,
      sim::Card::QuickBall,
  };
  state.prizes = {
      sim::Card::ForretressEx,
      sim::Card::ForestOfVitality,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::MysteriousTreasure,
      sim::Card::EarthenVessel,
  };
  state.discard = {
      sim::Card::Appletun,
      sim::Card::DialgaGX,
      sim::Card::Grass,
      sim::Card::MysteriousTreasure,
      sim::Card::Arven,
  };
  state.vstar_power_used = true;
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
        rng(1840),
        trace{true, {}},
        engine(scenario_value, recipe, rng, &trace) {}
};

void exact_full_combo_admits_the_surplus_fire() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, surplus_fire_state());
  const auto plan = sim::EngineTestAccess::cost_plan(fixture.engine);

  // The Active Regidrago VSTAR already has Fire. K1 proves Dawn, the remaining
  // Forretress ex, Basic Grass, prior-turn Pineco, and same-turn payload, so the
  // sole Fire has positive dynamic DCI only for this complete Secret Box route:
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2-132
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1840
  expect(plan.has_value(), "The complete K1 route did not produce three costs");
  expect(plan_contains(*plan, sim::Card::MegaDragonite),
         "The same-turn payload was not selected as a Secret Box cost");
  expect(plan_contains(*plan, sim::Card::Fire),
         "The fully surplus Fire remained incorrectly protected");
  expect(plan_contains(*plan, sim::Card::StevensResolve),
         "The route-replaced Supporter was not selected as a cost");

  expect(sim::EngineTestAccess::play_secret_box(fixture.engine),
         "Secret Box did not resolve the complete surplus-Fire route");
  const sim::State& after_box = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after_box.discard, sim::Card::Fire),
         "Secret Box did not discard the fully surplus Fire");
  expect(contains(after_box.discard, sim::Card::MegaDragonite),
         "Secret Box did not establish the same-turn Dragon payload");
  expect(contains(after_box.discard, sim::Card::StevensResolve),
         "Secret Box did not spend the replaced Steven's Resolve");
  expect(contains(after_box.hand, sim::Card::Dawn),
         "Secret Box did not search the completing Dawn Supporter");
  expect(!after_box.manual_energy_used,
         "The route consumed the unused manual attachment");
  expect(after_box.vstar_power_used,
         "The fixture's spent VSTAR Power state was corrupted");

  expect(sim::EngineTestAccess::advance_forretress_combo(fixture.engine),
         "Dawn and the prior-turn Pineco did not advance the Grass line");
  const sim::State& completed = sim::EngineTestAccess::state(fixture.engine);
  expect(completed.active.has_value() && completed.active->grass >= 2 &&
             completed.active->fire >= 1,
         "Exploding Energy did not complete GGF on the Active VSTAR");
}

void needed_fire_stays_protected() {
  Fixture fixture;
  sim::State state = surplus_fire_state();
  state.active->fire = 0;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::cost_plan(fixture.engine).has_value(),
         "Secret Box discarded the sole Fire while Fire was still needed");
}

void inactive_vstar_preserves_retreat_energy() {
  Fixture fixture;
  sim::State state = surplus_fire_state();
  state.bench.push_back(*state.active);
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::cost_plan(fixture.engine).has_value(),
         "Secret Box discarded Fire before the attacker was Active");
}

void absent_dawn_preserves_fire() {
  Fixture fixture;
  sim::State state = surplus_fire_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Dawn));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::cost_plan(fixture.engine).has_value(),
         "Secret Box spent Fire without a completing Dawn");
}

void absent_forretress_preserves_fire() {
  Fixture fixture;
  sim::State state = surplus_fire_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::ForretressEx));
  state.prizes.erase(std::remove(state.prizes.begin(), state.prizes.end(),
                                 sim::Card::ForretressEx),
                     state.prizes.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::cost_plan(fixture.engine).has_value(),
         "Secret Box spent Fire without a Forretress ex line");
}

void absent_grass_preserves_fire() {
  Fixture fixture;
  sim::State state = surplus_fire_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Grass),
                   state.deck.end());
  state.prizes.erase(std::remove(state.prizes.begin(), state.prizes.end(), sim::Card::Grass),
                     state.prizes.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::cost_plan(fixture.engine).has_value(),
         "Secret Box spent Fire without available Grass");
}

void item_lock_rejects_the_route() {
  Fixture fixture{sim::LockMode::FullItem};
  sim::EngineTestAccess::set_state(fixture.engine, surplus_fire_state());
  expect(!sim::EngineTestAccess::play_secret_box(fixture.engine),
         "Secret Box resolved through Item lock");
}

void supporter_lock_preserves_fire() {
  Fixture fixture{sim::LockMode::FullSupporter};
  sim::EngineTestAccess::set_state(fixture.engine, surplus_fire_state());
  expect(!sim::EngineTestAccess::cost_plan(fixture.engine).has_value(),
         "Secret Box spent Fire while Dawn was Supporter-locked");
}

void ability_lock_preserves_fire() {
  Fixture fixture{sim::LockMode::FullRuleBoxAbility};
  sim::EngineTestAccess::set_state(fixture.engine, surplus_fire_state());
  expect(!sim::EngineTestAccess::cost_plan(fixture.engine).has_value(),
         "Secret Box spent Fire while Exploding Energy was locked");
}

void current_turn_pineco_without_forest_preserves_fire() {
  Fixture fixture;
  sim::State state = surplus_fire_state();
  state.bench.back().entered_turn = state.turn;
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::ForestOfVitality),
                   state.deck.end());
  state.prizes.erase(std::remove(state.prizes.begin(), state.prizes.end(),
                                 sim::Card::ForestOfVitality),
                     state.prizes.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::cost_plan(fixture.engine).has_value(),
         "Secret Box spent Fire without legal evolution timing");
}

void unavailable_payload_preserves_fire() {
  Fixture fixture;
  sim::State state = surplus_fire_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::MegaDragonite));
  state.hand.push_back(sim::Card::FieldBlower);
  state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(),
                                  [](const sim::Card card) {
                                    return sim::is_payload(card);
                                  }),
                   state.deck.end());
  state.prizes.erase(std::remove_if(state.prizes.begin(), state.prizes.end(),
                                    [](const sim::Card card) {
                                      return sim::is_payload(card);
                                    }),
                     state.prizes.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::cost_plan(fixture.engine).has_value(),
         "Secret Box spent Fire without a same-turn payload");
}

void exact_seed_reaches_turn_four_without_steven() {
  const auto selected_scenario =
      sim::scenario_by_label("matchup-flex-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(selected_scenario.has_value(), "Missing matchup-flex going-second scenario");
  expect(deck != nullptr, "Missing registered Pineco deck");

  std::mt19937_64 rng{58};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  // Source-bound seed regression. The sole Fire and Steven are replaced by the
  // complete Dawn/Forretress route, while Mega Dragonite supplies current-turn JIT:
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/me2-87
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/me2-132
  // https://github.com/FlareZ123/pokemon-sims/issues/1840
  expect(outcome.first_ready_turn == 4,
         "Seed 58 did not improve from diagnostic T5 to T4");
  expect(trace_contains("Secret Box discarded three other cards"),
         "Seed 58 did not play Secret Box on T4");
  expect(trace_contains("Fire Energy (Secret Box cost)"),
         "Seed 58 did not spend the fully surplus Fire");
  expect(trace_contains("Mega Dragonite ex (Secret Box cost)"),
         "Seed 58 did not establish the current-turn payload");
  expect(trace_contains("Dawn searched and revealed"),
         "Seed 58 did not use Dawn for the Forretress line");
  expect(trace_contains("T4 | READY"),
         "Seed 58 did not reach readiness on T4");
  expect(!trace_contains("T4 | PLAY SUPPORTER | rules: R-STEVEN-01"),
         "Seed 58 still ended T4 with Steven's Resolve");
}

}  // namespace

int main() {
  try {
    exact_full_combo_admits_the_surplus_fire();
    needed_fire_stays_protected();
    inactive_vstar_preserves_retreat_energy();
    absent_dawn_preserves_fire();
    absent_forretress_preserves_fire();
    absent_grass_preserves_fire();
    item_lock_rejects_the_route();
    supporter_lock_preserves_fire();
    ability_lock_preserves_fire();
    current_turn_pineco_without_forest_preserves_fire();
    unavailable_payload_preserves_fire();
    exact_seed_reaches_turn_four_without_steven();
  } catch (const std::exception& error) {
    std::cerr << "issue-1840 surplus Fire test failure: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
'''

atomic_write(Path("tests/issue_1840_secret_box_surplus_fire_tests.cpp"), test_content)
