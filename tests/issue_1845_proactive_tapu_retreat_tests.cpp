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
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static bool proactive_tapu_attachment(const Engine& engine) {
    return engine.issue_1845_proactive_tapu_attachment_available();
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool retreat_banked_tapu(Engine& engine) {
    return engine.retreat_banked_tapu_to_regidrago();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario() {
  return sim::Scenario{"issue-1845-proactive-tapu-retreat",
                       sim::DciProfile::StrictJit,
                       sim::LockMode::FullCombined, true, 5};
}

sim::State opening_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Serena, sim::Card::MysteriousTreasure};
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture()
      : scenario_value(scenario()),
        recipe(sim::deck_by_id("regidrago-shell")->recipe),
        rng(1845),
        engine(scenario_value, recipe, rng) {}
};

void public_surplus_attachment_is_admitted() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, opening_state());
  // Three visible Grass preserve Regidrago's two-Grass reserve while the third
  // banks Tapu's one-Energy Retreat Cost. This does not inspect future draws:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1845
  expect(sim::EngineTestAccess::proactive_tapu_attachment(fixture.engine),
         "Public surplus Tapu attachment was rejected");
  expect(sim::EngineTestAccess::attach_manual(fixture.engine),
         "Tapu attachment action did not execute");
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(state.active && state.active->grass == 1,
         "Tapu did not receive the banked Grass");
  expect(state.manual_energy_used &&
             std::count(state.hand.begin(), state.hand.end(),
                        sim::Card::Grass) == 2,
         "Tapu attachment did not preserve exactly two Grass");
}

void only_two_grass_is_rejected() {
  Fixture fixture;
  sim::State state = opening_state();
  state.hand.pop_back();
  state.hand.erase(
      std::find(state.hand.begin(), state.hand.end(), sim::Card::Grass));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::proactive_tapu_attachment(fixture.engine),
         "Tapu attachment consumed the required two-Grass reserve");
}

void stronger_regidrago_attachment_is_rejected() {
  Fixture fixture;
  sim::State state = opening_state();
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                                     sim::Tool::None});
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::proactive_tapu_attachment(fixture.engine),
         "Tapu preempted an immediate Regidrago attachment");
}

void stronger_switch_routes_are_rejected() {
  for (const int mode : {0, 1}) {
    Fixture fixture;
    sim::State state = opening_state();
    if (mode == 0) {
      state.hand.push_back(sim::Card::TateLiza);
    } else {
      state.hand.push_back(sim::Card::LatiasEx);
    }
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::proactive_tapu_attachment(fixture.engine),
           "Tapu attachment preempted a stronger switch route");
  }
}

void used_resources_and_paid_active_are_rejected() {
  for (const int mode : {0, 1, 2}) {
    Fixture fixture;
    sim::State state = opening_state();
    if (mode == 0) state.manual_energy_used = true;
    if (mode == 1) state.retreat_used = true;
    if (mode == 2) state.active->grass = 1;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::proactive_tapu_attachment(fixture.engine),
           "Tapu attachment ignored a consumed or already-paid resource");
  }
}

void banked_energy_pays_later_retreat() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 1, 0,
                              sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoVstar, 3, 2, 0,
                                     sim::Tool::None});
  state.hand = {sim::Card::RegidragoVstar};
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // The paid retreat is deliberately unavailable while the target is still a
  // Regidrago V, preventing a random Celestial Roar from replacing this route.
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1845
  expect(sim::EngineTestAccess::retreat_banked_tapu(fixture.engine),
         "Banked Tapu Energy did not pay the later retreat");
  const sim::State& result = sim::EngineTestAccess::state(fixture.engine);
  expect(result.active && result.active->card == sim::Card::RegidragoVstar,
         "Paid retreat did not promote Regidrago VSTAR");
  expect(result.retreat_used &&
             std::count(result.discard.begin(), result.discard.end(),
                        sim::Card::Grass) == 1,
         "Paid retreat did not discard the banked Grass");
}

void exact_seed_reaches_turn_five() {
  const auto selected =
      sim::scenario_by_label("strict-jit-combined-lock/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value(), "Missing combined-lock going-first scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{271828};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  expect(outcome.first_ready_turn == 5,
         "Seed 271828 did not improve to diagnostic T5 readiness");
  expect(contains("future paid Retreat"),
         "Seed 271828 did not bank the public surplus Grass");
  expect(contains("banked one-Energy Retreat Cost"),
         "Seed 271828 did not pay Tapu's stored Retreat Cost");
  expect(contains("T5 | READY"), "Seed 271828 was not ready on T5");
}

}  // namespace

int main() {
  try {
    public_surplus_attachment_is_admitted();
    only_two_grass_is_rejected();
    stronger_regidrago_attachment_is_rejected();
    stronger_switch_routes_are_rejected();
    used_resources_and_paid_active_are_rejected();
    banked_energy_pays_later_retreat();
    exact_seed_reaches_turn_five();
  } catch (const std::exception& error) {
    std::cerr << "issue-1845 proactive Tapu test failure: " << error.what()
              << '\n';
    return 1;
  }
  return 0;
}
