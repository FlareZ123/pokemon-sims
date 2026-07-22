#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) {
    return engine.outcome_;
  }
  static bool use_exploding_energy_for_setup(Engine& engine) {
    return engine.use_exploding_energy_for_setup();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  Fixture()
      : scenario{"issue-1329/exact", sim::DciProfile::StrictJit,
                 sim::LockMode::None, false, 5},
        recipe(sim::pineco_recipe()),
        rng(1329),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {}
};

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

std::vector<sim::Card> grass(const int copies) {
  return std::vector<sim::Card>(static_cast<std::size_t>(copies),
                                sim::Card::Grass);
}

sim::Pokemon pokemon(const sim::Card card, const int grass_count = 0,
                     const int fire_count = 0) {
  return sim::Pokemon{card, 1, grass_count, fire_count, sim::Tool::None};
}

void expect_forretress_knockout(const Fixture& fixture) {
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  if (count(state.discard, sim::Card::ForretressEx) != 1 ||
      count(state.discard, sim::Card::Pineco) != 1 ||
      !sim::EngineTestAccess::outcome(fixture.engine).used_exploding_energy) {
    throw std::runtime_error("Exploding Energy did not complete its self-Knock-Out.");
  }
}

void test_selects_only_missing_grass() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = pokemon(sim::Card::RegidragoV, 1, 0);
  state.bench = {pokemon(sim::Card::ForretressEx)};
  state.deck = grass(5);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Forretress ex says "up to 5", so one searched Grass is the complete legal
  // choice when the target already has one Grass and only GGF is modeled:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  if (!sim::EngineTestAccess::use_exploding_energy_for_setup(fixture.engine)) {
    throw std::runtime_error("The one-Grass Exploding Energy route did not resolve.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.active->grass != 2 || count(after.deck, sim::Card::Grass) != 4) {
    throw std::runtime_error("Exploding Energy selected more than the one needed Grass.");
  }
  expect_forretress_knockout(fixture);
}

void test_selects_retreat_cost_plus_missing_grass() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = pokemon(sim::Card::TapuLeleGX);
  state.bench = {pokemon(sim::Card::ForretressEx),
                 pokemon(sim::Card::RegidragoV, 1, 0)};
  state.deck = grass(5);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Tapu Lele-GX has Retreat Cost one. The optimal legal selection is one Grass
  // for that cost and one for Regidrago, preserving the other three in deck:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  if (!sim::EngineTestAccess::use_exploding_energy_for_setup(fixture.engine)) {
    throw std::runtime_error("The paid-Retreat Exploding Energy route did not resolve.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.active->grass != 2 || count(after.deck, sim::Card::Grass) != 3 ||
      count(after.discard, sim::Card::Grass) != 1 || !after.retreat_used) {
    throw std::runtime_error("Exploding Energy did not select exactly two Grass for the paid Retreat route.");
  }
  expect_forretress_knockout(fixture);
}

void test_skyliner_selects_no_retreat_grass() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = pokemon(sim::Card::LatiasEx);
  state.bench = {pokemon(sim::Card::ForretressEx),
                 pokemon(sim::Card::RegidragoV, 1, 0)};
  state.deck = grass(5);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Skyliner removes the Basic Active's Retreat Cost, so only Regidrago's one
  // missing Grass may be selected:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  if (!sim::EngineTestAccess::use_exploding_energy_for_setup(fixture.engine)) {
    throw std::runtime_error("The Skyliner Exploding Energy route did not resolve.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.active->grass != 2 || count(after.deck, sim::Card::Grass) != 4 ||
      count(after.discard, sim::Card::Grass) != 0 || !after.retreat_used) {
    throw std::runtime_error("Skyliner did not preserve all unneeded Grass.");
  }
  expect_forretress_knockout(fixture);
}

void test_insufficient_grass_uses_only_available_copy() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = pokemon(sim::Card::RegidragoV);
  state.bench = {pokemon(sim::Card::ForretressEx)};
  state.deck = grass(1);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // "Up to 5" permits selecting the single available Grass even when it cannot
  // finish GGF. The route must preserve exact availability without inventing cards:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  if (!sim::EngineTestAccess::use_exploding_energy_for_setup(fixture.engine)) {
    throw std::runtime_error("The insufficient-Grass route did not use its one legal copy.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->grass != 1 ||
      count(after.deck, sim::Card::Grass) != 0) {
    throw std::runtime_error("The insufficient-Grass route selected an impossible amount.");
  }
  expect_forretress_knockout(fixture);
}

void test_fire_only_axis_preserves_forretress() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = pokemon(sim::Card::RegidragoV, 2, 0);
  state.bench = {pokemon(sim::Card::ForretressEx)};
  state.deck = grass(5);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Grass cannot advance a Fire-only deficit. The policy must preserve the
  // Forretress ex, its two-Prize liability, and all five Grass:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  if (sim::EngineTestAccess::use_exploding_energy_for_setup(fixture.engine)) {
    throw std::runtime_error("Exploding Energy resolved without a Grass or paid-Retreat axis.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->grass != 2 ||
      count(after.deck, sim::Card::Grass) != 5 || after.bench.size() != 1U ||
      after.bench.front().card != sim::Card::ForretressEx ||
      !after.discard.empty() ||
      sim::EngineTestAccess::outcome(fixture.engine).used_exploding_energy) {
    throw std::runtime_error("The Fire-only control changed public resources.");
  }
}

void test_active_forretress_promotes_minimally_powered_regidrago() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = pokemon(sim::Card::ForretressEx);
  state.bench = {pokemon(sim::Card::RegidragoV, 1, 0)};
  state.deck = grass(5);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // An Active Forretress ex still self-Knocks Out and promotes Regidrago after
  // selecting only the one Grass that the target needs:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  if (!sim::EngineTestAccess::use_exploding_energy_for_setup(fixture.engine)) {
    throw std::runtime_error("The Active Forretress route did not resolve.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.active->grass != 2 || count(after.deck, sim::Card::Grass) != 4) {
    throw std::runtime_error("The Active Forretress route did not preserve surplus Grass.");
  }
  expect_forretress_knockout(fixture);
}

}  // namespace

int main() {
  try {
    test_selects_only_missing_grass();
    test_selects_retreat_cost_plus_missing_grass();
    test_skyliner_selects_no_retreat_grass();
    test_insufficient_grass_uses_only_available_copy();
    test_fire_only_axis_preserves_forretress();
    test_active_forretress_promotes_minimally_powered_regidrago();
    std::cout << "Issue 1329 Exploding Energy selection tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
