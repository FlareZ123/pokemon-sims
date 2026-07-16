#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool retreat_with_latias(Engine& engine) {
    return engine.retreat_to_benched_vstar_with_latias();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"blender-active-route", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{725};
  sim::Engine engine{scenario, recipe, rng};
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State stranded_complete_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass};
  return state;
}

void expect(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

void test_stranded_vstar_holds_blender() {
  Fixture fixture;
  sim::State state = stranded_complete_vstar_state();
  const std::vector<sim::Card> hand_before = state.hand;
  const std::vector<sim::Card> deck_before = state.deck;
  const std::vector<sim::Card> discard_before = state.discard;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Brilliant Blender is a one-shot ACE SPEC. Strict JIT requires the payload to
  // enter discard during the same turn that the GGF Regidrago VSTAR becomes Active.
  // No switch or free-retreat connector exists in this exact public state:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect(!sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "Blender must be held without a same-turn Active route.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.hand == hand_before && after.deck == deck_before &&
             after.discard == discard_before,
         "The rejected Blender play must preserve every card.");
}

void test_tate_route_keeps_blender_live() {
  Fixture fixture;
  sim::State state = stranded_complete_vstar_state();
  state.hand.push_back(sim::Card::TateLiza);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Tate & Liza can switch the complete Benched VSTAR Active after Blender supplies
  // the current-turn Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "A held Tate switch must keep the Blender route live.");
}

void test_guzma_route_resolves_after_blender() {
  Fixture fixture;
  sim::State state = stranded_complete_vstar_state();
  state.hand.push_back(sim::Card::Guzma);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Guzma switches the user's Active Pokémon with a Benched Pokémon after the
  // opponent-side switch. The model uses that self-switch to promote Regidrago VSTAR:
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scope
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "A held Guzma must keep the Blender route live.");
  sim::EngineTestAccess::choose_supporter(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Guzma must promote the complete Benched Regidrago VSTAR.");
  expect(after.supporter_used && contains(after.discard, sim::Card::Guzma),
         "Guzma must consume the Supporter play and enter discard.");
}

void test_latias_route_keeps_blender_live() {
  Fixture fixture;
  sim::State state = stranded_complete_vstar_state();
  state.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0,
                                     sim::Tool::None});
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Skyliner gives the Basic Active no Retreat Cost, so the same turn can promote
  // the GGF-ready Benched VSTAR after Blender supplies its payload:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "A live Latias free-retreat route must keep Blender playable.");
  expect(sim::EngineTestAccess::retreat_with_latias(fixture.engine),
         "The Latias route must promote the complete VSTAR.");
}

void test_attached_fss_tate_route_keeps_blender_live() {
  Fixture fixture;
  sim::State state = stranded_complete_vstar_state();
  state.bench.front().tool = sim::Tool::ForestSealStone;
  state.deck.push_back(sim::Card::TateLiza);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Forest Seal Stone grants Star Alchemy to its Pokémon V, which can search Tate
  // & Liza for the same-turn promotion after Blender:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "An attached live Forest Seal Stone route must keep Blender playable.");
}

void test_held_fss_tate_route_keeps_blender_live() {
  Fixture fixture;
  sim::State state = stranded_complete_vstar_state();
  state.hand.push_back(sim::Card::ForestSealStone);
  state.deck.push_back(sim::Card::TateLiza);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Pokémon Tool cards may be attached during the turn through Item lock, and Star
  // Alchemy can then search Tate & Liza for promotion:
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm7-148
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "A held attachable Forest Seal Stone route must keep Blender playable.");
}

void test_turo_route_keeps_blender_live() {
  Fixture fixture;
  sim::State state = stranded_complete_vstar_state();
  state.hand.push_back(sim::Card::ProfessorTuro);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Professor Turo can return the Basic Active, forcing promotion of the complete
  // Benched Regidrago VSTAR after Blender establishes the payload:
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "A held Professor Turo promotion route must keep Blender playable.");
  sim::EngineTestAccess::choose_supporter(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Professor Turo must promote the complete Benched VSTAR.");
}

void test_direct_active_evolution_keeps_blender_live() {
  Fixture fixture;
  sim::State state = stranded_complete_vstar_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None};
  state.hand.push_back(sim::Card::RegidragoVstar);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // A prior-turn Active Regidrago V may evolve into Regidrago VSTAR during this turn,
  // so Blender's same-turn payload is not stranded by the separate Benched VSTAR:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-164
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "A legal direct Active evolution must keep Blender playable.");
}

}  // namespace

int main() {
  try {
    test_stranded_vstar_holds_blender();
    test_tate_route_keeps_blender_live();
    test_guzma_route_resolves_after_blender();
    test_latias_route_keeps_blender_live();
    test_attached_fss_tate_route_keeps_blender_live();
    test_held_fss_tate_route_keeps_blender_live();
    test_turo_route_keeps_blender_live();
    test_direct_active_evolution_keeps_blender_live();
    std::cout << "blender active-route regressions passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
