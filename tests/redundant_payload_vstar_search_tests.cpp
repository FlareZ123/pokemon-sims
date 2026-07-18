#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool seen) { engine.deck_seen_ = seen; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
  static bool play_ultra_ball(Engine& engine) { return engine.play_ultra_ball(false); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode lock = sim::LockMode::TurnTwoItem)
      : scenario{"issue-925", sim::DciProfile::StrictJit, lock, false, 4},
        recipe(sim::baseline_recipe()), rng(925), engine(scenario, recipe, rng) {}
};

sim::State treasure_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::DialgaGX,
                sim::Card::MegaDragonite, sim::Card::ProfessorBurnet,
                sim::Card::ForestSealStone};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Dragapult,
                sim::Card::Oricorio, sim::Card::Grass, sim::Card::Fire};
  return state;
}

void test_treasure_spends_one_redundant_payload_for_direct_vstar() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, treasure_state());

  // Mysterious Treasure may discard one redundant Dragon to search the missing
  // Regidrago VSTAR before turn-two Item lock. Professor Burnet remains a legal
  // Supporter outlet and another payload remains protected for strict DCI:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/me2pt5-16
  // https://github.com/FlareZ123/pokemon-sims/issues/925
  expect(sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
         "The confirmed direct-VSTAR route should spend one redundant payload.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Mysterious Treasure should fetch Regidrago VSTAR.");
  expect(contains(after.discard, sim::Card::MegaDragonite),
         "The lowest-priority modeled redundant payload should pay the cost.");
  expect(contains(after.hand, sim::Card::DialgaGX),
         "A second payload must remain protected in hand.");
}

void test_treasure_controls_preserve_strict_dci() {
  {
    Fixture fixture;
    sim::State state = treasure_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite));
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "The sole payload must remain protected.");
  }
  {
    Fixture fixture;
    sim::State state = treasure_state();
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::RegidragoVstar));
    state.discard = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                     sim::Card::RegidragoVstar};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "A known unavailable VSTAR target must block the exception.");
  }
  {
    Fixture fixture;
    sim::State state = treasure_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::ProfessorBurnet));
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "A route without a Supporter-based later payload outlet must stay blocked.");
  }
  {
    Fixture fixture(sim::LockMode::None);
    sim::EngineTestAccess::set_state(fixture.engine, treasure_state());
    expect(!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "The exception must not spend a payload without scheduled Item-lock pressure.");
  }
  {
    Fixture fixture;
    sim::State state = treasure_state();
    state.deck = {sim::Card::RegidragoVstar, sim::Card::Oricorio,
                  sim::Card::Grass, sim::Card::Fire};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    sim::EngineTestAccess::set_deck_seen(fixture.engine, true);
    expect(!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "Known-dead Burnet must not justify an early payload discard.");
  }
}

void test_ordinary_cost_remains_preferred() {
  Fixture fixture;
  sim::State state = treasure_state();
  state.hand.push_back(sim::Card::Arven);
  state.hand.push_back(sim::Card::Arven);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
         "A duplicate Supporter should keep the search payable.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(count(after.discard, sim::Card::Arven) == 1,
         "The ordinary duplicate cost must stay ahead of redundant payloads.");
  expect(contains(after.hand, sim::Card::MegaDragonite) &&
             contains(after.hand, sim::Card::DialgaGX),
         "Both payloads should remain when ordinary DCI fuel exists.");
}

void test_ultra_ball_uses_one_payload_plus_one_ordinary_cost() {
  Fixture fixture;
  sim::State state = treasure_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::MysteriousTreasure));
  state.hand.push_back(sim::Card::UltraBall);
  state.hand.push_back(sim::Card::Arven);
  state.hand.push_back(sim::Card::Arven);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Ultra Ball requires two other cards. One redundant payload plus one duplicate
  // Supporter is legal, while the second payload remains protected:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/925
  expect(sim::EngineTestAccess::play_ultra_ball(fixture.engine),
         "Ultra Ball should use the narrow redundant-payload fallback.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Ultra Ball should fetch Regidrago VSTAR.");
  expect(contains(after.discard, sim::Card::MegaDragonite) &&
             count(after.discard, sim::Card::Arven) == 1,
         "Ultra Ball should pay with one payload and one ordinary cost.");
  expect(contains(after.hand, sim::Card::DialgaGX),
         "Ultra Ball must leave another payload protected.");
}

void test_ultra_ball_requires_an_ordinary_second_cost() {
  Fixture fixture;
  sim::State state = treasure_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::MysteriousTreasure));
  state.hand.push_back(sim::Card::UltraBall);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::play_ultra_ball(fixture.engine),
         "Two protected payloads alone must not satisfy Ultra Ball's two-card cost.");
}
}  // namespace

int main() {
  test_treasure_spends_one_redundant_payload_for_direct_vstar();
  test_treasure_controls_preserve_strict_dci();
  test_ordinary_cost_remains_preferred();
  test_ultra_ball_uses_one_payload_plus_one_ordinary_cost();
  test_ultra_ball_requires_an_ordinary_second_cost();
  return 0;
}
