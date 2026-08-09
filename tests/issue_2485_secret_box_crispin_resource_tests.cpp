#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool preflight(const Engine& engine) {
    return engine.issue_2485_direct_crispin_preflight();
  }
  static bool play_route(Engine& engine) {
    return engine.play_issue_2485_direct_crispin_route();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::DeckRecipe pineco_recipe() {
  return {
      {sim::Card::SecretBox, 1},
      {sim::Card::Pineco, 2},
      {sim::Card::ForretressEx, 2},
      {sim::Card::Dawn, 1},
      {sim::Card::ForestOfVitality, 1},
      {sim::Card::MysteriousTreasure, 1},
      {sim::Card::Crispin, 1},
      {sim::Card::RegidragoVstar, 1},
      {sim::Card::GoodraVstar, 1},
      {sim::Card::Grass, 3},
      {sim::Card::Fire, 3},
  };
}

struct Fixture {
  sim::Scenario scenario{"issue-2485", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe{pineco_recipe()};
  std::mt19937_64 rng{2485};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State seed_40_shape() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.hand = {
      sim::Card::Fire,
      sim::Card::GoodraVstar,
      sim::Card::Grass,
      sim::Card::Gladion,
      sim::Card::StevensResolve,
      sim::Card::SecretBox,
      sim::Card::WishfulBaton,
  };
  state.deck = {
      sim::Card::MysteriousTreasure,
      sim::Card::Crispin,
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::Dawn,
      sim::Card::ForestOfVitality,
      sim::Card::Pineco,
      sim::Card::Pineco,
      sim::Card::ForretressEx,
      sim::Card::ForretressEx,
  };
  return state;
}

void test_direct_route_preserves_forretress() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, seed_40_shape());

  // The K1 deck contains both different Basic Energy types required for Crispin's
  // attachment branch. Secret Box may independently search Mysterious Treasure and
  // Crispin, and its Goodra cost is the current-turn Dragon payload. Crispin plus
  // the unused manual attachment completes GGF, then Treasure searches the VSTAR.
  // This reaches T2 without using Forretress ex's two-Prize self-KO Ability.
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Official Item, Supporter, Energy, search, discard, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Resource-priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2485
  expect(sim::EngineTestAccess::preflight(fixture.engine),
         "#2485 direct K1 Secret Box route was not admitted.");
  expect(sim::EngineTestAccess::play_route(fixture.engine),
         "#2485 direct Secret Box route did not complete.");

  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  const sim::TrialOutcome& outcome = sim::EngineTestAccess::outcome(fixture.engine);
  expect(state.active && state.active->card == sim::Card::RegidragoVstar,
         "#2485 route did not evolve the Active Regidrago VSTAR.");
  expect(state.active->grass + state.active->fire == 3,
         "#2485 route did not finish three Basic Energy units.");
  expect(std::find(state.discard.begin(), state.discard.end(),
                   sim::Card::GoodraVstar) != state.discard.end(),
         "#2485 Secret Box cost did not supply the strict-JIT Dragon payload.");
  expect(outcome.used_secret_box,
         "#2485 route did not record Secret Box usage.");
  expect(!outcome.used_exploding_energy,
         "#2485 direct route incorrectly used Exploding Energy.");
  expect(std::none_of(state.discard.begin(), state.discard.end(),
                      [](const sim::Card card) {
                        return card == sim::Card::Pineco ||
                               card == sim::Card::ForretressEx;
                      }),
         "#2485 direct route spent the preserved Pineco/Forretress line.");
}

void test_one_searchable_basic_rejects_direct_route() {
  Fixture fixture;
  sim::State state = seed_40_shape();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Fire),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // With only one Basic Energy type in deck, Crispin puts that single card into
  // hand and attaches none. The direct route therefore cannot claim the effect
  // attachment needed to reach GGF and must leave the existing Forretress fallback.
  // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Forretress ex fallback: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/2485
  expect(!sim::EngineTestAccess::preflight(fixture.engine),
         "#2485 admitted a direct Crispin route with one searchable Basic type.");
}

void test_zero_staged_energy_preserves_forretress_fallback() {
  Fixture fixture;
  sim::State state = seed_40_shape();
  state.active->grass = 0;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Crispin plus one manual attachment supplies only two Energy units from this
  // state, short of Apex Dragon's GGF cost. The wrapper must therefore preserve the
  // existing Forretress route instead of treating equal-turn resource preservation
  // as available.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/2485
  expect(!sim::EngineTestAccess::preflight(fixture.engine),
         "#2485 direct route admitted a Regidrago with no staged Energy.");
}
}  // namespace

int main() {
  try {
    test_direct_route_preserves_forretress();
    test_one_searchable_basic_rejects_direct_route();
    test_zero_staged_energy_preserves_forretress_fallback();
    std::cout << "issue_2485_secret_box_crispin_resource_tests: all checks passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
