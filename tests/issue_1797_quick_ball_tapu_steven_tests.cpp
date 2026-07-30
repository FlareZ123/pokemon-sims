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
  static bool quick_ball_route(const Engine& engine) {
    return engine.issue_1797_quick_ball_tapu_steven_route_available();
  }
  static std::optional<Card> quick_ball_cost(const Engine& engine) {
    return engine.issue_1797_quick_ball_cost();
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static bool wonder_tag_route(const Engine& engine) {
    return engine.issue_1797_wonder_tag_steven_route_available();
  }
  static Card supporter_target(const Engine& engine) {
    return engine.choose_supporter_after_search_started();
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
    const sim::LockMode locks = sim::LockMode::None,
    const int max_turn = 5) {
  return sim::Scenario{"issue-1797-quick-ball-tapu-steven",
                       sim::DciProfile::StrictJit, locks, true, max_turn};
}

sim::State t1_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {
      sim::Card::QuickBall,
      sim::Card::TateLiza,
      sim::Card::Lusamine,
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::Grass,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::StevensResolve,
      sim::Card::Crispin,
      sim::Card::EarthenVessel,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::QuickBall,
  };
  state.discard = {sim::Card::HisuianHeavyBall};
  state.prizes = {
      sim::Card::ProfessorTuro,
      sim::Card::Dragapult,
      sim::Card::MysteriousTreasure,
      sim::Card::PathToPeak,
      sim::Card::Guzma,
      sim::Card::Oricorio,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(sim::Scenario selected_scenario = scenario(),
          sim::DeckRecipe selected_recipe = sim::baseline_recipe(),
          const std::uint64_t seed = 1797)
      : scenario_value(std::move(selected_scenario)),
        recipe(std::move(selected_recipe)),
        rng(seed),
        engine(scenario_value, recipe, rng) {}
};

void quick_ball_selects_tapu_and_low_dci_cost() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, t1_state());

  // K1 proves the entire T1-T3 package before Quick Ball pays its cost:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://github.com/FlareZ123/pokemon-sims/issues/1797
  expect(sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The complete K1 Quick Ball route was not recognized");
  expect(sim::EngineTestAccess::quick_ball_cost(fixture.engine) ==
             sim::Card::TateLiza,
         "Quick Ball did not prefer Tate & Liza as the replaced Supporter cost");
  expect(sim::EngineTestAccess::play_quick_ball(fixture.engine),
         "Quick Ball did not resolve");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.hand, sim::Card::TapuLeleGX),
         "Quick Ball did not search Tapu Lele-GX");
  expect(contains(after.discard, sim::Card::TateLiza),
         "Quick Ball did not discard Tate & Liza");
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Quick Ball discarded the held VSTAR");
  expect(std::count(after.hand.begin(), after.hand.end(), sim::Card::Grass) == 2,
         "Quick Ball spent a required Grass attachment");
}

void lusamine_is_legal_fallback_cost() {
  Fixture fixture;
  sim::State state = t1_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::TateLiza));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::quick_ball_cost(fixture.engine) ==
             sim::Card::Lusamine,
         "Quick Ball did not use Lusamine after Tate & Liza was absent");
}

void wonder_tag_banks_steven_going_first() {
  Fixture fixture;
  sim::State state = t1_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::QuickBall));
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::TateLiza));
  state.discard.push_back(sim::Card::QuickBall);
  state.discard.push_back(sim::Card::TateLiza);
  state.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                                     sim::Tool::None});
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::TapuLeleGX));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(sim::EngineTestAccess::wonder_tag_route(fixture.engine),
         "Wonder Tag did not recognize the complete going-first route");
  expect(sim::EngineTestAccess::supporter_target(fixture.engine) ==
             sim::Card::StevensResolve,
         "Wonder Tag did not bank Steven's Resolve on T1");
}

void k0_rejects_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, t1_state(), false);
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route read deck or Prize identities at K0");
}

void missing_discard_cost_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::TateLiza), state.hand.end());
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::Lusamine), state.hand.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented a Quick Ball discard cost");
}

void missing_tapu_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::TapuLeleGX), state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented Tapu Lele-GX");
}

void missing_vstar_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::RegidragoVstar), state.hand.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented the held VSTAR");
}

void missing_second_grass_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Grass));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented the second Grass attachment");
}

void item_lock_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::FullItem)};
  sim::EngineTestAccess::set_state(fixture.engine, t1_state());
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route projected Quick Ball through Item lock");
}

void ability_lock_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::FullRuleBoxAbility)};
  sim::EngineTestAccess::set_state(fixture.engine, t1_state());
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route projected Wonder Tag through Ability lock");
}

void full_bench_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  while (state.bench.size() < 5U) {
    state.bench.push_back(
        sim::Pokemon{sim::Card::CrobatV, 1, 0, 0, sim::Tool::None});
  }
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented Tapu Bench space");
}

void missing_package_piece_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::EarthenVessel), state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented Earthen Vessel");
}

void missing_payload_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(),
                                  sim::is_payload), state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented a permitted Dragon payload");
}

void insufficient_energy_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::Grass));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route ignored future draw and Vessel Energy contention");
}

void expired_horizon_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::None, 2)};
  sim::EngineTestAccess::set_state(fixture.engine, t1_state());
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route exceeded the T3 setup horizon");
}

void exact_seed_reaches_turn_three() {
  const auto selected_scenario =
      sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected_scenario.has_value(), "Missing strict-JIT going-first scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{854};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  expect(outcome.first_ready_turn == 3,
         "Seed 854 did not reach the deterministic T3 route");
  expect(trace_contains("Quick Ball") && trace_contains("Tapu Lele-GX"),
         "Seed 854 did not use Quick Ball for Tapu Lele-GX");
  expect(trace_contains("WONDER TAG") && trace_contains("Steven's Resolve"),
         "Seed 854 did not bank Steven's Resolve through Wonder Tag");
  expect(trace_contains("T3 | READY"),
         "Seed 854 did not become ready on T3");
  expect(!trace_contains("Celestial Roar"),
         "Seed 854 still depended on random Celestial Roar");
}

}  // namespace

int main() {
  try {
    quick_ball_selects_tapu_and_low_dci_cost();
    lusamine_is_legal_fallback_cost();
    wonder_tag_banks_steven_going_first();
    k0_rejects_route();
    missing_discard_cost_rejects_route();
    missing_tapu_rejects_route();
    missing_vstar_rejects_route();
    missing_second_grass_rejects_route();
    item_lock_rejects_route();
    ability_lock_rejects_route();
    full_bench_rejects_route();
    missing_package_piece_rejects_route();
    missing_payload_rejects_route();
    insufficient_energy_rejects_route();
    expired_horizon_rejects_route();
    exact_seed_reaches_turn_three();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
