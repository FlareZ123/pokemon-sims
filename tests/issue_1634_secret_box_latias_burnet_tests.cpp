#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static State& state(Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) {
    return engine.outcome_;
  }
  static void establish_k1(Engine& engine) {
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1634_secret_box_latias_burnet_axes_visible() &&
        engine.issue_1634_secret_box_latias_burnet_costs().has_value();
  }
  static bool play_secret_box(Engine& engine) {
    return engine.play_secret_box();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State route_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1,
                   sim::Tool::None},
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                   sim::Tool::None},
  };
  state.hand = {
      sim::Card::StevensResolve, sim::Card::Fire, sim::Card::Guzma,
      sim::Card::Fire, sim::Card::Crispin, sim::Card::Arven,
      sim::Card::SecretBox, sim::Card::Pineco,
  };
  state.deck = {
      sim::Card::MysteriousTreasure, sim::Card::LatiasEx,
      sim::Card::ProfessorBurnet, sim::Card::MegaDragonite,
      sim::Card::GoodraVstar, sim::Card::Grass, sim::Card::Grass,
      sim::Card::Fire, sim::Card::QuickBall,
  };
  state.prizes = {
      sim::Card::ErikasInvitation, sim::Card::Appletun,
      sim::Card::Grant, sim::Card::Dragapult,
      sim::Card::RegidragoV, sim::Card::ProfessorTuro,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode lock = sim::LockMode::None)
      : scenario{"issue-1634/exact", sim::DciProfile::MatchupFlexJit,
                 lock, false, 5},
        recipe(sim::pineco_recipe()),
        rng(1634),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {}
};

void test_exact_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state());
  sim::EngineTestAccess::establish_k1(fixture.engine);

  // Secret Box pays three ordinary route-replaced cards and uses only its Item
  // and Supporter searches. Treasure pays a separate fourth card to find the
  // Basic Psychic Latias ex, Burnet establishes current-turn Dragon payload,
  // and Skyliner gives the Basic Dialga-GX a free retreat into the GGF VSTAR:
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1634
  expect(sim::EngineTestAccess::route_available(fixture.engine),
         "The exact issue-1634 route predicate was not admitted.");
  expect(sim::EngineTestAccess::play_secret_box(fixture.engine),
         "The exact issue-1634 Secret Box route was not resolved.");

  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  const sim::TrialOutcome& outcome =
      sim::EngineTestAccess::outcome(fixture.engine);
  expect(state.active && state.active->card == sim::Card::RegidragoVstar &&
             state.active->grass >= 2 && state.active->fire >= 1,
         "The issue-1634 route did not promote the powered VSTAR.");
  expect(std::any_of(state.bench.begin(), state.bench.end(),
                     [](const sim::Pokemon& pokemon) {
                       return pokemon.card == sim::Card::LatiasEx;
                     }),
         "The issue-1634 route did not Bench Latias ex.");
  expect(contains(state.discard, sim::Card::MysteriousTreasure) &&
             contains(state.discard, sim::Card::ProfessorBurnet) &&
             (contains(state.discard, sim::Card::MegaDragonite) ||
              contains(state.discard, sim::Card::GoodraVstar)),
         "The issue-1634 route did not establish its Item and payload discard.");
  expect(state.supporter_used && state.retreat_used && outcome.used_secret_box &&
             !outcome.used_exploding_energy,
         "The issue-1634 route consumed the wrong turn resources.");
}

bool route_available(sim::State state, const sim::LockMode lock,
                     const bool establish_k1) {
  Fixture fixture(lock);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (establish_k1) sim::EngineTestAccess::establish_k1(fixture.engine);
  return sim::EngineTestAccess::route_available(fixture.engine);
}

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto found = std::find(cards.begin(), cards.end(), card);
  if (found != cards.end()) cards.erase(found);
}

void test_route_gates() {
  expect(!route_available(route_state(), sim::LockMode::None, false),
         "The issue-1634 route used K0 deck identities.");
  expect(!route_available(route_state(), sim::LockMode::FullItem, true),
         "The issue-1634 route ignored Item lock.");
  expect(!route_available(route_state(), sim::LockMode::FullSupporter, true),
         "The issue-1634 route ignored Supporter lock.");
  expect(!route_available(route_state(), sim::LockMode::FullRuleBoxAbility, true),
         "The issue-1634 route ignored Rule Box Ability lock.");

  sim::State full_bench = route_state();
  full_bench.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  expect(!route_available(std::move(full_bench), sim::LockMode::None, true),
         "The issue-1634 route ignored a full Bench.");

  sim::State retreat_spent = route_state();
  retreat_spent.retreat_used = true;
  expect(!route_available(std::move(retreat_spent), sim::LockMode::None, true),
         "The issue-1634 route ignored a spent retreat.");

  sim::State incomplete_energy = route_state();
  incomplete_energy.bench.front().grass = 1;
  expect(!route_available(std::move(incomplete_energy), sim::LockMode::None, true),
         "The issue-1634 route ignored incomplete GGF.");

  sim::State no_fourth_cost = route_state();
  no_fourth_cost.hand = {
      sim::Card::SecretBox, sim::Card::Grant,
      sim::Card::WishfulBaton, sim::Card::ErikasInvitation,
  };
  expect(!route_available(std::move(no_fourth_cost), sim::LockMode::None, true),
         "The issue-1634 route ignored Treasure's separate discard cost.");

  sim::State no_latias = route_state();
  erase_one(no_latias.deck, sim::Card::LatiasEx);
  expect(!route_available(std::move(no_latias), sim::LockMode::None, true),
         "The issue-1634 route ignored a missing Latias ex.");

  sim::State no_burnet = route_state();
  erase_one(no_burnet.deck, sim::Card::ProfessorBurnet);
  expect(!route_available(std::move(no_burnet), sim::LockMode::None, true),
         "The issue-1634 route ignored a missing Professor Burnet.");

  sim::State no_payload = route_state();
  erase_one(no_payload.deck, sim::Card::MegaDragonite);
  erase_one(no_payload.deck, sim::Card::GoodraVstar);
  expect(!route_available(std::move(no_payload), sim::LockMode::None, true),
         "The issue-1634 route ignored a missing deck payload.");
}

void test_registered_seed_123() {
  const auto scenario =
      sim::scenario_by_label("matchup-flex-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1634 fixture is unavailable.");

  std::mt19937_64 rng(123);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The registered trace must use the complete public T4 chain and retain the
  // repository's earliest-ready objective instead of ending T4 with Steven:
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1634
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "Pineco seed 123 did not reach T4 readiness.");
  expect(trace_contains(trace,
                        "Secret Box discarded three route-replaced cards") &&
             trace_contains(trace,
                            "Mysterious Treasure discarded the reserved fourth cost and searched Latias ex") &&
             trace_contains(trace,
                            "T4 | PLAY SUPPORTER | rules: R-BURNET-01") &&
             trace_contains(trace, "T4 | RETREAT |") &&
             trace_contains(trace, "T4 | READY |") &&
             !trace_contains(trace,
                             "T4 | PLAY SUPPORTER | rules: R-STEVEN-01"),
         "Pineco seed 123 did not use the Secret Box-Latias-Burnet route.");
}

}  // namespace

int main() {
  test_exact_route();
  test_route_gates();
  test_registered_seed_123();
  return 0;
}
