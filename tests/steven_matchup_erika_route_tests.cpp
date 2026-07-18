#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
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

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::DciProfile dci)
      : scenario{"issue-915", dci, sim::LockMode::None, false, 4},
        recipe(sim::baseline_recipe()), rng(915), engine(scenario, recipe, rng) {}
};

sim::State matchup_route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::StevensResolve, sim::Card::RoseannesBackup,
                sim::Card::MysteriousTreasure, sim::Card::Serena,
                sim::Card::Fire, sim::Card::ErikasInvitation, sim::Card::Grass};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::BrilliantBlender,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  return state;
}

void test_matchup_flex_route_discards_erika_and_preserves_fire() {
  Fixture fixture(sim::DciProfile::MatchupFlexJit);
  sim::EngineTestAccess::set_state(fixture.engine, matchup_route_state());

  // Erika's Invitation is inert in this goldfish model and is the documented
  // low-impact matchup-flex cost. Mysterious Treasure can pay with Erika while
  // preserving Fire for the same Steven/Crispin turn-two route:
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/915
  expect(sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
         "The matchup-flex route should make Mysterious Treasure playable.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.discard, sim::Card::ErikasInvitation),
         "Matchup-flex must discard Erika before the route-conditioned Fire fallback.");
  expect(contains(after.hand, sim::Card::Fire),
         "The lower-DCI Erika cost must preserve the public Fire Energy.");
  expect(contains(after.hand, sim::Card::Grass),
         "Grass must remain for the turn-one manual attachment.");
  expect(contains(after.hand, sim::Card::RegidragoV),
         "Mysterious Treasure should still search Regidrago V.");
}

void test_strict_jit_retains_fire_route() {
  Fixture fixture(sim::DciProfile::StrictJit);
  sim::EngineTestAccess::set_state(fixture.engine, matchup_route_state());
  expect(sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
         "Strict JIT should retain the confirmed Fire-funded route.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.discard, sim::Card::Fire),
         "Strict JIT must still use Fire for the issue-775 route.");
  expect(contains(after.hand, sim::Card::ErikasInvitation),
         "Strict JIT must keep the singleton matchup card protected.");
}

void test_matchup_flex_controls() {
  {
    Fixture fixture(sim::DciProfile::MatchupFlexJit);
    sim::State state = matchup_route_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::ErikasInvitation), state.hand.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "The route should fall back to Fire when Erika is absent.");
    expect(contains(sim::EngineTestAccess::state(fixture.engine).discard, sim::Card::Fire),
           "Absent Erika must preserve the existing Fire fallback.");
  }
  {
    Fixture fixture(sim::DciProfile::MatchupFlexJit);
    sim::State state = matchup_route_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::Fire),
                     state.hand.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "Generic matchup-flex Erika discard remains legal without public Fire.");
    expect(contains(sim::EngineTestAccess::state(fixture.engine).discard,
                    sim::Card::ErikasInvitation),
           "Without Fire, Erika should still be selected by the generic flex table.");
  }
  {
    Fixture fixture(sim::DciProfile::MatchupFlexJit);
    sim::State state = matchup_route_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Crispin),
                     state.deck.end());
    state.discard.push_back(sim::Card::Crispin);
    state.discard.push_back(sim::Card::Crispin);
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "Erika remains generic matchup-flex fuel when the Steven route is unavailable.");
    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
    expect(contains(after.discard, sim::Card::ErikasInvitation),
           "An unavailable route component must not force the Fire-specific override.");
    expect(contains(after.hand, sim::Card::Fire),
           "The generic low-DCI cost should still preserve Fire.");
  }
}
}  // namespace

int main() {
  test_matchup_flex_route_discards_erika_and_preserves_fire();
  test_strict_jit_retains_fire_route();
  test_matchup_flex_controls();
  return 0;
}
