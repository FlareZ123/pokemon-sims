#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = deck_seen;
  }
  static std::optional<Card> quick_ball_cost(const Engine& engine) {
    return engine.choose_discard(false, true, true, Card::QuickBall, false);
  }
  static bool quick_ball_oricorio_latias_route_available(const Engine& engine) {
    return engine.quick_ball_oricorio_latias_route_available();
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static bool bench_oricorio(Engine& engine) {
    return engine.bench_oricorio_if_useful();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

struct Fixture {
  explicit Fixture(const sim::LockMode locks = sim::LockMode::None,
                   const int max_turn = 4)
      : scenario{"issue-1419", sim::DciProfile::StrictJit, locks, true,
                 max_turn},
        recipe{sim::baseline_recipe()},
        rng{1419},
        engine{scenario, recipe, rng} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::LatiasEx,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Oricorio, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Dragapult, sim::Card::MegaDragonite,
                sim::Card::GoodraVstar, sim::Card::Fire};
  state.prizes = {sim::Card::Arven, sim::Card::Gladion,
                  sim::Card::ForestSealStone, sim::Card::Lusamine,
                  sim::Card::RegidragoV, sim::Card::EarthenVessel};
  return state;
}

void test_k0_hidden_zone_uncertainty_keeps_latias_protected() {
  const auto rejected = [](sim::State state, const char* label) {
    Fixture fixture;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state), false);

    // Before legal deck or Prize inspection, singleton Oricorio may be in either
    // hidden zone. The policy must make the same safe decision in both physical
    // states and cannot spend Latias ex using knowledge obtained after the cost:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Core Item, search, Bench, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1467
    if (sim::EngineTestAccess::quick_ball_oricorio_latias_route_available(
            fixture.engine)) {
      throw std::runtime_error(std::string(label) +
                               ": K0 admitted the singleton Oricorio route");
    }
    if (sim::EngineTestAccess::quick_ball_cost(fixture.engine).has_value()) {
      throw std::runtime_error(std::string(label) +
                               ": K0 exposed Latias ex before inspection");
    }
    if (sim::EngineTestAccess::play_quick_ball(fixture.engine)) {
      throw std::runtime_error(std::string(label) +
                               ": K0 paid Quick Ball with no safe cost");
    }

    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
    if (std::find(after.hand.begin(), after.hand.end(), sim::Card::LatiasEx) ==
        after.hand.end()) {
      throw std::runtime_error(std::string(label) +
                               ": K0 discarded protected Latias ex");
    }
    if (std::find(after.hand.begin(), after.hand.end(), sim::Card::QuickBall) ==
        after.hand.end()) {
      throw std::runtime_error(std::string(label) +
                               ": K0 consumed Quick Ball");
    }
  };

  rejected(route_state(), "Oricorio physically in hidden deck");

  sim::State prized = route_state();
  prized.deck.erase(std::find(prized.deck.begin(), prized.deck.end(),
                              sim::Card::Oricorio));
  prized.prizes.front() = sim::Card::Oricorio;
  rejected(std::move(prized), "Oricorio physically in hidden Prizes");
}

void test_selects_latias_only_for_complete_k1_oricorio_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state());

  // Active Regidrago VSTAR has already replaced Latias ex's Skyliner setup role.
  // At K1, Quick Ball can spend Latias because Oricorio and both Grass targets are
  // legally known, T3 and T4 provide the attachments, and Blender retains payload:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1467
  expect(sim::EngineTestAccess::quick_ball_cost(fixture.engine) ==
             sim::Card::LatiasEx,
         "complete K1 Oricorio route did not expose inert Latias ex");
  expect(sim::EngineTestAccess::play_quick_ball(fixture.engine),
         "Quick Ball did not resolve the proven K1 Oricorio search");
  expect(sim::EngineTestAccess::bench_oricorio(fixture.engine),
         "Oricorio was not Benched for Vital Dance");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(std::count(after.hand.begin(), after.hand.end(), sim::Card::Grass) == 2,
         "Vital Dance did not put both required Grass Energy into hand");
  expect(std::find(after.discard.begin(), after.discard.end(),
                   sim::Card::LatiasEx) != after.discard.end(),
         "Latias ex did not pay Quick Ball's printed K1 discard cost");
  expect(std::any_of(after.bench.begin(), after.bench.end(),
                     [](const sim::Pokemon& pokemon) {
                       return pokemon.card == sim::Card::Oricorio;
                     }),
         "searched Oricorio did not remain on the Bench");
}

void test_preserves_latias_at_every_k1_route_boundary() {
  const auto rejected = [](sim::State state, const sim::LockMode lock,
                           const int max_turn, const char* label) {
    Fixture fixture{lock, max_turn};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    if (sim::EngineTestAccess::quick_ball_cost(fixture.engine) ==
        sim::Card::LatiasEx) {
      throw std::runtime_error(std::string(label) +
                               ": Latias ex must remain protected");
    }
  };

  sim::State state = route_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 1,
                              sim::Tool::None};
  rejected(std::move(state), sim::LockMode::None, 4, "Basic Active");

  state = route_state();
  state.bench.assign(5, sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                                     sim::Tool::None});
  rejected(std::move(state), sim::LockMode::None, 4, "full Bench");

  state = route_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::Oricorio));
  rejected(std::move(state), sim::LockMode::None, 4, "Oricorio absent");

  rejected(route_state(), sim::LockMode::FullRuleBoxAbility, 4,
           "Rule Box Ability lock");

  state = route_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::Grass));
  rejected(std::move(state), sim::LockMode::None, 4,
           "insufficient Grass");

  rejected(route_state(), sim::LockMode::None, 3, "short horizon");

  state = route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::BrilliantBlender));
  rejected(std::move(state), sim::LockMode::None, 4,
           "missing Blender");

  state = route_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::GoodraVstar));
  rejected(std::move(state), sim::LockMode::None, 4,
           "payload cannot survive both draws");
}

void test_ordinary_lower_dci_cost_stays_ahead_of_latias_at_k1() {
  Fixture fixture;
  sim::State state = route_state();
  state.hand.push_back(sim::Card::Dipplin);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::quick_ball_cost(fixture.engine) ==
             sim::Card::Dipplin,
         "route-specific Latias downgrade displaced ordinary dead-card fuel");
}

void test_source_bound_seed_129_rejects_k0_oracle_route() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "registered seed-129 fixture is unavailable");

  std::mt19937_64 rng(129);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Seed 129 has no legal inspection before T2 Quick Ball would pay its discard.
  // The former T4 route depended on the debug-only fact that Oricorio was unprized.
  // Correct K0 policy keeps Latias protected and removes that oracle-derived result:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
  // Original oracle-dependent route: https://github.com/FlareZ123/pokemon-sims/issues/1419
  // Refined correction: https://github.com/FlareZ123/pokemon-sims/issues/1467
  expect(outcome.first_ready_turn == 0 && outcome.setup_failed,
         "seed 129 retained the oracle-dependent T4 ready result");
  expect(!trace_contains(trace, "Latias ex (Quick Ball cost)"),
         "seed 129 discarded Latias ex before legal inspection");
  expect(!trace_contains(trace, "Searched a Basic Pokemon: Oricorio") &&
             !trace_contains(trace, "Searched a Basic Pokémon: Oricorio"),
         "seed 129 searched Oricorio through the rejected K0 route");
  expect(!trace_contains(trace, "Vital Dance"),
         "seed 129 resolved Vital Dance through the rejected K0 route");
}

}  // namespace

int main() {
  test_k0_hidden_zone_uncertainty_keeps_latias_protected();
  test_selects_latias_only_for_complete_k1_oricorio_route();
  test_preserves_latias_at_every_k1_route_boundary();
  test_ordinary_lower_dci_cost_stays_ahead_of_latias_at_k1();
  test_source_bound_seed_129_rejects_k0_oracle_route();
  return 0;
}
