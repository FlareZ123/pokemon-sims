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
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = false,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_2165_burnet_treasure_vstar_route_available();
  }
  static bool play_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
  static bool prizes_known(const Engine& engine) { return engine.prizes_known(); }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode lock = sim::LockMode::None)
      : scenario{"issue-2165", sim::DciProfile::StrictJit, lock, false, 5},
        recipe(sim::baseline_recipe()),
        rng(2165),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {}
};

sim::State exact_public_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::ProfessorBurnet,
                sim::Card::UltraBall, sim::Card::Dragapult,
                sim::Card::LatiasEx, sim::Card::Fire};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_treasure_precedes_burnet_and_establishes_k1() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, exact_public_state());

  // Mysterious Treasure has a one-card cost, Ultra Ball is redundant in this
  // exact public hand, and the Item search advances the VSTAR axis before the
  // one-per-turn Professor Burnet decision:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Official Trainer and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1 and action ordering: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2165
  expect(sim::EngineTestAccess::route_available(fixture.engine),
         "The exact public Treasure-before-Burnet route was rejected.");
  expect(sim::EngineTestAccess::play_treasure(fixture.engine),
         "Mysterious Treasure did not resolve.");

  const sim::State& after = fixture.engine.state();
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Mysterious Treasure did not search Regidrago VSTAR.");
  expect(contains(after.discard, sim::Card::MysteriousTreasure) &&
             contains(after.discard, sim::Card::UltraBall),
         "The Item and redundant Ultra Ball cost were not discarded.");
  expect(contains(after.hand, sim::Card::Dragapult),
         "The UDP-sensitive Dragon payload was discarded instead of redundant Ultra Ball.");
  expect(contains(after.hand, sim::Card::ProfessorBurnet) &&
             !after.supporter_used,
         "The Treasure route consumed or lost the Turn 1 Supporter decision.");
  expect(sim::EngineTestAccess::prizes_known(fixture.engine),
         "The legal deck search did not establish K1.");
}

void test_legality_and_scope_boundaries() {
  {
    Fixture fixture(sim::LockMode::FullItem);
    sim::EngineTestAccess::set_state(fixture.engine, exact_public_state());
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "The route bypassed Item lock.");
  }
  {
    Fixture fixture;
    sim::State state = exact_public_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::UltraBall), state.hand.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "The route ignored its low-DCI Ultra Ball cost requirement.");
  }
  {
    Fixture fixture;
    sim::State state = exact_public_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::RegidragoVstar), state.deck.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true, false);
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "K1 ignored the absence of Regidrago VSTAR from the physical deck.");
  }
  {
    Fixture fixture;
    sim::State state = exact_public_state();
    state.supporter_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "The route expanded beyond the pre-Supporter ordering window.");
  }
  {
    Fixture fixture;
    sim::State state = exact_public_state();
    state.bench.clear();
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "The issue-specific route expanded beyond the confirmed Tapu board.");
  }
}

}  // namespace

int main() {
  test_treasure_precedes_burnet_and_establishes_k1();
  test_legality_and_scope_boundaries();
  return 0;
}
