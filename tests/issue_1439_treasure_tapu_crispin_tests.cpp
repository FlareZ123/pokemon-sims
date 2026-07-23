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
  static void set_state(Engine& engine, State state, const bool deck_seen) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static bool play_route(Engine& engine) {
    return engine.play_issue_1439_treasure_tapu_crispin_completion();
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

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::State exact_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 2,
                              sim::Tool::ForestSealStone};
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::GoodraVstar,
      sim::Card::MysteriousTreasure,
      sim::Card::Fire,
      sim::Card::RegidragoVstar,
      sim::Card::SecretBox,
      sim::Card::StevensResolve,
      sim::Card::ForretressEx,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Pineco,
      sim::Card::Dawn,
      sim::Card::ForestOfVitality,
      sim::Card::QuickBall,
      sim::Card::WishfulBaton,
      sim::Card::Dragapult,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode lock = sim::LockMode::None)
      : scenario{"issue-1439-controls", sim::DciProfile::StrictJit, lock,
                 false, 5},
        recipe(sim::pineco_recipe()),
        rng(1439),
        engine(scenario, recipe, rng) {}
};

void test_exact_state_uses_lower_resource_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, exact_route_state(), true);

  // The exact K1 state can discard Goodra to Mysterious Treasure, search Psychic
  // Tapu Lele-GX, use Wonder Tag for Crispin, and manually attach the sole
  // searchable Grass. The same T2 deadline must preserve Secret Box and the
  // Pineco/Forretress ex self-Knock-Out line:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1439
  expect(sim::EngineTestAccess::play_route(fixture.engine),
         "The confirmed lower-resource route did not resolve.");
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(state.active && state.active->card == sim::Card::RegidragoVstar &&
             state.active->grass >= 2 && state.active->fire >= 1,
         "The route did not finish the Active Regidrago VSTAR Energy axis.");
  expect(contains(state.discarded_this_turn, sim::Card::GoodraVstar),
         "The route did not establish the same-turn Dragon payload.");
  expect(contains(state.hand, sim::Card::SecretBox) &&
             contains(state.hand, sim::Card::ForretressEx) &&
             contains(state.deck, sim::Card::Pineco),
         "The route did not preserve Secret Box and the Forretress line.");
  expect(state.bench.size() == 1 &&
             state.bench.front().card == sim::Card::TapuLeleGX,
         "The route spent more than the required Tapu Bench slot.");
}

void expect_guarded(sim::State state, const sim::LockMode lock,
                    const std::uint64_t seed, const char* message) {
  const sim::Scenario scenario{"issue-1439-guard", sim::DciProfile::StrictJit,
                               lock, false, 5};
  sim::DeckRecipe recipe = sim::pineco_recipe();
  std::mt19937_64 rng(seed);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), true);
  expect(!sim::EngineTestAccess::play_route(engine), message);
}

void test_route_boundaries() {
  sim::State missing_grass = exact_route_state();
  missing_grass.deck.erase(std::find(missing_grass.deck.begin(),
                                     missing_grass.deck.end(),
                                     sim::Card::Grass));
  expect_guarded(std::move(missing_grass), sim::LockMode::None, 143901,
                 "Missing searchable Grass still spent the protected route.");

  expect_guarded(exact_route_state(), sim::LockMode::FullItem, 143902,
                 "Item lock admitted Mysterious Treasure.");
  expect_guarded(exact_route_state(), sim::LockMode::FullRuleBoxAbility, 143903,
                 "Rule Box Ability lock admitted Wonder Tag.");

  sim::State full_bench = exact_route_state();
  for (int index = 0; index < 5; ++index) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 0});
  }
  expect_guarded(std::move(full_bench), sim::LockMode::None, 143904,
                 "A full Bench admitted Tapu Lele-GX.");

  sim::State supporter_used = exact_route_state();
  supporter_used.supporter_used = true;
  expect_guarded(std::move(supporter_used), sim::LockMode::None, 143905,
                 "A spent Supporter action admitted Crispin.");

  sim::State attachment_used = exact_route_state();
  attachment_used.manual_energy_used = true;
  expect_guarded(std::move(attachment_used), sim::LockMode::None, 143906,
                 "A spent manual attachment admitted the one-type Crispin route.");

  sim::State missing_payload = exact_route_state();
  missing_payload.hand.erase(std::find(missing_payload.hand.begin(),
                                       missing_payload.hand.end(),
                                       sim::Card::GoodraVstar));
  expect_guarded(std::move(missing_payload), sim::LockMode::None, 143907,
                 "Missing held Dragon payload admitted the Treasure route.");

  sim::State missing_vstar = exact_route_state();
  missing_vstar.hand.erase(
      std::remove(missing_vstar.hand.begin(), missing_vstar.hand.end(),
                  sim::Card::RegidragoVstar),
      missing_vstar.hand.end());
  expect_guarded(std::move(missing_vstar), sim::LockMode::None, 143908,
                 "Missing Regidrago VSTAR admitted the route.");

  sim::State ineligible_evolution = exact_route_state();
  ineligible_evolution.active->entered_turn = 2;
  expect_guarded(std::move(ineligible_evolution), sim::LockMode::None, 143909,
                 "A same-turn Regidrago V illegally evolved.");

  sim::State missing_tapu = exact_route_state();
  missing_tapu.deck.erase(std::find(missing_tapu.deck.begin(),
                                    missing_tapu.deck.end(),
                                    sim::Card::TapuLeleGX));
  expect_guarded(std::move(missing_tapu), sim::LockMode::None, 143910,
                 "Missing Tapu Lele-GX admitted the route.");

  sim::State missing_crispin = exact_route_state();
  missing_crispin.deck.erase(std::find(missing_crispin.deck.begin(),
                                       missing_crispin.deck.end(),
                                       sim::Card::Crispin));
  expect_guarded(std::move(missing_crispin), sim::LockMode::None, 143911,
                 "Missing Crispin admitted the route.");
}

void test_registered_seed_52_preserves_secret_box() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered seed-52 fixture is unavailable.");

  std::mt19937_64 rng(52);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The source-bound trace must retain its T2 deadline while avoiding Secret
  // Box, Dawn, Forest of Vitality, and Exploding Energy:
  // https://github.com/FlareZ123/pokemon-sims/issues/1439
  // https://github.com/FlareZ123/pokemon-sims/actions/runs/30026300664
  expect(outcome.first_ready_turn == 2,
         "Seed 52 lost strict-JIT T2 readiness.");
  expect(trace_contains(trace, "Hisuian Goodra VSTAR (Mysterious Treasure cost)"),
         "Seed 52 did not use the held Dragon as the legal Treasure cost.");
  expect(trace_contains(trace, "WONDER TAG") &&
             trace_contains(trace, "Crispin for the missing Grass Energy axis"),
         "Seed 52 did not complete the Tapu-Crispin search chain.");
  expect(!trace_contains(trace, "T2 | PLAY ITEM | rules: R-SECRET-BOX-01"),
         "Seed 52 still spent Secret Box.");
  expect(!trace_contains(trace, "T2 | EXPLODING ENERGY"),
         "Seed 52 still used the Forretress ex self-Knock-Out.");
  expect(!trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-DAWN-01"),
         "Seed 52 still spent Dawn.");
}

}  // namespace

int main() {
  try {
    test_exact_state_uses_lower_resource_route();
    test_route_boundaries();
    test_registered_seed_52_preserves_secret_box();
    std::cout << "Issue 1439 Treasure-Tapu-Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
