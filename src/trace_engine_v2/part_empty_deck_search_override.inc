  bool deck_search_available() const {
    // A deck search cannot begin when there are zero cards left in the deck:
    // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
    return !state_.deck.empty();
  }

  bool bench_from_hand(const Card card, const bool resolve_entry) {
    if (!resolve_entry || deck_search_available() ||
        (card != Card::TapuLeleGX && card != Card::Oricorio)) {
      return bench_from_hand_empty_deck_original(card, resolve_entry);
    }

    // Playing the Basic Pokemon remains legal. Wonder Tag and Vital Dance each say
    // "you may search", so decline that optional search when the deck is empty:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
    const bool benched = bench_from_hand_empty_deck_original(card, false);
    if (benched) {
      trace("ABILITY DECLINED", card == Card::TapuLeleGX ? "R-TAPU-01" : "R-ORICORIO-01",
            "The optional deck search was not used because the deck was empty.");
    }
    return benched;
  }

  bool bench_oricorio_if_useful() {
    // Vital Dance cannot supply an Energy connector from an empty deck:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
    return deck_search_available() && bench_oricorio_if_useful_empty_deck_original();
  }

  bool bench_tapu_if_useful() {
    // Wonder Tag cannot supply a Supporter connector from an empty deck:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
    return deck_search_available() && bench_tapu_if_useful_empty_deck_original();
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    // Mysterious Treasure searches the deck after its discard cost, so reject the
    // play before paying that cost when the deck is empty:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return deck_search_available() &&
           play_mysterious_treasure_empty_deck_original(permit_payload);
  }

  bool play_quick_ball(const bool permit_payload) {
    // Quick Ball cannot be played for a deck search when zero cards remain:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return deck_search_available() && play_quick_ball_empty_deck_original(permit_payload);
  }

  bool play_ultra_ball(const bool permit_payload) {
    // Ultra Ball cannot spend its two-card cost when its deck search cannot begin:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return deck_search_available() && play_ultra_ball_empty_deck_original(permit_payload);
  }

  bool play_evolution_incense(const bool permit_payload = false) {
    // Evolution Incense is a search-only Item and is unplayable with an empty deck:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return deck_search_available() &&
           play_evolution_incense_empty_deck_original(permit_payload);
  }

  bool play_earthen_vessel(const bool permit_payload) {
    // Earthen Vessel cannot discard another card when the required deck search cannot begin:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return deck_search_available() && play_earthen_vessel_empty_deck_original(permit_payload);
  }

  bool play_brilliant_blender() {
    // Brilliant Blender searches the deck before discarding the selected cards:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return deck_search_available() && play_brilliant_blender_empty_deck_original();
  }

  bool use_fss() {
    // Star Alchemy is an optional deck-search VSTAR Power. Preserve the once-per-game
    // resource when the deck contains zero cards:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
    return deck_search_available() && use_fss_empty_deck_original();
  }

  bool play_crispin() {
    // Crispin has no effect without a deck to search and therefore cannot be played:
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return deck_search_available() && play_crispin_empty_deck_original();
  }

  bool play_professor_burnet() {
    // Professor Burnet is a search-only Supporter and cannot be played with an empty deck:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return deck_search_available() && play_professor_burnet_empty_deck_original();
  }

  bool play_steven() {
    // Steven's Resolve also ends the turn, so the empty-search and no-effect rulings
    // do not establish that the whole Supporter is illegal when the deck is empty:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return play_steven_empty_deck_original();
  }

  bool play_arven() {
    // Arven searches the deck for both of its optional targets, so zero cards in deck
    // makes the Supporter unplayable before its Supporter slot is consumed:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return deck_search_available() && play_arven_empty_deck_original();
  }
