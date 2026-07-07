#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Engine make_engine(std::mt19937_64& rng, const sim::Scenario& scenario,
                        const sim::DeckRecipe& recipe) {
  return sim::Engine(scenario, recipe, rng);
}

void test_deck_only_and_non_discard_cards_do_not_justify_payload_fetch() {
  const sim::Scenario scenario{"invalid-payload-outlets", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};

  for (const sim::Card non_hand_outlet : {sim::Card::EvolutionIncense,
                                          sim::Card::BrilliantBlender,
                                          sim::Card::ProfessorBurnet}) {
    std::mt19937_64 rng{91U + static_cast<unsigned int>(non_hand_outlet)};
    sim::Engine engine = make_engine(rng, scenario, recipe);
    sim::State& state = sim::EngineTestAccess::state(engine);
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
    state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dipplin, non_hand_outlet};
    state.deck = {sim::Card::MegaDragonite};

    // Mysterious Treasure can search a Dragon Pokémon after discarding from hand:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // Evolution Incense has no discard effect, and Brilliant Blender and Professor Burnet
    // discard selected deck cards rather than the Dragon Treasure would fetch to hand:
    // https://api.pokemontcg.io/v2/cards/swsh1-163 https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    assert(!sim::EngineTestAccess::play_mysterious_treasure(engine, true));
    assert(contains(state.hand, sim::Card::MysteriousTreasure));
    assert(contains(state.deck, sim::Card::MegaDragonite));
  }
}

void test_spent_supporter_slot_disables_serena_payload_continuation() {
  const sim::Scenario scenario{"spent-serena-payload-outlet", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{95};
  sim::Engine engine = make_engine(rng, scenario, recipe);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.supporter_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dipplin, sim::Card::Serena};
  state.deck = {sim::Card::MegaDragonite};

  // Serena may discard from hand, but it is a Supporter and cannot be played after
  // this turn's Supporter play has been used: https://api.pokemontcg.io/v2/cards/swsh12-164 https://www.pokemon.com/us/pokemon-tcg/rules
  assert(!sim::EngineTestAccess::play_mysterious_treasure(engine, true));
  assert(contains(state.hand, sim::Card::MysteriousTreasure));
  assert(contains(state.deck, sim::Card::MegaDragonite));
}

}  // namespace

int main() {
  test_deck_only_and_non_discard_cards_do_not_justify_payload_fetch();
  test_spent_supporter_slot_disables_serena_payload_continuation();
  return 0;
}
