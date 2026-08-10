  bool deck_has_cards() const {
    return !state_.deck.empty();
  }

#include "public_card_inventory.inc"

  int known_deck_copy_count(const Card card) const {
    // Exact deck membership is actionable only after a legal deck inspection (K1):
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return deck_seen_ ? deck_count_after_search_started(card) : 0;
  }

  bool known_deck_contains(const Card card) const {
    return known_deck_copy_count(card) > 0;
  }

  bool has_known_hand_or_deck_copy(const Card card) const {
    return hand_count(card) > 0 || known_deck_contains(card);
  }
