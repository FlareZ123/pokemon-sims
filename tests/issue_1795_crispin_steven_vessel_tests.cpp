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
  static bool route_available(const Engine& engine) {
    return engine.issue_1795_crispin_steven_vessel_route_available();
  }
  static void choose_supporter(Engine& engine) {
    engine.choose_supporter_issue_1152();
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
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
    const bool going_first = true,
    const int max_turn = 5) {
  return sim::Scenario{"issue-1795-crispin-steven-vessel",
                       sim::DciProfile::StrictJit, locks, going_first,
                       max_turn};
}

sim::State base_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::MegaDragonite,
      sim::Card::Serena,
      sim::Card::StevensResolve,
      sim::Card::Gladion,
      sim::Card::Crispin,
      sim::Card::TapuLeleGX,
  };
  state.deck = {
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::RegidragoVstar,
      sim::Card::RegidragoVstar,
      sim::Card::EarthenVessel,
      sim::Card::EarthenVessel,
      sim::Card::QuickBall,
  };
  state.prizes = {
      sim::Card::MysteriousTreasure,
      sim::Card::Arven,
      sim::Card::Guzma,
      sim::Card::Dragapult,
      sim::Card::Grass,
      sim::Card::Fire,
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
          const std::uint64_t seed = 1795)
      : scenario_value(std::move(selected_scenario)),
        recipe(std::move(selected_recipe)),
        rng(seed),
        engine(scenario_value, recipe, rng) {}
};

void crispin_outranks_gladion_for_complete_schedule() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state());

  // K1 proves the two Crispin Energy targets, the final Grass, Regidrago VSTAR,
  // Earthen Vessel, and a held Dragon cost. The Supporter schedule uses Crispin on
  // T2 and Steven on T3, then Vessel supplies the strict-JIT payload and final
  // attachment on T4:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1795
  expect(sim::EngineTestAccess::route_available(fixture.engine),
         "The complete Crispin-Steven-Vessel route was not recognized");
  sim::EngineTestAccess::choose_supporter(fixture.engine);

  const sim::State& after_crispin = sim::EngineTestAccess::state(fixture.engine);
  expect(after_crispin.supporter_used, "Crispin did not consume the Supporter action");
  expect(contains(after_crispin.discard, sim::Card::Crispin),
         "Crispin did not enter discard");
  expect(contains(after_crispin.hand, sim::Card::Gladion),
         "Gladion was incorrectly consumed");
  expect(contains(after_crispin.hand, sim::Card::StevensResolve),
         "Steven's Resolve was not preserved for T3");
  expect(contains(after_crispin.hand, sim::Card::MegaDragonite),
         "The strict-JIT Vessel payload was not preserved");
  expect(after_crispin.active->grass + after_crispin.active->fire == 1,
         "Crispin did not attach exactly one Basic Energy");
  expect(sim::EngineTestAccess::attach_manual(fixture.engine),
         "The second Crispin Energy was not manually attachable");
  const sim::State& after_attachment = sim::EngineTestAccess::state(fixture.engine);
  expect(after_attachment.active->grass == 1 &&
             after_attachment.active->fire == 1,
         "Crispin plus the manual attachment did not reach GF");
}

void k0_rejects_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state(), false);
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route read deck or Prize identities at K0");
}

void missing_final_grass_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Grass));
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Grass));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route invented the post-Crispin Vessel Grass target");
}

void missing_fire_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Fire),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route invented Crispin's Fire target");
}

void missing_vstar_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(
      std::remove(state.deck.begin(), state.deck.end(), sim::Card::RegidragoVstar),
      state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "Steven invented an absent Regidrago VSTAR");
}

void missing_vessel_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(
      std::remove(state.deck.begin(), state.deck.end(), sim::Card::EarthenVessel),
      state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "Steven invented an absent Earthen Vessel");
}

void missing_payload_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::MegaDragonite));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route invented a strict-JIT Vessel payload cost");
}

void spent_attachment_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route reused the T2 manual attachment");
}

void current_turn_active_rejects_evolution_schedule() {
  Fixture fixture;
  sim::State state = base_state();
  state.active->entered_turn = 2;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route ignored Regidrago V evolution timing");
}

void lock_rejects_future_vessel() {
  Fixture fixture{scenario(sim::LockMode::FullItem)};
  sim::EngineTestAccess::set_state(fixture.engine, base_state());
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route projected Earthen Vessel through Item lock");
}

void expired_horizon_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::None, true, 3)};
  sim::EngineTestAccess::set_state(fixture.engine, base_state());
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route exceeded the configured setup horizon");
}

void held_direct_vstar_connector_stays_ahead() {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.push_back(sim::Card::RegidragoVstar);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // Preserve a direct current VSTAR connector ahead of the deferred T4 package:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1795
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The deferred route displaced a held direct VSTAR connector");
}

void exact_seed_reaches_turn_four() {
  const auto selected_scenario =
      sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected_scenario.has_value(), "Missing strict-JIT going-first scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{364};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  // Source-bound exact regression. The visible route is independent of future
  // draws and Celestial Roar's top three cards:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1795
  expect(outcome.first_ready_turn == 4,
         "Seed 364 did not reach the deterministic T4 route");
  expect(trace_contains("Selected Crispin over Gladion"),
         "Seed 364 did not select the complete Supporter schedule");
  expect(trace_contains("T4 | READY"),
         "Seed 364 did not become ready on T4");
  expect(trace_contains("T2 | HOLD ATTACK"),
         "Seed 364 did not preserve the known route from the random attack");
  expect(!trace_contains("T2 | ATTACK"),
         "Seed 364 still resolved the random Celestial Roar attack");
}

}  // namespace

int main() {
  try {
    crispin_outranks_gladion_for_complete_schedule();
    k0_rejects_route();
    missing_final_grass_rejects_route();
    missing_fire_rejects_route();
    missing_vstar_rejects_route();
    missing_vessel_rejects_route();
    missing_payload_rejects_route();
    spent_attachment_rejects_route();
    current_turn_active_rejects_evolution_schedule();
    lock_rejects_future_vessel();
    expired_horizon_rejects_route();
    held_direct_vstar_connector_stays_ahead();
    exact_seed_reaches_turn_four();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
