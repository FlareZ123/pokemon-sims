#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }

  static bool play_issue_3107_route(Engine& engine) {
    return engine.play_issue_1898_pineco_vessel_payload_finish();
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

sim::Scenario scenario(const sim::LockMode lock) {
  return sim::Scenario{"issue-3107-pineco-vessel-lock-legality",
                       sim::DciProfile::StrictJit, lock, true, 5};
}

sim::State complete_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Dragapult, sim::Card::EarthenVessel,
                sim::Card::Guzma};
  state.deck = {sim::Card::Fire, sim::Card::Grass, sim::Card::QuickBall};
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode lock)
      : scenario_value(scenario(lock)),
        recipe(sim::deck_by_id("regidrago-pineco")->recipe),
        rng(3107),
        engine(scenario_value, recipe, rng) {}
};

void rule_box_ability_lock_keeps_the_trainer_route_legal() {
  Fixture fixture{sim::LockMode::FullRuleBoxAbility};
  sim::EngineTestAccess::set_state(fixture.engine, complete_state());

  // Rule Box Ability lock changes Pokémon Ability availability. Earthen Vessel's
  // Item discard/search and the later manual Energy attachment remain legal, so
  // the public K1 GG -> Vessel -> Fire payload finish must stay available:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Item, search, discard, and Energy-attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Scenario-lock semantics: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3107
  expect(sim::EngineTestAccess::play_issue_3107_route(fixture.engine),
         "Rule Box Ability lock rejected the legal issue-3107 Vessel route.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.discarded_this_turn, sim::Card::Dragapult),
         "The legal Rule Box lock route did not spend the current-turn payload.");
  expect(contains(after.hand, sim::Card::Fire),
         "The legal Rule Box lock route did not search the missing Fire Energy.");
}

void actual_route_legality_boundaries_still_block() {
  {
    Fixture item_lock{sim::LockMode::FullItem};
    sim::EngineTestAccess::set_state(item_lock.engine, complete_state());
    expect(!sim::EngineTestAccess::play_issue_3107_route(item_lock.engine),
           "Item lock incorrectly allowed Earthen Vessel.");
  }
  {
    Fixture k0{sim::LockMode::FullRuleBoxAbility};
    sim::EngineTestAccess::set_state(k0.engine, complete_state(), false);
    expect(!sim::EngineTestAccess::play_issue_3107_route(k0.engine),
           "K0 incorrectly spent a Dragon payload through Earthen Vessel.");
  }
  {
    Fixture attachment_spent{sim::LockMode::FullRuleBoxAbility};
    sim::State state = complete_state();
    state.manual_energy_used = true;
    sim::EngineTestAccess::set_state(attachment_spent.engine, std::move(state));
    expect(!sim::EngineTestAccess::play_issue_3107_route(attachment_spent.engine),
           "A spent manual attachment still admitted the issue-3107 route.");
  }
  {
    Fixture no_fire{sim::LockMode::FullRuleBoxAbility};
    sim::State state = complete_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::Fire), state.deck.end());
    sim::EngineTestAccess::set_state(no_fire.engine, std::move(state));
    expect(!sim::EngineTestAccess::play_issue_3107_route(no_fire.engine),
           "A missing searchable Fire target still admitted the route.");
  }
}
}  // namespace

int main() {
  try {
    rule_box_ability_lock_keeps_the_trainer_route_legal();
    actual_route_legality_boundaries_still_block();
    std::cout << "Issue 3107 Pineco Vessel lock-legality tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
