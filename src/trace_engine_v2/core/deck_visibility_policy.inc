#include "deck_knowledge.inc"

  bool might_be_unseen(const Card card) const {
    // The remaining deck size is public game state. A deck with zero cards cannot
    // contain any hidden candidate, even before a legal deck or Prize inspection:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    return deck_has_cards() && might_be_unseen_empty_deck_original(card);
  }
