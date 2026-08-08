#define main pokemon_sims_paper_expanded_main
#include "regidrago_sim.cpp"
#undef main

#include "tcg_live_card_pool.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

int main(int argc, char** argv) {
  bool paper_expanded_model = false;
  std::vector<char*> forwarded;
  forwarded.reserve(static_cast<std::size_t>(argc));
  forwarded.push_back(argv[0]);

  for (int i = 1; i < argc; ++i) {
    if (std::string_view(argv[i]) == "--paper-expanded-model") {
      paper_expanded_model = true;
      continue;
    }
    forwarded.push_back(argv[i]);
  }

  std::string_view selected_deck;
  for (std::size_t i = 1; i + 1 < forwarded.size(); ++i) {
    if (std::string_view(forwarded[i]) == "--deck") {
      selected_deck = forwarded[i + 1];
      break;
    }
  }

  if (paper_expanded_model) {
    // Double Dragon Energy is retained only behind an explicit paper-Expanded
    // modeling opt-in because its XY print is unavailable in Pokémon TCG Live:
    // https://support.pokemon.com/hc/en-us/articles/6489934466708-Pok%C3%A9mon-TCG-Live-Migration-FAQ-from-the-Pok%C3%A9mon-TCG-Online
    // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // https://github.com/FlareZ123/pokemon-sims/issues/2332
    if (selected_deck != "regidrago-dde-model" ||
        sim::deck_by_id_for_pool(selected_deck, sim::CardPool::PaperExpanded) == nullptr) {
      std::cerr << "--paper-expanded-model requires --deck regidrago-dde-model\n";
      return 2;
    }
  } else if (!selected_deck.empty()) {
    const sim::NamedDeck* structural = sim::deck_by_id(selected_deck);
    if (structural != nullptr &&
        sim::deck_by_id_for_pool(selected_deck, sim::CardPool::TcgLiveExpanded) == nullptr) {
      std::cerr << "Deck " << selected_deck
                << " is outside the Pokémon TCG Live Expanded card pool; "
                   "use --paper-expanded-model only for explicit paper modeling.\n";
      return 2;
    }
  }

  return pokemon_sims_paper_expanded_main(
      static_cast<int>(forwarded.size()), forwarded.data());
}
