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
  static bool selector_route(const Engine& engine) {
    return engine.issue_1796_wonder_tag_steven_route_available();
  }
  static Card selector(const Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
  static bool t2_route(const Engine& engine) {
    return engine.issue_1796_t2_steven_route_available();
  }
  static void choose_supporter(Engine& engine) {
    engine.choose_supporter_issue_1152();
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
  return sim::Scenario{"issue-1796-wonder-tag-steven",
                       sim::DciProfile::StrictJit, locks, true, max_turn};
}

sim::State selector_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::TapuLeleGX, 2, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::TeamYellsCheer,
      sim::Card::EarthenVessel,
      sim::Card::Gladion,
      sim::Card::MysteriousTreasure,
      sim::Card::Grass,
  };
  state.deck = {
      sim::Card::StevensResolve,
      sim::Card::RegidragoVstar,
      sim::Card::Crispin,
      sim::Card::LatiasEx,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::MegaDragonite,
      sim::Card::QuickBall,
  };
  state.prizes = {
      sim::Card::ProfessorTuro,
      sim::Card::Dragapult,
      sim::Card::RegidragoV,
      sim::Card::RegidragoV,
      sim::Card::MysteriousTreasure,
      sim::Card::PathToPeak,
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
          const std::uint64_t seed = 1796)
      : scenario_value(std::move(selected_scenario)),
        recipe(std::move(selected_recipe)),
        rng(seed),
        engine(scenario_value, recipe, rng) {}
};

void wonder_tag_selects_steven_for_complete_package() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, selector_state());

  // K1 proves the exact VSTAR, Energy, payload, Treasure, Latias, Bench, and Active
  // channels before Wonder Tag chooses its Supporter:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/issues/1796
  expect(sim::EngineTestAccess::selector_route(fixture.engine),
         "The complete Wonder Tag Steven route was not recognized");
  expect(sim::EngineTestAccess::selector(fixture.engine) ==
             sim::Card::StevensResolve,
         "Wonder Tag did not select Steven's Resolve");
}

void t2_steven_banks_exact_package() {
  Fixture fixture;
  sim::State state = selector_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::StevensResolve));
  state.hand.push_back(sim::Card::StevensResolve);
  state.manual_energy_used = true;
  state.bench.front().grass = 1;
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Grass));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(sim::EngineTestAccess::t2_route(fixture.engine),
         "The post-attachment Steven package was not recognized");
  sim::EngineTestAccess::choose_supporter(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.turn_ended, "Steven's Resolve did not end T2");
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Steven did not bank Regidrago VSTAR");
  expect(contains(after.hand, sim::Card::Crispin),
         "Steven did not bank Crispin");
  expect(std::any_of(after.hand.begin(), after.hand.end(), sim::is_payload),
         "Steven did not bank a Dragon payload");
  expect(contains(after.hand, sim::Card::Gladion),
         "The redundant Gladion route consumed the Supporter play");
}

void k0_rejects_selector_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, selector_state(), false);
  expect(!sim::EngineTestAccess::selector_route(fixture.engine),
         "The route read deck or Prize identities at K0");
}

void missing_treasure_rejects_selector_route() {
  Fixture fixture;
  sim::State state = selector_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::MysteriousTreasure));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::selector_route(fixture.engine),
         "The route invented held Mysterious Treasure");
}

void missing_payload_rejects_selector_route() {
  Fixture fixture;
  sim::State state = selector_state();
  state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(),
                                  sim::is_payload),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::selector_route(fixture.engine),
         "The route invented a Steven-searchable Dragon payload");
}

void item_lock_rejects_selector_route() {
  Fixture fixture{scenario(sim::LockMode::FullItem)};
  sim::EngineTestAccess::set_state(fixture.engine, selector_state());
  expect(!sim::EngineTestAccess::selector_route(fixture.engine),
         "The route projected Mysterious Treasure through Item lock");
}

void full_bench_rejects_selector_route() {
  Fixture fixture;
  sim::State state = selector_state();
  while (state.bench.size() < 5U) {
    state.bench.push_back(
        sim::Pokemon{sim::Card::CrobatV, 1, 0, 0, sim::Tool::None});
  }
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::selector_route(fixture.engine),
         "The route invented Latias ex Bench space");
}

void wrong_active_rejects_selector_route() {
  Fixture fixture;
  sim::State state = selector_state();
  state.active->card = sim::Card::TapuLeleGX;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::selector_route(fixture.engine),
         "The route ignored the Oricorio Skyliner Active condition");
}

void missing_energy_rejects_selector_route() {
  Fixture fixture;
  sim::State state = selector_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::Fire),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::selector_route(fixture.engine),
         "The route invented Crispin's Fire target");
}

void missing_latias_rejects_selector_route() {
  Fixture fixture;
  sim::State state = selector_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::LatiasEx),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::selector_route(fixture.engine),
         "The route invented Latias ex");
}

void expired_horizon_rejects_selector_route() {
  Fixture fixture{scenario(sim::LockMode::None, 2)};
  sim::EngineTestAccess::set_state(fixture.engine, selector_state());
  expect(!sim::EngineTestAccess::selector_route(fixture.engine),
         "The route exceeded the T3 horizon");
}

void exact_seed_reaches_turn_three() {
  const auto selected_scenario =
      sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected_scenario.has_value(), "Missing strict-JIT going-first scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{20260729};
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
         "Seed 20260729 did not reach the deterministic T3 route");
  expect(trace_contains("WONDER TAG") && trace_contains("Steven's Resolve"),
         "Seed 20260729 did not select Steven through Wonder Tag");
  expect(trace_contains("T3 | READY"),
         "Seed 20260729 did not become ready on T3");
  expect(!trace_contains("T2 | PLAY SUPPORTER") ||
             !trace_contains("Gladion exchanged"),
         "Seed 20260729 still spent Gladion on the redundant Regidrago V");
}

}  // namespace

int main() {
  try {
    wonder_tag_selects_steven_for_complete_package();
    t2_steven_banks_exact_package();
    k0_rejects_selector_route();
    missing_treasure_rejects_selector_route();
    missing_payload_rejects_selector_route();
    item_lock_rejects_selector_route();
    full_bench_rejects_selector_route();
    wrong_active_rejects_selector_route();
    missing_energy_rejects_selector_route();
    missing_latias_rejects_selector_route();
    expired_horizon_rejects_selector_route();
    exact_seed_reaches_turn_three();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
