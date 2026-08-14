#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_steven(Engine& engine) { return engine.play_steven(); }
  static bool held_routes_complete(const Engine& engine, const std::vector<Card>& wanted,
                                   const bool next_turn_item_locked,
                                   const bool projected_vstar_available) {
    return engine.steven_held_routes_complete_next_turn(
        wanted, next_turn_item_locked, projected_vstar_available);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-978-steven-minimal-targets", sim::DciProfile::StrictJit,
                       lock, true, 3};
}

sim::State live_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0};
  state.hand = {sim::Card::StevensResolve, sim::Card::Fire,
                sim::Card::QuickBall, sim::Card::Dragapult,
                sim::Card::FieldBlower, sim::Card::LatiasEx};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::BrilliantBlender, sim::Card::TapuLeleGX,
                sim::Card::MegaDragonite, sim::Card::Grass,
                sim::Card::Fire};
  state.manual_energy_used = true;
  return state;
}

sim::State resolve(sim::State state, const sim::LockMode lock = sim::LockMode::None) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario selected = scenario(lock);
  std::mt19937_64 rng{978};
  sim::Engine engine(selected, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(sim::EngineTestAccess::play_steven(engine), "Steven should resolve in the test state.");
  return sim::EngineTestAccess::state(engine);
}

bool held_routes_complete(sim::State state, const bool next_turn_item_locked = false) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario selected = scenario();
  std::mt19937_64 rng{9781};
  sim::Engine engine(selected, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);
  return sim::EngineTestAccess::held_routes_complete(
      engine, {sim::Card::RegidragoVstar}, next_turn_item_locked, true);
}

void test_banks_free_blender_over_paid_quick_ball_route() {
  // Steven may search up to three unrestricted cards. Once the VSTAR target is
  // required, the otherwise-free Brilliant Blender target dominates spending
  // Quick Ball plus the held Dragon as the next-turn payload outlet:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced search/discard procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Connector/resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3653
  const sim::State after = resolve(live_route_state());
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Steven should search the missing VSTAR card.");
  expect(contains(after.hand, sim::Card::BrilliantBlender),
         "Steven should use the free target slot to bank Brilliant Blender.");
  expect(!contains(after.hand, sim::Card::Crispin),
         "Held Fire must prevent redundant Crispin search.");
  expect(contains(after.hand, sim::Card::QuickBall) &&
             contains(after.hand, sim::Card::Dragapult),
         "Banking Blender must preserve Quick Ball and the held Dragon payload.");
  expect(contains(after.deck, sim::Card::Crispin) &&
             !contains(after.deck, sim::Card::BrilliantBlender),
         "Crispin should remain in deck while the free Blender target moves to hand.");
}

void test_rejects_minimal_route_without_held_energy() {
  sim::State state = live_route_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::Fire),
                   state.hand.end());
  // Crispin or another Energy connector remains necessary without the held manual attachment:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/issues/978
  expect(!held_routes_complete(std::move(state)),
         "The minimal Steven route must require the held Energy.");
}

void test_rejects_minimal_route_without_held_payload() {
  sim::State state = live_route_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::Dragapult),
                   state.hand.end());
  // Quick Ball requires a discard cost, while Blender remains the direct deck payload route:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/978
  expect(!held_routes_complete(std::move(state)),
         "The minimal Steven route must require a held Dragon payload.");
}

void test_rejects_minimal_route_when_quick_ball_has_no_basic_target() {
  sim::State state = live_route_state();
  state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(), sim::is_basic),
                   state.deck.end());
  // A known targetless Quick Ball cannot be played, so its discard cost is not a live route:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/978
  expect(!held_routes_complete(std::move(state)),
         "A targetless Quick Ball must not suppress Blender.");
}

void test_rejects_minimal_route_under_next_turn_item_lock() {
  sim::State state = live_route_state();
  // Steven ends the turn, and scheduled Item lock makes Quick Ball unavailable next turn:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/978
  expect(!held_routes_complete(std::move(state), true),
         "Next-turn Item lock must reject the held Quick Ball route.");
}

}  // namespace

int main() {
  test_banks_free_blender_over_paid_quick_ball_route();
  test_rejects_minimal_route_without_held_energy();
  test_rejects_minimal_route_without_held_payload();
  test_rejects_minimal_route_when_quick_ball_has_no_basic_target();
  test_rejects_minimal_route_under_next_turn_item_lock();
  return 0;
}
