#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

}  // namespace

int main() {
  {
    sim::Scenario scenario{"arven-tool-during-item-lock", sim::DciProfile::StrictJit,
                           sim::LockMode::FullItem, false, 4};
    sim::DeckRecipe recipe{sim::baseline_recipe()};
    std::mt19937_64 rng{321};
    sim::Engine engine{scenario, recipe, rng};
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
    state.hand = {sim::Card::Arven};
    state.deck = {sim::Card::ForestSealStone, sim::Card::RegidragoVstar};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // Arven is a Supporter, so Item lock does not prevent playing it: https://www.pokemon.com/us/pokemon-tcg/rules
    // Pokémon Tools are separate from Item cards after the Tool erratum, so Arven may still find Forest Seal Stone: https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    // Forest Seal Stone is the Tool Arven can retrieve for the Regidrago V route: https://api.pokemontcg.io/v2/cards/swsh12-156
    if (!sim::EngineTestAccess::play_arven(engine)) {
      throw std::runtime_error("Arven should be playable during Item lock when Forest Seal Stone is a live Tool route.");
    }

    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!contains(after.hand, sim::Card::ForestSealStone) || !contains(after.discard, sim::Card::Arven)) {
      throw std::runtime_error("Arven should retrieve Forest Seal Stone and discard itself as the Supporter.");
    }
  }

  {
    sim::Scenario scenario{"arven-dead-item-during-item-lock", sim::DciProfile::StrictJit,
                           sim::LockMode::FullItem, false, 4};
    sim::DeckRecipe recipe{sim::baseline_recipe()};
    std::mt19937_64 rng{322};
    sim::Engine engine{scenario, recipe, rng};
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None};
    state.hand = {sim::Card::Arven};
    state.deck = {sim::Card::QuickBall, sim::Card::RegidragoV};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // Item lock prevents playing Quick Ball, while no Tool route exists here: https://www.pokemon.com/us/pokemon-tcg/rules
    if (sim::EngineTestAccess::play_arven(engine)) {
      throw std::runtime_error("Arven should be held during Item lock when it can find only an unplayable Item route.");
    }
  }

  {
    sim::Scenario scenario{"known-prizes-hold-gladion-for-arven-tool", sim::DciProfile::StrictJit,
                           sim::LockMode::FullItem, false, 4};
    sim::DeckRecipe recipe{sim::baseline_recipe()};
    std::mt19937_64 rng{323};
    sim::Engine engine{scenario, recipe, rng};
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
    state.hand = {sim::Card::HisuianHeavyBall, sim::Card::Gladion, sim::Card::Arven};
    state.deck = {sim::Card::ForestSealStone, sim::Card::RegidragoVstar};
    state.prizes = {sim::Card::MegaDragonite, sim::Card::MegaDragonite, sim::Card::Dragapult,
                    sim::Card::Dragapult, sim::Card::GoodraVstar, sim::Card::GoodraVstar};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // Heavy Ball legally reveals the Prize cards, so the policy can distinguish a deck-resident VSTAR from a prized one: https://api.pokemontcg.io/v2/cards/swsh10-146
    if (!sim::EngineTestAccess::play_heavy_ball(engine)) {
      throw std::runtime_error("Heavy Ball should reveal the non-Basic Prize set before Gladion is evaluated.");
    }

    // Gladion should not consume the Supporter slot when Arven can retrieve the live Forest Seal Stone route through Item lock: https://api.pokemontcg.io/v2/cards/sm4-95
    if (sim::EngineTestAccess::play_gladion(engine)) {
      throw std::runtime_error("Gladion should be held for the live Arven-to-Forest-Seal-Stone route.");
    }
  }

  return 0;
}
