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
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static Card fss_target(const Engine& engine) { return engine.fss_target_after_search_started(); }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-976-fss-direct-crispin", sim::DciProfile::StrictJit,
                       lock, true, 3};
}

sim::State live_route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 2, 1, 0,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::Dragapult, sim::Card::Grass,
                sim::Card::MysteriousTreasure, sim::Card::RegidragoVstar,
                sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Crispin, sim::Card::BrilliantBlender,
                sim::Card::Grass, sim::Card::Fire, sim::Card::TapuLeleGX,
                sim::Card::MegaDragonite};
  return state;
}

sim::Card target_for(sim::State state, const sim::LockMode lock = sim::LockMode::None) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{976};
  sim::Engine engine(scenario(lock), recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);
  return sim::EngineTestAccess::fss_target(engine);
}

void test_prefers_direct_crispin_when_held_treasure_supplies_payload() {
  // Crispin plus the unused manual attachment completes GGF, while Mysterious
  // Treasure pays its printed cost with Dragapult ex and keeps a legal search target:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/976
  expect(target_for(live_route_state()) == sim::Card::Crispin,
         "Star Alchemy should take direct Crispin when the held Item supplies strict-JIT payload.");
}

void test_preserves_blender_when_one_discard_item_has_no_legal_target() {
  sim::State state = live_route_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::MysteriousTreasure),
                   state.hand.end());
  state.hand.push_back(sim::Card::QuickBall);
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::TapuLeleGX),
                   state.deck.end());
  // Quick Ball cannot be played when K1 proves the deck has no Basic Pokémon.
  // The remaining Stage 2 Dragon stays a live Brilliant Blender payload:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/976
  expect(target_for(std::move(state)) == sim::Card::BrilliantBlender,
         "A dead Quick Ball search must not suppress Blender.");
}

void test_preserves_blender_without_held_payload() {
  sim::State state = live_route_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::Dragapult),
                   state.hand.end());
  // Mysterious Treasure needs a card discarded from hand as its cost:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/976
  expect(target_for(std::move(state)) == sim::Card::BrilliantBlender,
         "Blender must remain the payload route without a held Dragon cost.");
}

void test_preserves_blender_without_deck_crispin() {
  sim::State state = live_route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Crispin),
                   state.deck.end());
  // Star Alchemy can only take a card that remains in the inspected deck:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/issues/976
  expect(target_for(std::move(state)) == sim::Card::BrilliantBlender,
         "The preference must require a deck-resident Crispin.");
}

void test_preserves_blender_after_supporter_is_used() {
  sim::State state = live_route_state();
  state.supporter_used = true;
  // Only one Supporter may be played each turn:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/976
  expect(target_for(std::move(state)) == sim::Card::BrilliantBlender,
         "An unusable Crispin must not displace Blender.");
}

void test_does_not_select_crispin_under_item_lock() {
  sim::State state = live_route_state();
  state.deck.push_back(sim::Card::ProfessorBurnet);
  // Item lock disables the held Mysterious Treasure route, while Professor Burnet
  // remains a Supporter payload bridge:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/976
  expect(target_for(std::move(state), sim::LockMode::FullItem) ==
             sim::Card::ProfessorBurnet,
         "Item lock must preserve the live Supporter payload bridge.");
}

}  // namespace

int main() {
  test_prefers_direct_crispin_when_held_treasure_supplies_payload();
  test_preserves_blender_when_one_discard_item_has_no_legal_target();
  test_preserves_blender_without_held_payload();
  test_preserves_blender_without_deck_crispin();
  test_preserves_blender_after_supporter_is_used();
  test_does_not_select_crispin_under_item_lock();
  return 0;
}
