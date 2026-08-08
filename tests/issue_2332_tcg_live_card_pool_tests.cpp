#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include "../src/tcg_live_card_pool.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_live_rejects_xy_double_dragon_energy() {
  const sim::NamedDeck dde{"regidrago-dde-model",
                           sim::double_dragon_modeling_recipe()};
  std::string error;

  // Pokémon Support says XY cards are not playable in Pokémon TCG Live, and
  // Double Dragon Energy is XY—Roaring Skies 97/108:
  // https://support.pokemon.com/hc/en-us/articles/6489934466708-Pok%C3%A9mon-TCG-Live-Migration-FAQ-from-the-Pok%C3%A9mon-TCG-Online
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://github.com/FlareZ123/pokemon-sims/issues/2332
  require(!sim::validate_recipe_for_pool(
              dde, sim::CardPool::TcgLiveExpanded, &error),
          "TCG Live validation accepted XY Double Dragon Energy.");
  require(error.find("Double Dragon Energy") != std::string::npos,
          "TCG Live rejection did not identify Double Dragon Energy.");
  require(sim::deck_by_id_for_pool(
              "regidrago-dde-model", sim::CardPool::TcgLiveExpanded) == nullptr,
          "TCG Live lookup exposed the paper-only DDE model.");
}

void test_paper_expanded_preserves_dde_model() {
  const sim::NamedDeck dde{"regidrago-dde-model",
                           sim::double_dragon_modeling_recipe()};
  std::string error;

  // The supplied card corpus records xy6-97 as Expanded-legal in paper play;
  // the simulator keeps that mechanics model available only through the explicit
  // paper pool. Card text: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Scope contract: https://github.com/FlareZ123/pokemon-sims/issues/2332
  require(sim::validate_recipe_for_pool(
              dde, sim::CardPool::PaperExpanded, &error),
          "Paper Expanded validation rejected the DDE model.");
  require(sim::deck_by_id_for_pool(
              "regidrago-dde-model", sim::CardPool::PaperExpanded) != nullptr,
          "Explicit paper lookup lost the DDE model.");
}

void test_live_registry_remains_available() {
  for (const sim::NamedDeck& deck : sim::deck_registry()) {
    std::string error;
    require(sim::validate_recipe_for_pool(
                deck, sim::CardPool::TcgLiveExpanded, &error),
            "A registered Pokémon TCG Live deck failed Live card-pool validation.");
  }

  // Sun & Moon and Sword & Shield cards are playable in Pokémon TCG Live under
  // the current Pokémon Support card-pool statement. Tapu Lele-GX is the SM2
  // Guardians Rising print and Regidrago V is the SWSH12 Silver Tempest print:
  // https://support.pokemon.com/hc/en-us/articles/6489934466708-Pok%C3%A9mon-TCG-Live-Migration-FAQ-from-the-Pok%C3%A9mon-TCG-Online
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  require(sim::card_supported_in_pool(
              sim::Card::TapuLeleGX, sim::CardPool::TcgLiveExpanded),
          "Sun & Moon card was rejected by Live validation.");
  require(sim::card_supported_in_pool(
              sim::Card::RegidragoV, sim::CardPool::TcgLiveExpanded),
          "Sword & Shield card was rejected by Live validation.");
}

}  // namespace

int main() {
  try {
    test_live_rejects_xy_double_dragon_energy();
    test_paper_expanded_preserves_dde_model();
    test_live_registry_remains_available();
    std::cout << "Issue 2332 TCG Live card-pool tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
