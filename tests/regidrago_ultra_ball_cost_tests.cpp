#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool run_search_items_one_step(Engine& engine, const bool permit_payload) {
    return engine.run_search_items_one_step(permit_payload);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Scenario scenario(const char* label) {
  return sim::Scenario{label, sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
}

sim::State state(const sim::Card active, const int grass, const int fire) {
  sim::State result;
  result.turn = 2;
  result.active = sim::Pokemon{active, 0, grass, fire, sim::Tool::None};
  return result;
}

void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_ultra_ball_requires_two_physical_cost_cards() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(77);
  sim::State initial = state(sim::Card::RegidragoV, 2, 1);
  initial.hand = {sim::Card::UltraBall, sim::Card::Dipplin};
  initial.deck = {sim::Card::RegidragoVstar};
  sim::Engine engine(scenario("ultra-ball-one-cost"), recipe, rng);
  sim::EngineTestAccess::set_state(engine, initial);

  // Ultra Ball requires discarding 2 other cards: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  require(!sim::EngineTestAccess::run_search_items_one_step(engine, false),
          "Ultra Ball must not resolve with only one other card in hand.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  require(contains(after.hand, sim::Card::UltraBall) && contains(after.hand, sim::Card::Dipplin) &&
              !contains(after.hand, sim::Card::RegidragoVstar),
          "A failed two-card Ultra Ball cost must preserve the hand and perform no search.");
}

void test_ultra_ball_consumes_two_distinct_cards() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(77);
  sim::State initial = state(sim::Card::RegidragoVstar, 2, 1);
  initial.hand = {sim::Card::UltraBall, sim::Card::MegaDragonite, sim::Card::Dipplin};
  initial.deck = {sim::Card::MawileGX};
  sim::Engine engine(scenario("ultra-ball-two-costs"), recipe, rng);
  sim::EngineTestAccess::set_state(engine, initial);

  require(sim::EngineTestAccess::run_search_items_one_step(engine, true),
          "Ultra Ball should resolve when two other cards are available.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  require(contains(after.discard, sim::Card::MegaDragonite) && contains(after.discard, sim::Card::Dipplin),
          "Ultra Ball must discard both selected physical cost cards.");
  require(!contains(after.hand, sim::Card::MegaDragonite) && !contains(after.hand, sim::Card::Dipplin) &&
              contains(after.hand, sim::Card::MawileGX),
          "Ultra Ball must not duplicate a cost card while completing its search.");
}

}  // namespace

int main() {
  try {
    test_ultra_ball_requires_two_physical_cost_cards();
    test_ultra_ball_consumes_two_distinct_cards();
    std::cout << "PASS Ultra Ball cost regressions\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAIL Ultra Ball cost regressions: " << error.what() << '\n';
    return 1;
  }
}
