#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool live_incense_outlet(Engine& engine) {
    return engine.evolution_incense_has_live_post_payload_outlet();
  }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool play_forretress_incense(Engine& engine) {
    return engine.play_evolution_incense_for_forretress();
  }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::DeckRecipe continuation_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::Pineco, 1);
  recipe.emplace_back(sim::Card::ForretressEx, 1);
  return recipe;
}

struct Fixture {
  sim::Scenario scenario{"issue-1255/continuations", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{continuation_recipe()};
  std::mt19937_64 rng{12550};
  sim::Engine engine{scenario, recipe, rng};
};

sim::Pokemon ready_vstar() {
  return sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
}

void test_post_incense_outlet_projects_appletun() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = ready_vstar();
  state.hand = {sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Appletun, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Evolution Incense removes Appletun, then Mysterious Treasure retains the
  // remaining Dragon target while discarding Appletun for Apex Dragon:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1255
  if (!sim::EngineTestAccess::live_incense_outlet(fixture.engine)) {
    throw std::runtime_error("Post-Incense outlet planner omitted Appletun.");
  }
}

void test_legacy_star_recovers_appletun_incense_bridge() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = ready_vstar();
  state.discard = {sim::Card::EvolutionIncense, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Appletun, sim::Card::RegidragoV,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Legacy Star discards the seven top Grass Energy, then may recover Evolution
  // Incense plus Mysterious Treasure for the legal Appletun payload bridge:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1255
  if (!sim::EngineTestAccess::use_legacy_star(fixture.engine)) {
    throw std::runtime_error("Legacy Star rejected the Appletun Incense bridge.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::EvolutionIncense) ||
      !contains(after.hand, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("Legacy Star failed to recover the Appletun bridge.");
  }
}

void test_forretress_incense_fallback_takes_appletun() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::EvolutionIncense};
  state.deck = {sim::Card::Appletun};
  // K0 keeps the registered Forretress ex plausible until Evolution Incense
  // performs its legal deck inspection and discovers Appletun as the fallback:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/1255
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), false);

  // The Forretress route uses Evolution Incense's exact Evolution target class:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1255
  if (!sim::EngineTestAccess::play_forretress_incense(fixture.engine)) {
    throw std::runtime_error("Forretress Incense route rejected Appletun fallback.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun)) {
    throw std::runtime_error("Forretress Incense fallback omitted Appletun.");
  }
}

}  // namespace

int main() {
  test_post_incense_outlet_projects_appletun();
  test_legacy_star_recovers_appletun_incense_bridge();
  test_forretress_incense_fallback_takes_appletun();
  std::cout << "Issue 1255 Evolution Incense continuation tests passed.\n";
  return 0;
}
