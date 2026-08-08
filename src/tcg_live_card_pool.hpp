#pragma once

namespace sim {

enum class CardPool : std::uint8_t {
  TcgLiveExpanded,
  PaperExpanded,
};

constexpr bool card_supported_in_pool(const Card card, const CardPool pool) {
  if (pool == CardPool::PaperExpanded) return true;

  // Pokémon Support currently says Sun & Moon and Sword & Shield cards are
  // playable in Pokémon TCG Live, while XY and Black & White cards are not
  // playable until future Live updates:
  // https://support.pokemon.com/hc/en-us/articles/6489934466708-Pok%C3%A9mon-TCG-Live-Migration-FAQ-from-the-Pok%C3%A9mon-TCG-Online
  // Double Dragon Energy is XY—Roaring Skies 97/108, so its paper Expanded
  // legality cannot be used as Pokémon TCG Live card-pool legality:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Confirmed scope bug: https://github.com/FlareZ123/pokemon-sims/issues/2332
  return card != Card::DoubleDragonEnergy;
}

inline bool validate_recipe_for_pool(const NamedDeck& deck, const CardPool pool,
                                     std::string* error = nullptr) {
  if (!validate_recipe(deck, error)) return false;
  for (const auto& [card, copies] : deck.recipe) {
    if (copies <= 0 || card_supported_in_pool(card, pool)) continue;
    if (error != nullptr) {
      *error = deck.id + ": " + std::string(name(card)) +
          " is unavailable in the Pokémon TCG Live Expanded card pool";
    }
    return false;
  }
  return true;
}

inline const NamedDeck* deck_by_id_for_pool(const std::string_view id,
                                             const CardPool pool) {
  const NamedDeck* deck = deck_by_id(id);
  if (deck == nullptr) return nullptr;
  std::string error;
  return validate_recipe_for_pool(*deck, pool, &error) ? deck : nullptr;
}

}  // namespace sim
