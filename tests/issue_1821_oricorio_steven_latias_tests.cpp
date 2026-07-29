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
  static bool candidate(const Engine& engine) {
    return engine.issue_1821_public_route_candidate();
  }
  static bool targets_available(const Engine& engine) {
    return engine.issue_1821_targets_available();
  }
  static bool play_route(Engine& engine) {
    return engine.play_issue_1821_oricorio_steven_route();
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

sim::Scenario scenario(
    const sim::LockMode locks = sim::LockMode::None,
    const bool going_first = false,
    const int max_turn = 4) {
  return sim::Scenario{"issue-1821-oricorio-steven-latias",
                       sim::DciProfile::StrictJit, locks, going_first, max_turn};
}

sim::State base_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::Dragapult,
      sim::Card::StevensResolve,
      sim::Card::EarthenVessel,
      sim::Card::Grass,
      sim::Card::TeamYellsCheer,
      sim::Card::Crispin,
  };
  state.deck = {
      sim::Card::RegidragoV,
      sim::Card::LatiasEx,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::BrilliantBlender,
      sim::Card::MegaDragonite,
      sim::Card::QuickBall,
  };
  state.prizes = {
      sim::Card::ErikasInvitation,
      sim::Card::MysteriousTreasure,
      sim::Card::Grass,
      sim::Card::Dipplin,
      sim::Card::Powerglass,
      sim::Card::Lusamine,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(sim::Scenario selected_scenario = scenario(),
          const std::uint64_t seed = 1821)
      : scenario_value(std::move(selected_scenario)),
        recipe(sim::baseline_recipe()),
        rng(seed),
        engine(scenario_value, recipe, rng) {}
};

void exact_package_pays_retreat_and_preserves_held_axes() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state());

  // The exact K1 route uses the held Grass for Oricorio's one-Energy Retreat
  // Cost, promotes Tapu Lele-GX, and searches Regidrago V, Latias ex, and Grass.
  // Held Regidrago VSTAR and Earthen Vessel plus Dragapult ex remain protected:
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1821
  expect(sim::EngineTestAccess::candidate(fixture.engine),
         "The public issue-1821 route was not recognized");
  expect(sim::EngineTestAccess::targets_available(fixture.engine),
         "The K1 issue-1821 targets were not recognized");
  expect(sim::EngineTestAccess::play_route(fixture.engine),
         "The issue-1821 route did not resolve");

  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(state.active && state.active->card == sim::Card::TapuLeleGX,
         "Tapu Lele-GX was not promoted before Steven");
  expect(std::any_of(state.bench.begin(), state.bench.end(),
                     [](const sim::Pokemon& pokemon) {
                       return pokemon.card == sim::Card::Oricorio;
                     }),
         "Oricorio was not moved to the Bench");
  expect(state.manual_energy_used, "The retreat attachment was not recorded");
  expect(state.retreat_used, "The retreat action was not recorded");
  expect(state.supporter_used, "Steven's Supporter play was not recorded");
  expect(state.turn_ended, "Steven's Resolve did not end the turn");
  expect(contains(state.hand, sim::Card::RegidragoV),
         "Steven did not search Regidrago V");
  expect(contains(state.hand, sim::Card::LatiasEx),
         "Steven did not search Latias ex");
  expect(count(state.hand, sim::Card::Grass) == 1,
         "Steven did not leave exactly the searched Grass Energy in hand");
  expect(count(state.hand, sim::Card::RegidragoVstar) == 1,
         "The held VSTAR axis was duplicated or consumed");
  expect(contains(state.hand, sim::Card::EarthenVessel),
         "The held Earthen Vessel was not preserved");
  expect(contains(state.hand, sim::Card::Dragapult),
         "The held strict-JIT payload was not preserved");
  expect(!contains(state.hand, sim::Card::BrilliantBlender),
         "Steven searched the redundant Blender axis");
  expect(contains(state.discard, sim::Card::Grass),
         "The paid Retreat Energy did not enter discard");
  expect(contains(state.discard, sim::Card::StevensResolve),
         "Steven's Resolve did not enter discard");
}

void public_and_inventory_guards_reject_invalid_routes() {
  {
    Fixture fixture;
    sim::EngineTestAccess::set_state(fixture.engine, base_state(), false);
    expect(!sim::EngineTestAccess::targets_available(fixture.engine),
           "The route read target identities at K0");
  }
  {
    Fixture fixture{scenario(sim::LockMode::None, true)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::candidate(fixture.engine),
           "The route ignored the first player's Supporter restriction");
  }
  for (const sim::LockMode lock : {sim::LockMode::FullItem,
                                   sim::LockMode::FullRuleBoxAbility,
                                   sim::LockMode::FullSupporter,
                                   sim::LockMode::FullCombined}) {
    Fixture fixture{scenario(lock)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::candidate(fixture.engine),
           "The route projected through an incompatible lock");
  }
  {
    Fixture fixture{scenario(sim::LockMode::None, false, 2)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::candidate(fixture.engine),
           "The route exceeded the configured setup horizon");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    state.bench.resize(4, sim::Pokemon{sim::Card::TapuLeleGX, 1});
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::candidate(fixture.engine),
           "The route ignored the two required T2 Bench spaces");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    state.manual_energy_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::candidate(fixture.engine),
           "The route reused the manual attachment");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    state.retreat_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::candidate(fixture.engine),
           "The route reused the Retreat action");
  }

  for (const sim::Card missing : {sim::Card::StevensResolve,
                                  sim::Card::Crispin,
                                  sim::Card::RegidragoVstar,
                                  sim::Card::EarthenVessel,
                                  sim::Card::Grass,
                                  sim::Card::Dragapult}) {
    Fixture fixture;
    sim::State state = base_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), missing));
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::candidate(fixture.engine),
           "The route invented a missing held axis");
  }

  for (const sim::Card missing : {sim::Card::RegidragoV, sim::Card::LatiasEx}) {
    Fixture fixture;
    sim::State state = base_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), missing),
                     state.deck.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::targets_available(fixture.engine),
           "The route invented a missing Steven target");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Grass));
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::targets_available(fixture.engine),
           "The route ignored a draw-fragile Grass inventory");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Fire));
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Fire));
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::targets_available(fixture.engine),
           "The route ignored a draw-fragile Fire inventory");
  }
}

void exact_seed_reaches_turn_three() {
  const auto selected_scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected_scenario.has_value(), "Missing strict-JIT going-second scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{161803};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  // Exact current-main regression and source-bound CI evidence:
  // https://github.com/FlareZ123/pokemon-sims/issues/1821
  // https://github.com/FlareZ123/pokemon-sims/actions/runs/30466318357
  expect(outcome.first_ready_turn == 3,
         "Seed 161803 did not reach the deterministic T3 route");
  expect(trace_contains("Paid Oricorio's one-Energy Retreat Cost"),
         "Seed 161803 did not perform the T1 retreat");
  expect(trace_contains("searched Regidrago V, Latias ex, and Grass Energy"),
         "Seed 161803 did not select the exact Steven package");
  expect(trace_contains("Dragapult ex (Earthen Vessel cost)"),
         "Seed 161803 did not use the held Dragon as the strict-JIT cost");
  expect(trace_contains("T3 | READY"),
         "Seed 161803 did not become ready on T3");
  expect(!trace_contains("Searched up to 3 cards: Regidrago V, Brilliant Blender, Regidrago VSTAR"),
         "Seed 161803 retained the dominated Steven package");
}

}  // namespace

int main() {
  try {
    exact_package_pays_retreat_and_preserves_held_axes();
    public_and_inventory_guards_reject_invalid_routes();
    exact_seed_reaches_turn_three();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
