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
    return engine.issue_1821_steven_latias_grass_route_available();
  }
  static bool play(Engine& engine) {
    return engine.play_issue_1821_steven_latias_grass_route();
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

sim::Scenario scenario(const int max_turn = 5) {
  return sim::Scenario{"issue-1821-steven-latias-grass",
                       sim::DciProfile::StrictJit,
                       sim::LockMode::None,
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
      sim::Card::Dragapult,
      sim::Card::StevensResolve,
      sim::Card::EarthenVessel,
      sim::Card::Grass,
      sim::Card::Crispin,
      sim::Card::TeamYellsCheer,
  };
  state.deck = {
      sim::Card::RegidragoV,
      sim::Card::LatiasEx,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::MysteriousTreasure,
      sim::Card::QuickBall,
      sim::Card::MegaDragonite,
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

  explicit Fixture(sim::Scenario selected = scenario())
      : scenario_value(std::move(selected)),
        recipe(sim::baseline_recipe()),
        rng(1821),
        engine(scenario_value, recipe, rng) {}
};

void exact_k1_route_uses_retreat_and_three_targets() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state());
  expect(sim::EngineTestAccess::route(fixture.engine),
         "The complete issue-1821 route was not recognized");
  expect(sim::EngineTestAccess::play(fixture.engine),
         "The issue-1821 Steven route did not execute");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::TapuLeleGX,
         "Tapu Lele-GX was not promoted before Steven ended the turn");
  expect(std::any_of(after.bench.begin(), after.bench.end(),
                     [](const sim::Pokemon& pokemon) {
                       return pokemon.card == sim::Card::Oricorio;
                     }),
         "Oricorio did not move to the Bench");
  expect(after.manual_energy_used && after.retreat_used &&
             after.supporter_used && after.turn_ended,
         "The route did not consume its legal once-per-turn actions");
  expect(contains(after.hand, sim::Card::RegidragoV) &&
             contains(after.hand, sim::Card::LatiasEx) &&
             contains(after.hand, sim::Card::Grass),
         "Steven did not search Regidrago V, Latias ex, and Grass Energy");
  expect(contains(after.discard, sim::Card::Grass) &&
             contains(after.discard, sim::Card::StevensResolve),
         "The Retreat Cost or played Steven was not placed in discard");

  // Exact card text and route authority:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/1821
}

void k0_rejects_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state(), false, false);
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route read deck or Prize identities at K0");
}

void k1_provenance_equivalence() {
  const auto route_is_live = [](const bool deck_seen,
                                const bool prizes_revealed) {
    Fixture fixture;
    sim::EngineTestAccess::set_state(
        fixture.engine, route_state(), deck_seen, prizes_revealed);
    return sim::EngineTestAccess::route(fixture.engine);
  };

  // Either legal inspection supplies the same K1 knowledge for the Steven,
  // Latias ex, and Grass route. K0 remains rejected:
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Correction precedent: https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official attachment, retreat, Supporter, search, evolution, Ability, and Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1928
  expect(route_is_live(true, false),
         "Deck-search K1 rejected the issue-1821 route");
  expect(route_is_live(false, true),
         "Prize-inspection K1 rejected the issue-1821 route");
  expect(!route_is_live(false, false),
         "K0 used the issue-1821 route");
}

void missing_latias_rejects_route() {
  Fixture fixture;
  sim::State state = route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::LatiasEx),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route invented Latias ex");
}

void short_horizon_rejects_route() {
  Fixture fixture{scenario(2)};
  sim::EngineTestAccess::set_state(fixture.engine, route_state());
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route exceeded the T3 horizon");
}

void exact_seed_reaches_turn_three() {
  const auto selected = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value(), "Missing strict-JIT going-second scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{161803};
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
         "Seed 161803 did not reach the deterministic T3 route");
  expect(trace_contains("Regidrago V, Latias ex, Grass Energy"),
         "Seed 161803 did not use the corrected Steven target package");
  expect(trace_contains("T3 | READY"),
         "Seed 161803 did not reach T3 readiness");
}

}  // namespace

int main() {
  try {
    exact_k1_route_uses_retreat_and_three_targets();
    k0_rejects_route();
    k1_provenance_equivalence();
    missing_latias_rejects_route();
    short_horizon_rejects_route();
    exact_seed_reaches_turn_three();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
