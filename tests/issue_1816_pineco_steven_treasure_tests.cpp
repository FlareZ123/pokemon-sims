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
  static bool public_preflight(const Engine& engine) {
    return engine.issue_1816_public_preflight();
  }
  static bool targets_available(const Engine& engine) {
    return engine.issue_1816_targets_available_after_steven();
  }
  static bool play_steven(Engine& engine) {
    return engine.play_issue_1816_direct_t3_route();
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

sim::Scenario scenario(
    const sim::LockMode locks = sim::LockMode::FullRuleBoxAbility,
    const int max_turn = 4) {
  return sim::Scenario{"issue-1816-pineco-steven-treasure",
                       sim::DciProfile::StrictJit, locks, true, max_turn};
}

sim::State base_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.hand = {
      sim::Card::StevensResolve,
      sim::Card::Fire,
      sim::Card::Dragapult,
      sim::Card::Grant,
  };
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::MysteriousTreasure,
      sim::Card::SecretBox,
      sim::Card::Dragapult,
      sim::Card::MegaDragonite,
      sim::Card::Dipplin,
      sim::Card::QuickBall,
  };
  state.prizes = {
      sim::Card::TapuLeleGX,
      sim::Card::Gladion,
      sim::Card::Dawn,
      sim::Card::ProfessorBurnet,
      sim::Card::Fire,
      sim::Card::RegidragoV,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(sim::Scenario selected_scenario = scenario(),
          const std::uint64_t seed = 1816)
      : scenario_value(std::move(selected_scenario)),
        recipe(sim::pineco_recipe()),
        rng(seed),
        engine(scenario_value, recipe, rng) {}
};

void direct_package_precedes_secret_box_filler() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state());

  // K1 proves the complete next-turn package. Steven banks Regidrago VSTAR,
  // Grass Energy, and Mysterious Treasure. Treasure then discards the held
  // Dragon payload during the ready turn while Secret Box remains preserved:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI, strict-JIT, lock, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-model https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1816
  expect(sim::EngineTestAccess::public_preflight(fixture.engine),
         "The direct T3 Treasure route was not recognized");
  expect(sim::EngineTestAccess::play_steven(fixture.engine),
         "Steven's Resolve did not play");

  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(state.turn_ended, "Steven's Resolve did not end the turn");
  expect(contains(state.hand, sim::Card::RegidragoVstar),
         "Steven did not search Regidrago VSTAR");
  expect(contains(state.hand, sim::Card::Grass),
         "Steven did not search Grass Energy");
  expect(contains(state.hand, sim::Card::MysteriousTreasure),
         "Steven did not search Mysterious Treasure");
  expect(!contains(state.hand, sim::Card::SecretBox),
         "Steven incorrectly consumed the Secret Box axis");
  expect(contains(state.deck, sim::Card::SecretBox),
         "Secret Box was not preserved in the deck");
}

void missing_payload_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.erase(std::remove_if(state.hand.begin(), state.hand.end(),
                                  sim::is_payload),
                   state.hand.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::public_preflight(fixture.engine),
         "The route invented a held strict-JIT Treasure cost");
}

void insufficient_post_draw_target_count_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::MegaDragonite),
                   state.deck.end());
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::Dipplin),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::targets_available(fixture.engine),
         "The route ignored the mandatory next-turn draw removing the last target");
}

void missing_axis_rejects_route() {
  for (const sim::Card missing : {sim::Card::RegidragoVstar, sim::Card::Grass,
                                  sim::Card::MysteriousTreasure}) {
    Fixture fixture;
    sim::State state = base_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), missing),
                     state.deck.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::targets_available(fixture.engine),
           "The route invented a missing Steven target");
  }
}

void item_lock_rejects_route() {
  for (const sim::LockMode lock : {sim::LockMode::TurnTwoItem,
                                   sim::LockMode::FullItem,
                                   sim::LockMode::FullCombined}) {
    Fixture fixture{scenario(lock)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::public_preflight(fixture.engine),
           "The route projected Mysterious Treasure through Item lock");
  }
}

void energy_state_and_horizon_are_enforced() {
  Fixture wrong_energy;
  sim::State state = base_state();
  state.active->fire = 1;
  sim::EngineTestAccess::set_state(wrong_energy.engine, std::move(state));
  expect(!sim::EngineTestAccess::public_preflight(wrong_energy.engine),
         "The route ignored the missing Fire attachment");

  Fixture expired{scenario(sim::LockMode::FullRuleBoxAbility, 2)};
  sim::EngineTestAccess::set_state(expired.engine, base_state());
  expect(!sim::EngineTestAccess::public_preflight(expired.engine),
         "The route exceeded the configured setup horizon");
}

void k0_rejects_direct_projection() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state(), false);
  expect(!sim::EngineTestAccess::targets_available(fixture.engine),
         "The projection read deck identities before a legal inspection");
}

void exact_seed_reaches_turn_three() {
  const auto selected_scenario =
      sim::scenario_by_label("strict-jit-rulebox-ability-lock/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(selected_scenario.has_value(), "Missing Rule Box Ability-lock scenario");
  expect(deck != nullptr, "Missing registered Pineco deck");

  std::mt19937_64 rng{200};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  // Source-bound exact regression: https://github.com/FlareZ123/pokemon-sims/issues/1816
  // CI reproduction: https://github.com/FlareZ123/pokemon-sims/actions/runs/30454351797
  expect(outcome.first_ready_turn == 3,
         "Seed 200 did not reach the deterministic T3 route");
  expect(trace_contains("banked Regidrago VSTAR, Grass Energy, and Mysterious Treasure"),
         "Seed 200 did not select the direct Steven package");
  expect(trace_contains("Dragapult ex (Mysterious Treasure cost)"),
         "Seed 200 did not use the held Dragon as the strict-JIT cost");
  expect(trace_contains("T3 | READY"), "Seed 200 did not become ready on T3");
  expect(!trace_contains("Secret Box discarded three other cards"),
         "Seed 200 still spent Secret Box on the dominated route");
}

}  // namespace

int main() {
  try {
    direct_package_precedes_secret_box_filler();
    missing_payload_rejects_route();
    insufficient_post_draw_target_count_rejects_route();
    missing_axis_rejects_route();
    item_lock_rejects_route();
    energy_state_and_horizon_are_enforced();
    k0_rejects_direct_projection();
    exact_seed_reaches_turn_three();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
