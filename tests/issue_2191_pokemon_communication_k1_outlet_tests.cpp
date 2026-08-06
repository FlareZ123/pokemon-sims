#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }

  static const State& state(const Engine& engine) { return engine.state_; }

  static void set_knowledge(Engine& engine, const bool prize_knowledge) {
    if (prize_knowledge) {
      engine.prizes_revealed_ = true;
    } else {
      engine.deck_seen_ = true;
    }
  }

  static bool communication_payload_outlet_after_exchange(
      Engine& engine, const Card returned, const Card fetched) {
    return engine.pokemon_communication_payload_outlet_after_exchange(
        returned, fetched, true, false);
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(std::string label)
      : scenario{std::move(label), sim::DciProfile::StrictJit,
                 sim::LockMode::None, false, 4},
        rng(2191), engine(scenario, recipe, rng) {}
};

sim::State ready_state(std::vector<sim::Card> hand,
                       std::vector<sim::Card> deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = std::move(hand);
  state.deck = std::move(deck);
  return state;
}

void require_unchanged(const sim::State& before, const sim::State& after) {
  if (after.hand != before.hand || after.deck != before.deck) {
    throw std::runtime_error(
        "Pokémon Communication outlet preflight must restore copied zones.");
  }
}

void test_mysterious_treasure_uses_exact_k1_deck() {
  for (const bool prize_knowledge : {false, true}) {
    Fixture positive(prize_knowledge ? "issue-2191-treasure-prize-k1"
                                     : "issue-2191-treasure-deck-k1");
    sim::State state = ready_state(
        {sim::Card::PokemonCommunication, sim::Card::CrobatV,
         sim::Card::MysteriousTreasure, sim::Card::Grass},
        {sim::Card::RegidragoV, sim::Card::Appletun});
    sim::EngineTestAccess::set_state(positive.engine, state);
    sim::EngineTestAccess::set_knowledge(positive.engine, prize_knowledge);

    // After Communication returns Crobat V and fetches Regidrago V, the exact K1
    // deck still contains off-recipe Appletun. Its Dragon typing is a legal later
    // Mysterious Treasure target, and surplus Grass pays that Item's discard cost:
    // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2191
    if (!sim::EngineTestAccess::communication_payload_outlet_after_exchange(
            positive.engine, sim::Card::CrobatV, sim::Card::RegidragoV)) {
      throw std::runtime_error(
          "K1 Communication preflight missed an exact-deck Appletun target.");
    }
    require_unchanged(state, sim::EngineTestAccess::state(positive.engine));

    Fixture negative(prize_knowledge ? "issue-2191-treasure-prize-no-target"
                                     : "issue-2191-treasure-deck-no-target");
    state = ready_state(
        {sim::Card::PokemonCommunication, sim::Card::CrobatV,
         sim::Card::MysteriousTreasure, sim::Card::Grass},
        {sim::Card::RegidragoV});
    sim::EngineTestAccess::set_state(negative.engine, state);
    sim::EngineTestAccess::set_knowledge(negative.engine, prize_knowledge);
    if (sim::EngineTestAccess::communication_payload_outlet_after_exchange(
            negative.engine, sim::Card::CrobatV, sim::Card::RegidragoV)) {
      throw std::runtime_error(
          "K1 Communication preflight invented a Treasure target from the recipe.");
    }
    require_unchanged(state, sim::EngineTestAccess::state(negative.engine));
  }
}

void test_quick_ball_uses_exact_k1_deck() {
  for (const bool prize_knowledge : {false, true}) {
    Fixture positive(prize_knowledge ? "issue-2191-quick-prize-k1"
                                     : "issue-2191-quick-deck-k1");
    sim::State state = ready_state(
        {sim::Card::PokemonCommunication, sim::Card::ForretressEx,
         sim::Card::QuickBall, sim::Card::Grass},
        {sim::Card::RegidragoVstar, sim::Card::MawileGX});
    sim::EngineTestAccess::set_state(positive.engine, state);
    sim::EngineTestAccess::set_knowledge(positive.engine, prize_knowledge);

    // After Communication returns Forretress ex and fetches Regidrago VSTAR, the
    // exact K1 deck still contains off-recipe Basic Mawile-GX. Quick Ball may use
    // that physical Basic target after surplus Grass pays its discard cost:
    // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mawile-GX: https://api.pokemontcg.io/v2/cards/sm11-141
    // Core search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2191
    if (!sim::EngineTestAccess::communication_payload_outlet_after_exchange(
            positive.engine, sim::Card::ForretressEx,
            sim::Card::RegidragoVstar)) {
      throw std::runtime_error(
          "K1 Communication preflight missed an exact-deck Mawile-GX target.");
    }
    require_unchanged(state, sim::EngineTestAccess::state(positive.engine));

    Fixture negative(prize_knowledge ? "issue-2191-quick-prize-no-target"
                                     : "issue-2191-quick-deck-no-target");
    state = ready_state(
        {sim::Card::PokemonCommunication, sim::Card::ForretressEx,
         sim::Card::QuickBall, sim::Card::Grass},
        {sim::Card::RegidragoVstar});
    sim::EngineTestAccess::set_state(negative.engine, state);
    sim::EngineTestAccess::set_knowledge(negative.engine, prize_knowledge);
    if (sim::EngineTestAccess::communication_payload_outlet_after_exchange(
            negative.engine, sim::Card::ForretressEx,
            sim::Card::RegidragoVstar)) {
      throw std::runtime_error(
          "K1 Communication preflight invented a Quick Ball target from the recipe.");
    }
    require_unchanged(state, sim::EngineTestAccess::state(negative.engine));
  }
}

}  // namespace

int main() {
  test_mysterious_treasure_uses_exact_k1_deck();
  test_quick_ball_uses_exact_k1_deck();
  return 0;
}
