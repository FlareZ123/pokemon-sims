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
  static void set_state(Engine& engine, State state, const bool k1 = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool public_candidate(const Engine& engine) {
    return engine.issue_1811_public_arven_route_candidate();
  }
  static bool inspected_complete(const Engine& engine) {
    return engine.issue_1811_inspected_arven_route_complete();
  }
  static bool play_arven(Engine& engine) {
    return engine.play_issue_1811_arven_route();
  }
  static bool play_treasure(Engine& engine) {
    return engine.play_issue_1811_mysterious_treasure_route();
  }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
  static bool use_fss(Engine& engine) { return engine.use_issue_1811_fss_route(); }
  static bool hold_roar(Engine& engine) { return engine.hold_issue_1811_celestial_roar(); }
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

sim::Scenario scenario(const sim::LockMode locks = sim::LockMode::None,
                       const bool going_first = false,
                       const int max_turn = 5) {
  return sim::Scenario{"issue-1811-arven-treasure-crispin",
                       sim::DciProfile::StrictJit, locks, going_first,
                       max_turn};
}

sim::State base_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.manual_energy_used = true;
  state.hand = {
      sim::Card::Arven,
      sim::Card::Fire,
      sim::Card::EarthenVessel,
      sim::Card::Dragapult,
      sim::Card::TeamYellsCheer,
      sim::Card::ChaoticSwell,
  };
  state.deck = {
      sim::Card::MysteriousTreasure,
      sim::Card::ForestSealStone,
      sim::Card::RegidragoVstar,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::RegidragoV,
      sim::Card::MegaDragonite,
  };
  state.prizes = {
      sim::Card::Gladion,
      sim::Card::GoodraVstar,
      sim::Card::Channeler,
      sim::Card::QuickBall,
      sim::Card::ProfessorBurnet,
      sim::Card::TateLiza,
  };
  return state;
}

struct Fixture {
  sim::Scenario selected_scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(sim::Scenario value = scenario())
      : selected_scenario(std::move(value)),
        recipe(sim::baseline_recipe()),
        rng(1811),
        engine(selected_scenario, recipe, rng) {}
};

void public_preflight_and_k1_gate() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state());
  expect(sim::EngineTestAccess::public_candidate(fixture.engine),
         "The public T1 Arven route was not recognized");
  expect(!sim::EngineTestAccess::inspected_complete(fixture.engine),
         "The route read physical deck contents before legal inspection");

  sim::EngineTestAccess::set_state(fixture.engine, base_state(), true);
  expect(sim::EngineTestAccess::inspected_complete(fixture.engine),
         "The K1 route components were not recognized");
}

void complete_t1_sequence_is_deterministic() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state());

  // Arven establishes K1, Treasure spends route-replaced Fire for Regidrago
  // VSTAR, Forest Seal Stone banks Crispin, and Celestial Roar is held because
  // Earthen Vessel plus Dragapult ex preserve strict-JIT on T2:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1811
  expect(sim::EngineTestAccess::play_arven(fixture.engine),
         "Arven did not select Treasure and Forest Seal Stone");
  expect(sim::EngineTestAccess::play_treasure(fixture.engine),
         "Mysterious Treasure did not use the route-replaced Fire");
  expect(sim::EngineTestAccess::attach_fss(fixture.engine),
         "Forest Seal Stone did not attach to Regidrago V");
  expect(sim::EngineTestAccess::use_fss(fixture.engine),
         "Star Alchemy did not bank Crispin");
  expect(sim::EngineTestAccess::hold_roar(fixture.engine),
         "The complete T2 route did not hold Celestial Roar");

  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(state.hand, sim::Card::RegidragoVstar),
         "Regidrago VSTAR was not held for T2 evolution");
  expect(contains(state.hand, sim::Card::Crispin),
         "Crispin was not held for T2 Energy acceleration");
  expect(contains(state.hand, sim::Card::EarthenVessel),
         "Earthen Vessel was not preserved for strict-JIT");
  expect(contains(state.hand, sim::Card::Dragapult),
         "The T2 Vessel payload was discarded too early");
  expect(contains(state.discard, sim::Card::Fire),
         "The route-replaced Fire did not pay Treasure");
}

void missing_post_crispin_vessel_target_rejects() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Grass));
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Fire));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true);
  expect(!sim::EngineTestAccess::inspected_complete(fixture.engine),
         "The route invented an Earthen Vessel target after Crispin");
}

void missing_route_piece_rejects() {
  for (const sim::Card missing : {sim::Card::MysteriousTreasure,
                                  sim::Card::ForestSealStone,
                                  sim::Card::RegidragoVstar,
                                  sim::Card::Crispin}) {
    Fixture fixture;
    sim::State state = base_state();
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(), missing));
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true);
    expect(!sim::EngineTestAccess::inspected_complete(fixture.engine),
           "The route invented a missing searched card");
  }
}

void public_controls_reject() {
  {
    Fixture fixture{scenario(sim::LockMode::FullItem)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::public_candidate(fixture.engine),
           "The route ignored Item lock");
  }
  {
    Fixture fixture{scenario(sim::LockMode::FullRuleBoxAbility)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::public_candidate(fixture.engine),
           "The route ignored Rule Box Ability lock");
  }
  {
    Fixture fixture{scenario(sim::LockMode::None, true)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::public_candidate(fixture.engine),
           "The route ignored the going-first T1 Supporter restriction");
  }
  {
    Fixture fixture{scenario(sim::LockMode::None, false, 1)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::public_candidate(fixture.engine),
           "The route exceeded the configured T2 horizon");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Dragapult));
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::public_candidate(fixture.engine),
           "The route invented a strict-JIT Vessel payload");
  }
}

void exact_seed_reaches_turn_two() {
  const auto selected_scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected_scenario.has_value(), "Missing strict-JIT going-second scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{424242};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  expect(outcome.first_ready_turn == 2 && outcome.ready_by_2 && !outcome.setup_failed,
         "Seed 424242 did not reach strict-JIT readiness on T2");
  expect(trace_contains("Searched Mysterious Treasure and Forest Seal Stone"),
         "The exact seed did not take the issue-1811 Arven route");
  expect(trace_contains("Discarded route-replaced Fire Energy"),
         "The exact seed did not pay Treasure with Fire");
  expect(trace_contains("Banked Crispin"),
         "The exact seed did not bank Crispin through Star Alchemy");
  expect(trace_contains("Held Celestial Roar"),
         "The exact seed exposed the deterministic route to Celestial Roar");
}

}  // namespace

int main() {
  try {
    public_preflight_and_k1_gate();
    complete_t1_sequence_is_deterministic();
    missing_post_crispin_vessel_target_rejects();
    missing_route_piece_rejects();
    public_controls_reject();
    exact_seed_reaches_turn_two();
    std::cout << "issue-1811 Arven Treasure Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "issue-1811 Arven Treasure Crispin test failure: "
              << error.what() << '\n';
    return 1;
  }
}
