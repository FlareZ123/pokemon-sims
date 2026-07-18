#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool seen) { engine.deck_seen_ = seen; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
};

}  // namespace sim

namespace {

const sim::DeckRecipe& test_recipe() {
  // Engine stores the recipe by reference, so this owner must outlive every test Engine:
  // https://eel.is/c++draft/class.temporary#6.10
  // https://github.com/FlareZ123/pokemon-sims/issues/907
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return recipe;
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Engine make_engine() {
  static const sim::Scenario scenario{"issue-889", sim::DciProfile::StrictJit,
                                      sim::LockMode::None, false, 4};
  static std::mt19937_64 rng{889};
  return sim::Engine(scenario, test_recipe(), rng);
}

sim::State base_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Serena,
                sim::Card::MegaDragonite, sim::Card::RegidragoVstar,
                sim::Card::Fire, sim::Card::Fire, sim::Card::Fire,
                sim::Card::ProfessorBurnet, sim::Card::RoseannesBackup, sim::Card::ErikasInvitation};
  state.deck = {sim::Card::Dragapult, sim::Card::GoodraVstar,
                sim::Card::DialgaGX, sim::Card::Grass};
  return state;
}

void test_held_serena_payload_blocks_redundant_search() {
  sim::Engine engine = make_engine();
  const sim::State before = base_state();
  sim::EngineTestAccess::set_state(engine, before);
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // Mysterious Treasure discards one card and searches a Psychic or Dragon
  // Pokémon. Serena may already discard the held Mega Dragonite ex, so fetching
  // Dragapult ex would duplicate the same Apex Dragon payload axis:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/889
  if (sim::EngineTestAccess::play_mysterious_treasure(engine, false)) {
    throw std::runtime_error("Mysterious Treasure fetched a redundant second payload.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.hand != before.hand || after.deck != before.deck || after.discard != before.discard) {
    throw std::runtime_error("Rejected redundant search must not mutate the game state.");
  }
}

void test_serena_without_held_payload_keeps_search_live() {
  sim::Engine engine = make_engine();
  sim::State state = base_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite));
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // With no permitted Dragon in hand, Mysterious Treasure may fetch Dragapult ex
  // for the surviving Serena outlet:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/sv8-130
  if (!sim::EngineTestAccess::play_mysterious_treasure(engine, false)) {
    throw std::runtime_error("The missing-payload control should keep the search live.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Dragapult) ||
      !contains(after.discard, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("The control should search a Dragon payload and pay the Item cost.");
  }
}

}  // namespace

int main() {
  test_held_serena_payload_blocks_redundant_search();
  test_serena_without_held_payload_keeps_search_live();
  std::cout << "Mysterious Treasure redundant-payload tests passed\n";
}
