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
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1737_prized_steven_route_available();
  }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
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
    const sim::LockMode locks = sim::LockMode::None,
    const sim::OpponentBenchState opponent_bench =
        sim::OpponentBenchState::Unknown) {
  sim::Scenario value{"issue-1737-prized-steven",
                      sim::DciProfile::NoDiscardControl, locks, false, 5};
  value.opponent_bench = opponent_bench;
  return value;
}

sim::State base_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Dragapult,
                sim::Card::Grass};
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::RegidragoVstar,
      sim::Card::SecretBox,
      sim::Card::MysteriousTreasure,
      sim::Card::MysteriousTreasure,
      sim::Card::Grant,
      sim::Card::WishfulBaton,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::ProfessorBurnet,
  };
  state.prizes = {sim::Card::StevensResolve, sim::Card::Guzma,
                  sim::Card::Arven, sim::Card::ForestSealStone,
                  sim::Card::QuickBall, sim::Card::GoodraVstar};
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(sim::Scenario selected_scenario = scenario(),
          sim::DeckRecipe selected_recipe = sim::pineco_recipe(),
          const std::uint64_t seed = 1737)
      : scenario_value(std::move(selected_scenario)),
        recipe(std::move(selected_recipe)),
        rng(seed),
        engine(scenario_value, recipe, rng) {}
};

void prized_steven_outranks_inert_guzma() {
  Fixture fixture;
  sim::EngineTestAccess::set_k1_state(fixture.engine, base_state());

  // Gladion legally reveals every Prize. Steven's Resolve banks Secret Box and
  // two cost cards, then Secret Box reaches Mysterious Treasure for the held
  // Dragon discard and Regidrago VSTAR search. Guzma's switch mode is unavailable
  // while the aggregate opponent Bench is unknown:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Guzma: https://api.pokemontcg.io/v2/cards/sm3-115
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Prize, Supporter, Item, evolution, and attack procedure:
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and shortest complete route:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1737
  expect(sim::EngineTestAccess::route_available(fixture.engine),
         "The registered prized-Steven continuation was not recognized");
  expect(sim::EngineTestAccess::play_gladion(fixture.engine),
         "Gladion did not resolve the known prized-Steven route");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.hand, sim::Card::StevensResolve),
         "Steven's Resolve did not enter hand");
  expect(contains(after.prizes, sim::Card::Gladion),
         "Gladion did not replace the selected Prize");
  expect(contains(after.prizes, sim::Card::Guzma),
         "Setup-inert Guzma should remain in Prizes");
}

void absent_steven_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.prizes[0] = sim::Card::ErikasInvitation;
  sim::EngineTestAccess::set_k1_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route invented an absent prized Steven's Resolve");
}

void held_direct_vstar_connector_stays_ahead() {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.push_back(sim::Card::MysteriousTreasure);
  sim::EngineTestAccess::set_k1_state(fixture.engine, std::move(state));
  // Mysterious Treasure already reaches the unresolved VSTAR axis and must stay
  // ahead of a two-turn Steven package:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1737
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "Prized Steven displaced a held direct VSTAR connector");
}

void item_lock_rejects_future_secret_box() {
  Fixture fixture{scenario(sim::LockMode::FullItem)};
  sim::EngineTestAccess::set_k1_state(fixture.engine, base_state());
  // Secret Box and Mysterious Treasure are Item cards and cannot form the
  // completion route through modeled full Item lock:
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1737
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The projected route ignored future Item lock");
}

void missing_secret_box_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::SecretBox));
  sim::EngineTestAccess::set_k1_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route invented an absent Secret Box");
}

void completed_vstar_axis_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::None};
  sim::EngineTestAccess::set_k1_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "Prized Steven was selected after the VSTAR axis was complete");
}

void available_opponent_bench_preserves_guzma_value() {
  Fixture fixture{scenario(sim::LockMode::None,
                           sim::OpponentBenchState::Available)};
  sim::EngineTestAccess::set_k1_state(fixture.engine, base_state());
  // Guzma has discrete switch value only when the opponent Bench prerequisite is
  // explicitly available in the model:
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#guzma-opponent-bench-prerequisite
  // https://github.com/FlareZ123/pokemon-sims/issues/1737
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route erased modeled Guzma value with an available opponent Bench");
}

void shell_recipe_cannot_impersonate_pineco_route() {
  Fixture fixture{scenario(), sim::baseline_recipe()};
  sim::EngineTestAccess::set_k1_state(fixture.engine, base_state());
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The shell recipe impersonated the registered Pineco continuation");
}

void exact_seed_reaches_turn_four() {
  const auto selected_scenario =
      sim::scenario_by_label("no-discard-control/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(selected_scenario.has_value(), "Missing registered scenario");
  expect(deck != nullptr, "Missing registered Pineco deck");

  std::mt19937_64 rng{8675309};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
      return line.find(text) != std::string::npos;
    });
  };

  // Exact source-bound regression:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1737
  expect(outcome.first_ready_turn == 4,
         "Seed 8675309 did not reach the proven T4 route");
  expect(trace_contains("Exchanged Gladion for Steven's Resolve"),
         "Seed 8675309 did not take prized Steven's Resolve");
  expect(trace_contains("Steven's Resolve banked: Secret Box"),
         "Seed 8675309 did not bank the Secret Box package");
  expect(trace_contains("T4 | READY"),
         "Seed 8675309 did not become ready on T4");
  expect(!trace_contains("Completed mandatory Prize exchange with Guzma"),
         "Seed 8675309 still selected setup-inert Guzma");
}

}  // namespace

int main() {
  try {
    prized_steven_outranks_inert_guzma();
    absent_steven_rejects_route();
    held_direct_vstar_connector_stays_ahead();
    item_lock_rejects_future_secret_box();
    missing_secret_box_rejects_route();
    completed_vstar_axis_rejects_route();
    available_opponent_bench_preserves_guzma_value();
    shell_recipe_cannot_impersonate_pineco_route();
    exact_seed_reaches_turn_four();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
