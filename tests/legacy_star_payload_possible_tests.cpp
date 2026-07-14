#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State ready_except_payload(std::vector<sim::Card> deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.deck = std::move(deck);
  return state;
}

void test_legacy_star_holds_with_public_empty_deck() {
  const sim::Scenario scenario{"legacy-star-empty-deck", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{405};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, ready_except_payload({}));

  // Legacy Star creates a current-turn payload only through its top-seven deck
  // discard. An empty deck cannot supply a Dragon, and the VSTAR Power is shared:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://www.pokemon.com/us/pokemon-tcg/rules
  expect(!sim::EngineTestAccess::use_legacy_star(engine),
         "Legacy Star should be held when the public deck size is zero.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(!after.vstar_power_used && after.deck.empty() && after.discard.empty(),
         "Holding Legacy Star must preserve the VSTAR Power and all zones.");
}

void test_legacy_star_holds_when_k1_proves_payload_absent() {
  const sim::Scenario scenario{"legacy-star-k1-payload-absent", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{406};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine,
      ready_except_payload({sim::Card::Grass, sim::Card::Fire}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // After legal deck inspection, the fixed-list complement may prove every modeled
  // Dragon payload absent. Legacy Star then has zero chance to create the missing axis:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  expect(!sim::EngineTestAccess::use_legacy_star(engine),
         "Legacy Star should be held when K1 proves no payload remains in deck.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(!after.vstar_power_used && after.deck.size() == 2U && after.discard.empty(),
         "The K1-known dead route must preserve the VSTAR Power and deck.");
}

void test_legacy_star_keeps_k0_plausible_random_route() {
  const sim::Scenario scenario{"legacy-star-k0-plausible", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{407};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine,
      ready_except_payload({sim::Card::Grass}));

  // Before legal inspection, a fixed-list payload may still be in the private deck.
  // Preserve the modeled random Legacy Star route at K0:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  expect(sim::EngineTestAccess::use_legacy_star(engine),
         "Legacy Star should remain available while a payload is still plausible at K0.");
  expect(sim::EngineTestAccess::state(engine).vstar_power_used,
         "The K0 plausible route should consume the VSTAR Power when chosen.");
}

void test_legacy_star_uses_known_present_payload() {
  const sim::Scenario scenario{"legacy-star-known-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{408};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine,
      ready_except_payload({sim::Card::MegaDragonite}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // A known Dragon payload in deck makes Legacy Star a live current-turn discard line:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  expect(sim::EngineTestAccess::use_legacy_star(engine),
         "Legacy Star should use the known-present Dragon payload route.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The known payload discarded by Legacy Star should complete the payload axis.");
}

}  // namespace

int main() {
  try {
    test_legacy_star_holds_with_public_empty_deck();
    test_legacy_star_holds_when_k1_proves_payload_absent();
    test_legacy_star_keeps_k0_plausible_random_route();
    test_legacy_star_uses_known_present_payload();
    std::cout << "Legacy Star payload feasibility tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
