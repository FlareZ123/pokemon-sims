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
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static bool pre_burnet_route(const Engine& engine) {
    return engine.issue_1221_burnet_then_treasure_route_available();
  }
  static std::optional<Card> post_burnet_cost(const Engine& engine) {
    return engine.issue_1221_burnet_treasure_vstar_cost();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&needle](const std::string& line) {
    return line.find(needle) != std::string::npos;
  });
}

sim::Scenario strict_scenario(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-1221", sim::DciProfile::StrictJit, locks, true, 5};
}

sim::State pre_burnet_state() {
  sim::State state;
  state.turn = 5;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::MysteriousTreasure,
                sim::Card::Channeler, sim::Card::ErikasInvitation,
                sim::Card::RoseannesBackup, sim::Card::Powerglass,
                sim::Card::Fire, sim::Card::ChaoticSwell};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::Grass,
                sim::Card::RegidragoV};
  return state;
}

sim::State post_burnet_state() {
  sim::State state = pre_burnet_state();
  state.supporter_used = true;
  state.hand.erase(state.hand.begin());
  state.discard = {sim::Card::ProfessorBurnet, sim::Card::MegaDragonite,
                   sim::Card::Dragapult};
  state.discarded_this_turn = {sim::Card::MegaDragonite,
                               sim::Card::Dragapult};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::RegidragoV};
  return state;
}

sim::Engine make_engine(const sim::Scenario& chosen, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(chosen, recipe, rng, nullptr);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_route_preflight_and_cost() {
  const sim::Scenario chosen = strict_scenario();
  std::mt19937_64 rng{122101};
  sim::Engine before = make_engine(chosen, rng, pre_burnet_state());

  // Burnet supplies the current-turn Dragon payload. Treasure may then spend one
  // Supporter whose setup role ended when Burnet consumed the Supporter action.
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1221
  expect(sim::EngineTestAccess::pre_burnet_route(before),
         "The complete Burnet-Treasure route must be live before Burnet.");

  sim::Engine after = make_engine(chosen, rng, post_burnet_state());
  const auto cost = sim::EngineTestAccess::post_burnet_cost(after);
  expect(cost && *cost == sim::Card::Channeler,
         "Channeler must be the first narrow route-conditioned Treasure cost.");
}

void test_protected_resources_and_blockers() {
  const auto unavailable = [](sim::State state, const sim::Scenario chosen,
                              const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(chosen, rng, std::move(state));
    expect(!sim::EngineTestAccess::post_burnet_cost(engine).has_value(), message);
  };

  sim::State no_payload = post_burnet_state();
  no_payload.discarded_this_turn.clear();
  unavailable(no_payload, strict_scenario(), 122102,
              "No current-turn Dragon payload must block the fallback.");

  sim::State same_turn_regi = post_burnet_state();
  same_turn_regi.active->entered_turn = 5;
  unavailable(same_turn_regi, strict_scenario(), 122103,
              "A same-turn Regidrago V cannot evolve.");

  sim::State missing_energy = post_burnet_state();
  missing_energy.active->fire = 0;
  unavailable(missing_energy, strict_scenario(), 122104,
              "An incomplete Energy axis must block the fallback.");

  sim::State no_vstar = post_burnet_state();
  no_vstar.deck.erase(no_vstar.deck.begin());
  unavailable(no_vstar, strict_scenario(), 122105,
              "Known missing VSTAR must block the fallback.");

  unavailable(post_burnet_state(), strict_scenario(sim::LockMode::FullItem), 122106,
              "Item lock must block Mysterious Treasure.");

  sim::State protected_only = post_burnet_state();
  protected_only.hand = {sim::Card::MysteriousTreasure,
                         sim::Card::ProfessorTuro,
                         sim::Card::RoseannesBackup,
                         sim::Card::Powerglass,
                         sim::Card::Fire};
  unavailable(protected_only, strict_scenario(), 122107,
              "Movement, recovery, Tool, and Energy resources must remain protected.");
}

void test_seed_123_regression() {
  const sim::Scenario chosen{"strict-jit/go-first", sim::DciProfile::StrictJit,
                             sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{123};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(chosen, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 5,
         "Seed 123 must complete the legal route on T5.");
  expect(trace_contains(trace, "T5 | PLAY SUPPORTER | rules: R-BURNET-01"),
         "Seed 123 must play Professor Burnet.");
  expect(trace_contains(trace, "T5 | DISCARD | rules: R-MT-01 | Channeler"),
         "Seed 123 must spend Channeler as the Treasure cost.");
  expect(trace_contains(trace, "T5 | PLAY ITEM | rules: R-MT-01"),
         "Seed 123 must search Regidrago VSTAR with Treasure.");
  expect(trace_contains(trace, "T5 | READY |"),
         "Seed 123 must record readiness after evolution.");
}
}  // namespace

int main() {
  try {
    test_route_preflight_and_cost();
    test_protected_resources_and_blockers();
    test_seed_123_regression();
    std::cout << "Issue 1221 Burnet-Treasure VSTAR tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
