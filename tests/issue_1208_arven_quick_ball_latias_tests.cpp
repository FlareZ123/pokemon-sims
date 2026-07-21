#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_known_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool direct_route_completes(Engine& engine) {
    return engine.held_quick_ball_latias_completes_current_turn_without_arven();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State complete_route_state() {
  sim::State state;
  state.turn = 4;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 3, 2, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::Arven, sim::Card::RegidragoVstar,
                sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::LatiasEx, sim::Card::MysteriousTreasure,
                sim::Card::Grass};
  state.prizes = {sim::Card::ProfessorBurnet, sim::Card::RegidragoV,
                  sim::Card::ForestSealStone, sim::Card::QuickBall,
                  sim::Card::QuickBall, sim::Card::Grass};
  return state;
}

sim::Engine make_engine(const sim::LockMode lock, const std::uint64_t seed,
                        sim::State state) {
  static const sim::Scenario no_lock{"issue-1208", sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 5};
  static const sim::Scenario item_lock{"issue-1208", sim::DciProfile::StrictJit,
                                       sim::LockMode::FullItem, true, 5};
  static const sim::Scenario ability_lock{
      "issue-1208", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, true, 5};
  const sim::Scenario& scenario = lock == sim::LockMode::FullItem
      ? item_lock
      : lock == sim::LockMode::FullRuleBoxAbility ? ability_lock : no_lock;
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng;
  rng.seed(seed);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_known_state(engine, std::move(state));
  return engine;
}

void test_direct_route_holds_arven_and_reaches_ready() {
  // Quick Ball may discard the held Dragon, search Latias ex, and establish both
  // the current-turn strict-JIT payload and free-retreat route. Arven adds no edge:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1208
  sim::Engine engine = make_engine(sim::LockMode::None, 120800,
                                   complete_route_state());
  expect(sim::EngineTestAccess::direct_route_completes(engine),
         "The complete held Quick Ball-Latias route was not recognized.");
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& state = engine.state();
  expect(!state.supporter_used, "Arven consumed the Supporter play.");
  expect(contains(state.hand, sim::Card::Arven), "Arven was not preserved.");
  expect(state.active && state.active->card == sim::Card::RegidragoVstar,
         "The direct route did not promote Regidrago VSTAR.");
  expect(state.active->grass >= 2 && state.active->fire >= 1,
         "The promoted Regidrago VSTAR was not GGF-ready.");
  expect(contains(state.discard, sim::Card::QuickBall),
         "Quick Ball was not resolved.");
  expect(contains(state.discarded_this_turn, sim::Card::MegaDragonite),
         "The held Dragon did not satisfy strict-JIT payload timing.");
}

void test_boundary_controls() {
  auto rejects = [](sim::State state, const sim::LockMode lock,
                    const std::uint64_t seed, const char* message) {
    sim::Engine engine = make_engine(lock, seed, std::move(state));
    expect(!sim::EngineTestAccess::direct_route_completes(engine), message);
  };

  sim::State no_quick_ball = complete_route_state();
  no_quick_ball.hand.erase(std::find(no_quick_ball.hand.begin(),
                                     no_quick_ball.hand.end(),
                                     sim::Card::QuickBall));
  rejects(std::move(no_quick_ball), sim::LockMode::None, 120801,
          "Missing Quick Ball must keep Arven live.");

  sim::State no_payload = complete_route_state();
  no_payload.hand.erase(std::find(no_payload.hand.begin(), no_payload.hand.end(),
                                  sim::Card::MegaDragonite));
  rejects(std::move(no_payload), sim::LockMode::None, 120802,
          "Missing Dragon payload cost must keep Arven live.");

  sim::State no_latias = complete_route_state();
  no_latias.deck.erase(std::find(no_latias.deck.begin(), no_latias.deck.end(),
                                 sim::Card::LatiasEx));
  rejects(std::move(no_latias), sim::LockMode::None, 120803,
          "Missing Latias ex target must keep Arven live.");

  sim::State full_bench = complete_route_state();
  while (full_bench.bench.size() < 5U) {
    full_bench.bench.push_back(
        sim::Pokemon{sim::Card::Oricorio, 3, 0, 0, sim::Tool::None});
  }
  rejects(std::move(full_bench), sim::LockMode::None, 120804,
          "A full Bench must keep Arven live.");

  rejects(complete_route_state(), sim::LockMode::FullItem, 120805,
          "Item lock must keep Arven live.");
  rejects(complete_route_state(), sim::LockMode::FullRuleBoxAbility, 120806,
          "Rule Box Ability lock must keep Arven live.");

  sim::State missing_vstar = complete_route_state();
  missing_vstar.hand.erase(std::find(missing_vstar.hand.begin(),
                                     missing_vstar.hand.end(),
                                     sim::Card::RegidragoVstar));
  rejects(std::move(missing_vstar), sim::LockMode::None, 120807,
          "Missing held VSTAR must keep Arven live.");

  sim::State incomplete_energy = complete_route_state();
  incomplete_energy.bench.front().fire = 0;
  rejects(std::move(incomplete_energy), sim::LockMode::None, 120808,
          "Incomplete Energy must keep Arven live.");
}

void test_seed_73_preserves_arven() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-first scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{73};
  sim::TraceLog trace{true, {}, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The exact production seed keeps the same earliest T4 result while preserving
  // Arven and avoiding the unused Mysterious Treasure search:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1208
  expect(outcome.first_ready_turn == 4,
         "Seed 73 did not retain its earliest T4 ready result.");
  const bool played_arven = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("Arven searched") != std::string::npos;
      });
  const bool held_arven = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("Retained Arven because held Quick Ball") !=
            std::string::npos;
      });
  expect(!played_arven, "Seed 73 still spent Arven on an unused Item.");
  expect(held_arven, "Seed 73 did not record the resource-preserving hold.");
}

}  // namespace

int main() {
  try {
    test_direct_route_holds_arven_and_reaches_ready();
    test_boundary_controls();
    test_seed_73_preserves_arven();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
