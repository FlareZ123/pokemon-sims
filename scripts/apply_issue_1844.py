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


source_path = Path("src/trace_engine_v2/part_013_legacy_star_override.inc")
source = source_path.read_text(encoding="utf-8")
helper_anchor = "  bool use_legacy_star() {\n"
helper = r'''  bool legacy_star_delayed_vessel_route() const {
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const bool one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool known_energy_target =
        (grass_needed() == 1 && count_of(state_.deck, Card::Grass) > 0) ||
        (fire_needed() == 1 && count_of(state_.deck, Card::Fire) > 0);

    // Legacy Star may return Earthen Vessel now for a fully public next-turn
    // finish. The current turn's manual attachment is already spent, while the
    // next turn gets a fresh attachment. A held Dragon pays Vessel and enters
    // discard during that next strict-JIT ready turn:
    // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Official Item, discard, search, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && deck_seen_ &&
        state_.turn + 1 <= scenario_.max_turn && state_.manual_energy_used &&
        active_is_vstar() && one_energy_missing && known_energy_target &&
        held_payload && count_of(state_.discard, Card::EarthenVessel) > 0;
  }

'''
if helper not in source:
    if source.count(helper_anchor) != 1:
        raise RuntimeError("Expected one Legacy Star helper insertion point")
    source = source.replace(helper_anchor, helper + helper_anchor, 1)

recovery_anchor = '''    const bool burnet_payload_line = need_payload() && supporter_allowed() && payload_might_be_in_deck();
    if (burnet_payload_line && count_of(state_.discard, Card::ProfessorBurnet) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::ProfessorBurnet)) throw std::logic_error("Legacy Star Burnet target disappeared");
      recovered.push_back(Card::ProfessorBurnet);
    }

'''
recovery = recovery_anchor + r'''    if (legacy_star_delayed_vessel_route() && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::EarthenVessel)) {
        throw std::logic_error("Legacy Star delayed Earthen Vessel target disappeared");
      }
      recovered.push_back(Card::EarthenVessel);
    }

'''
if recovery not in source:
    if source.count(recovery_anchor) != 1:
        raise RuntimeError("Expected one Legacy Star recovery insertion point")
    source = source.replace(recovery_anchor, recovery, 1)
atomic_write(source_path, source)


test = r'''#define REGIDRAGO_SIM_NO_MAIN
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
  static bool delayed_vessel_route(const Engine& engine) {
    return engine.legacy_star_delayed_vessel_route();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None,
                       const int max_turn = 5) {
  return sim::Scenario{"issue-1844-legacy-vessel-next-turn",
                       sim::DciProfile::StrictJit, lock, false, max_turn};
}

sim::State complete_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Dragapult, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::QuickBall};
  state.discard = {sim::Card::EarthenVessel, sim::Card::DialgaGX};
  state.manual_energy_used = true;
  state.vstar_power_used = true;
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode lock = sim::LockMode::None,
                   const int max_turn = 5)
      : scenario_value(scenario(lock, max_turn)),
        recipe(sim::deck_by_id("regidrago-shell")->recipe),
        rng(1844),
        engine(scenario_value, recipe, rng) {}
};

void complete_public_route_is_admitted() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, complete_state());
  // The next turn has a fresh manual attachment. Vessel discards a held Dragon,
  // searches the known Grass, and completes GGF with same-turn strict JIT:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/issues/1844
  expect(sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
         "Complete delayed Vessel route was rejected");
}

void item_lock_rejects_route() {
  Fixture fixture{sim::LockMode::FullItem};
  sim::EngineTestAccess::set_state(fixture.engine, complete_state());
  expect(!sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
         "Delayed Vessel route ignored Item lock");
}

void missing_payload_rejects_route() {
  Fixture fixture;
  sim::State state = complete_state();
  state.hand.clear();
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
         "Delayed Vessel route lacked a next-turn payload cost");
}

void multiple_energy_attachments_reject_route() {
  Fixture fixture;
  sim::State state = complete_state();
  state.active->grass = 0;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
         "Delayed Vessel route required multiple future attachments");
}

void missing_energy_target_rejects_route() {
  Fixture fixture;
  sim::State state = complete_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Grass),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
         "Delayed Vessel route had no K1-legal Energy target");
}

void inactive_vstar_rejects_route() {
  Fixture fixture;
  sim::State state = complete_state();
  state.bench.push_back(*state.active);
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
         "Delayed Vessel route ignored Active-position completion");
}

void exhausted_horizon_rejects_route() {
  Fixture fixture{sim::LockMode::None, 2};
  sim::EngineTestAccess::set_state(fixture.engine, complete_state());
  expect(!sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
         "Delayed Vessel route extended beyond the modeled horizon");
}

void exact_seed_reaches_turn_three() {
  const auto selected = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value(), "Missing strict-JIT going-second scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{314159};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  expect(outcome.first_ready_turn == 3,
         "Seed 314159 did not improve to deterministic T3 readiness");
  expect(contains("Earthen Vessel"),
         "Seed 314159 did not preserve the Vessel continuation");
  expect(contains("T3 | READY"), "Seed 314159 was not ready on T3");
}

}  // namespace

int main() {
  try {
    complete_public_route_is_admitted();
    item_lock_rejects_route();
    missing_payload_rejects_route();
    multiple_energy_attachments_reject_route();
    missing_energy_target_rejects_route();
    inactive_vstar_rejects_route();
    exhausted_horizon_rejects_route();
    exact_seed_reaches_turn_three();
  } catch (const std::exception& error) {
    std::cerr << "issue-1844 delayed Vessel test failure: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
'''
atomic_write(Path("tests/issue_1844_legacy_vessel_next_turn_tests.cpp"), test)
