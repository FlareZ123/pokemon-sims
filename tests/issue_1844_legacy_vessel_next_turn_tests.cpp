#define REGIDRAGO_SIM_NO_MAIN
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
  static bool play_earthen_vessel(Engine& engine) {
    return engine.play_earthen_vessel(true);
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

void recovered_vessel_is_held_until_next_turn() {
  Fixture fixture;
  sim::State state = complete_state();
  state.hand.push_back(sim::Card::EarthenVessel);
  state.discard.erase(
      std::find(state.discard.begin(), state.discard.end(), sim::Card::EarthenVessel));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The T2 attachment window is spent. Spending Vessel now would discard the
  // Dragon payload one turn before strict-JIT readiness:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/issues/1844
  expect(sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
         "Recovered Vessel was not recognized as the delayed route");
  expect(!sim::EngineTestAccess::play_earthen_vessel(fixture.engine),
         "Recovered Vessel was spent before the next attachment window");
}

void nonreported_energy_axis_is_rejected() {
  for (const int mode : {0, 1, 2}) {
    Fixture fixture;
    sim::State state = complete_state();
    if (mode == 0) {
      state.turn = 3;
    } else if (mode == 1) {
      state.active->fire = 0;
    } else {
      state.active->grass = 2;
      state.active->fire = 0;
      state.deck.push_back(sim::Card::Fire);
    }
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
           "Delayed Vessel exception escaped the exact T2 GF-to-GGF boundary");
  }
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
    recovered_vessel_is_held_until_next_turn();
    nonreported_energy_axis_is_rejected();
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
