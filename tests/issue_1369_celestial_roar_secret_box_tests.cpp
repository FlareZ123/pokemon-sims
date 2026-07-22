#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool guaranteed_route(const Engine& engine) {
    return engine.guaranteed_next_turn_secret_box_route_after_hold();
  }
  static bool use_celestial_roar(Engine& engine) {
    return engine.use_celestial_roar();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State guaranteed_secret_box_state() {
  sim::State state;
  state.turn = 1;
  state.supporter_used = true;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::Fire, sim::Card::GoodraVstar, sim::Card::Grass,
                sim::Card::Gladion, sim::Card::StevensResolve,
                sim::Card::SecretBox};
  state.deck = {sim::Card::Dawn, sim::Card::ForestOfVitality,
                sim::Card::Pineco, sim::Card::ForretressEx,
                sim::Card::MysteriousTreasure, sim::Card::RegidragoVstar,
                sim::Card::Grass, sim::Card::Grass, sim::Card::QuickBall,
                sim::Card::Fire, sim::Card::TapuLeleGX,
                sim::Card::ErikasInvitation};
  state.prizes = {sim::Card::HisuianHeavyBall, sim::Card::ProfessorBurnet,
                  sim::Card::TapuLeleGX, sim::Card::Fire,
                  sim::Card::QuickBall, sim::Card::Arven};
  return state;
}

void test_holds_attack_for_every_draw_guaranteed_route() {
  const sim::Scenario scenario{"issue-1369-exact",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  const sim::DeckRecipe recipe{sim::pineco_recipe()};
  std::mt19937_64 rng{1369};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, guaranteed_secret_box_state());

  // K1 proves that every possible next draw preserves Secret Box's three legal
  // costs and the Dawn, Forest, Pineco, Forretress, Treasure, VSTAR, GGF, and
  // payload axes. Celestial Roar cannot improve readiness before T2 and may
  // discard a required category, so the attack must be held:
  // Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Pineco and Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1369
  expect(sim::EngineTestAccess::guaranteed_route(engine),
         "The exact K1 Secret Box route must be guaranteed across every draw.");
  const std::size_t deck_before = sim::EngineTestAccess::state(engine).deck.size();
  const std::size_t discard_before = sim::EngineTestAccess::state(engine).discard.size();
  expect(!sim::EngineTestAccess::use_celestial_roar(engine),
         "Celestial Roar must be held for the guaranteed T2 route.");
  expect(sim::EngineTestAccess::state(engine).deck.size() == deck_before,
         "Holding must leave the known deck intact.");
  expect(sim::EngineTestAccess::state(engine).discard.size() == discard_before,
         "Holding must leave required categories out of discard.");
}

void test_attack_remains_live_when_one_draw_breaks_grass_axis() {
  const sim::Scenario scenario{"issue-1369-not-guaranteed",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  const sim::DeckRecipe recipe{sim::pineco_recipe()};
  std::mt19937_64 rng{1370};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = guaranteed_secret_box_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::Grass));
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // If drawing the final deck Grass would remove Exploding Energy's required
  // search target, the route is not guaranteed. The existing Celestial Roar
  // policy must remain available rather than overextending the hold:
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // No-future-card oracle: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  expect(!sim::EngineTestAccess::guaranteed_route(engine),
         "A route that fails for one possible draw must not be called guaranteed.");
  expect(sim::EngineTestAccess::use_celestial_roar(engine),
         "Celestial Roar must remain live outside the all-draw guarantee.");
}

void test_shell_recipe_never_uses_pineco_hold() {
  const sim::Scenario scenario{"issue-1369-shell-control",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1371};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, guaranteed_secret_box_state());

  // The route is recipe-gated. A shell fixture containing synthetic Pineco cards
  // in its zones must retain the original Celestial Roar behavior:
  // Repository deck registry: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1369
  expect(!sim::EngineTestAccess::guaranteed_route(engine),
         "The shell recipe must not enter the Pineco-specific hold.");
  expect(sim::EngineTestAccess::use_celestial_roar(engine),
         "The shell recipe must retain the original attack policy.");
}

}  // namespace

int main() {
  try {
    test_holds_attack_for_every_draw_guaranteed_route();
    test_attack_remains_live_when_one_draw_breaks_grass_axis();
    test_shell_recipe_never_uses_pineco_hold();
    std::cout << "Issue 1369 Celestial Roar Secret Box tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
