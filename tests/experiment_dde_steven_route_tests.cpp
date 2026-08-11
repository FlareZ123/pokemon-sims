#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_steven(Engine& engine) { return engine.play_steven(); }
  static bool dde_completes_apex(Engine& engine, Pokemon pokemon) {
    return engine.attach_energy_card(pokemon, Card::DoubleDragonEnergy) &&
           engine.pays_apex_energy_cost(pokemon);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void adjust(sim::DeckRecipe& recipe, const sim::Card card, const int delta) {
  const auto found = std::find_if(
      recipe.begin(), recipe.end(), [card](const auto& entry) {
        return entry.first == card;
      });
  if (found == recipe.end()) {
    if (delta <= 0) throw std::logic_error("missing experiment card");
    recipe.push_back({card, delta});
    return;
  }
  found->second += delta;
  if (found->second < 0) throw std::logic_error("invalid experiment cut");
  if (found->second == 0) recipe.erase(found);
}

void test_steven_reserves_dde_instead_of_crispin() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  adjust(recipe, sim::Card::Grass, -2);
  adjust(recipe, sim::Card::DoubleDragonEnergy, +2);

  sim::Scenario scenario{"experiment-steven-dde",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 3};
  std::mt19937_64 rng{20260811};
  sim::Engine engine{scenario, recipe, rng};

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None, 0};
  state.hand = {sim::Card::StevensResolve};
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::DoubleDragonEnergy,
      sim::Card::Crispin,
      sim::Card::ProfessorBurnet,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::Fire,
  };
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Steven's Resolve searches any three cards, so with one Basic Energy already
  // attached it may reserve VSTAR + DDE + Burnet. DDE supplies two flexible units
  // on Dragon Pokémon, making the next-turn manual DDE attachment complete GGF;
  // Crispin is then unnecessary and the next-turn Supporter slot remains available
  // for Professor Burnet's payload search/discard:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // DDE modeling contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  if (!sim::EngineTestAccess::play_steven(engine)) {
    throw std::runtime_error("Steven DDE experiment route was not selected");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) ||
      !contains(after.hand, sim::Card::DoubleDragonEnergy) ||
      !contains(after.hand, sim::Card::ProfessorBurnet)) {
    throw std::runtime_error(
        "Steven must reserve VSTAR + DDE + Burnet in the experiment state");
  }
  if (contains(after.hand, sim::Card::Crispin) ||
      !contains(after.deck, sim::Card::Crispin)) {
    throw std::runtime_error(
        "Crispin must remain in deck when DDE is the one-card Energy connector");
  }
  if (!after.turn_ended || !contains(after.discard, sim::Card::StevensResolve)) {
    throw std::runtime_error("Steven must end the turn after resolving");
  }

  sim::Pokemon projected = *after.active;
  if (!sim::EngineTestAccess::dde_completes_apex(engine, projected)) {
    throw std::runtime_error(
        "one attached Basic plus one DDE must pay Apex Dragon's GGF cost");
  }
}

}  // namespace

int main() {
  test_steven_reserves_dde_instead_of_crispin();
  std::cout << "Steven DDE experiment route test passed.\n";
  return 0;
}
