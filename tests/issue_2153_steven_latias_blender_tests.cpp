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
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route(const Engine& engine) {
    return engine.issue_2153_steven_latias_blender_route_available();
  }
  static bool play(Engine& engine) {
    return engine.play_issue_2153_steven_latias_blender_route();
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

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::Scenario scenario(const sim::LockMode locks = sim::LockMode::None,
                       const int max_turn = 5) {
  return sim::Scenario{"issue-2153-steven-latias-blender",
                       sim::DciProfile::StrictJit,
                       locks,
                       false,
                       max_turn};
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::DialgaGX,
      sim::Card::Dragapult,
      sim::Card::StevensResolve,
      sim::Card::EarthenVessel,
      sim::Card::Crispin,
      sim::Card::ForestSealStone,
  };
  state.deck = {
      sim::Card::RegidragoV,
      sim::Card::LatiasEx,
      sim::Card::BrilliantBlender,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::MegaDragonite,
      sim::Card::MysteriousTreasure,
      sim::Card::QuickBall,
  };
  state.prizes = {
      sim::Card::GoodraVstar,
      sim::Card::Channeler,
      sim::Card::TeamYellsCheer,
      sim::Card::QuickBall,
      sim::Card::Powerglass,
      sim::Card::FieldBlower,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(sim::Scenario selected = scenario())
      : scenario_value(std::move(selected)),
        recipe(sim::baseline_recipe()),
        rng(2153),
        engine(scenario_value, recipe, rng) {}
};

void exact_k1_route_searches_three_missing_connectors() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state());
  expect(sim::EngineTestAccess::route(fixture.engine),
         "The exact issue-2153 K1 route was not recognized");
  expect(sim::EngineTestAccess::play(fixture.engine),
         "The issue-2153 Steven route did not execute");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::Oricorio,
         "Steven incorrectly changed the Active Pokemon");
  expect(after.supporter_used && after.turn_ended,
         "Steven did not consume the Supporter action and end the turn");
  expect(contains(after.hand, sim::Card::RegidragoV) &&
             contains(after.hand, sim::Card::LatiasEx) &&
             contains(after.hand, sim::Card::BrilliantBlender),
         "Steven did not search Regidrago V, Latias ex, and Brilliant Blender");
  expect(count(after.hand, sim::Card::RegidragoVstar) == 1,
         "Steven duplicated the already held Regidrago VSTAR");
  expect(contains(after.discard, sim::Card::StevensResolve),
         "Played Steven's Resolve was not placed in discard");

  // Steven searches any three cards; Skyliner supplies the free-retreat axis;
  // Brilliant Blender supplies the strict-JIT payload on the ready turn:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void k0_rejects_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state(), false, false);
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route read deck or Prize identities at K0");

  // K1 begins only after a legal deck or Prize inspection:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void prized_latias_rejects_route() {
  Fixture fixture;
  sim::State state = route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::LatiasEx),
                   state.deck.end());
  state.prizes[0] = sim::Card::LatiasEx;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route invented a prized Latias ex");

  // Skyliner cannot be selected when K1 proves Latias ex is in the Prizes:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void rule_box_ability_lock_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::FullRuleBoxAbility)};
  sim::EngineTestAccess::set_state(fixture.engine, route_state());
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route used Skyliner through Rule Box Ability lock");

  // Latias ex is a Rule Box Pokemon, so the modeled lock suppresses Skyliner:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void full_bench_rejects_route() {
  Fixture fixture;
  sim::State state = route_state();
  state.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0,
                                     sim::Tool::None});
  state.bench.push_back(sim::Pokemon{sim::Card::CrobatV, 1, 0, 0,
                                     sim::Tool::None});
  state.bench.push_back(sim::Pokemon{sim::Card::MawileGX, 1, 0, 0,
                                     sim::Tool::None});
  state.bench.push_back(sim::Pokemon{sim::Card::Pineco, 1, 0, 0,
                                     sim::Tool::None});
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route exceeded the five-Pokemon Bench limit");

  // Regidrago V and Latias ex both need legal Bench slots:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void insufficient_intervening_draw_buffer_rejects_route() {
  Fixture fixture;
  sim::State state = route_state();
  const auto fire = std::find(state.deck.begin(), state.deck.end(),
                              sim::Card::Fire);
  expect(fire != state.deck.end(), "Test state had no Fire Energy to remove");
  state.deck.erase(fire);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route ignored the T2 draw consuming a required Fire target");

  // Vessel and Crispin each search the deck. Three Fire copies at K1 preserve
  // both searches even when the intervening turn draw is Fire Energy:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void short_horizon_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::None, 2)};
  sim::EngineTestAccess::set_state(fixture.engine, route_state());
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route exceeded the T3 setup horizon");

  // T2 through T4 are setup-success turns; this exact package first completes T3:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#ready-state-and-t5-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void exact_seed_408_reaches_turn_three() {
  const auto selected = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value(), "Missing strict-JIT going-second scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{408};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  expect(outcome.first_ready_turn == 3,
         "Seed 408 did not reach the deterministic T3 route");
  expect(trace_contains("Regidrago V, Latias ex, Brilliant Blender"),
         "Seed 408 did not use the corrected Steven target package");
  expect(trace_contains("T3 | READY"),
         "Seed 408 did not reach T3 readiness");

  // Exact full-game regression and governing route sources:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

}  // namespace

int main() {
  try {
    exact_k1_route_searches_three_missing_connectors();
    k0_rejects_route();
    prized_latias_rejects_route();
    rule_box_ability_lock_rejects_route();
    full_bench_rejects_route();
    insufficient_intervening_draw_buffer_rejects_route();
    short_horizon_rejects_route();
    exact_seed_408_reaches_turn_three();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
