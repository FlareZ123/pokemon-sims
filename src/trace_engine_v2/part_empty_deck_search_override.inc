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

  bool ultra_ball_payload_continuation_survives_search(const bool permit_payload) const {
    if (state_.deck.size() != 1U || !need_payload()) return true;

    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (can_jit_discard) return true;

    // Preserve ordinary Ultra Ball searches. The dead route exists only when the
    // final deck card is sought as the Dragon payload and no other target axis makes
    // the current Ultra Ball play useful:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const bool has_non_payload_target =
        (need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV)) ||
        (need_vstar() && hand_count(Card::RegidragoVstar) == 0 &&
         might_be_unseen(Card::RegidragoVstar)) ||
        (need_active_vstar() && bench_space() > 0 &&
         ability_available_for_pokemon(Card::LatiasEx) && !in_play(Card::LatiasEx) &&
         hand_count(Card::LatiasEx) == 0 && might_be_unseen(Card::LatiasEx)) ||
        (need_energy() && hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) &&
         bench_space() > 0 && needs_oricorio_connector() &&
         might_be_unseen(Card::Oricorio)) ||
        (hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
         bench_space() > 0 && needs_tapu_connector() && might_be_unseen(Card::TapuLeleGX));
    if (has_non_payload_target) return true;

    const bool payload_might_be_searchable = might_be_unseen(Card::MegaDragonite) ||
        might_be_unseen(Card::Dragapult) || might_be_unseen(Card::GoodraVstar) ||
        might_be_unseen(Card::DialgaGX);
    if (!payload_might_be_searchable) return true;

    // Mysterious Treasure, Quick Ball, Earthen Vessel, and another Ultra Ball all
    // require a deck search after discarding the fetched payload. If the first Ultra
    // Ball removes the final deck card, those Items cannot begin their search and
    // cannot serve as the promised strict-JIT outlet:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    const bool search_item_outlet = hand_count(Card::MysteriousTreasure) > 0 ||
        hand_count(Card::QuickBall) > 0 || hand_count(Card::EarthenVessel) > 0 ||
        hand_count(Card::UltraBall) > 1;
    // Serena's draw mode discards from hand and does not require a deck search, so it
    // remains a live outlet after Ultra Ball removes the last deck card:
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    const bool serena_outlet = supporter_allowed() && hand_count(Card::Serena) > 0;
    return !search_item_outlet || serena_outlet;
  }

  bool play_ultra_ball(const bool permit_payload) {
    // Ultra Ball cannot spend its two-card cost when its deck search cannot begin:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    return deck_search_available() &&
           ultra_ball_payload_continuation_survives_search(permit_payload) &&
           play_ultra_ball_empty_deck_original(permit_payload);
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
