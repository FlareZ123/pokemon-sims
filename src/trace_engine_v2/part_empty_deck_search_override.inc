  bool deck_search_available() const {
    // A deck search cannot begin when there are zero cards left in the deck:
    // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
    return !state_.deck.empty();
  }

  bool earthen_vessel_has_legal_target() const {
    // Earthen Vessel searches for Basic Energy. At K0, fixed-list copies remain
    // plausible; at K1, at least one Grass or Fire must actually remain in deck:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    return might_be_unseen(Card::Grass) || might_be_unseen(Card::Fire);
  }

  bool ultra_ball_has_legal_target() const {
    // Ultra Ball searches for any Pokémon. K0 keeps fixed-list Pokémon plausible;
    // K1 requires at least one modeled Pokémon to remain in the inspected deck:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    return !deck_seen_ || std::any_of(state_.deck.begin(), state_.deck.end(), is_pokemon);
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

  bool ultra_ball_payload_search_would_strand_final_card(const bool permit_payload) {
    if (!deck_seen_ || state_.deck.size() != 1U || !need_payload()) return false;

    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (can_jit_discard) return false;

    // Preserve ordinary Ultra Ball searches. The final-card guard applies only when
    // the sole inspected Pokémon is the Dragon payload and no other setup axis makes
    // the current search useful:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
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
    if (has_non_payload_target || !is_payload(state_.deck.front())) return false;

    // Every search-based outlet becomes unplayable after the first Ultra Ball removes
    // the final deck card. Serena remains live because its discard-and-draw mode does
    // not require a second deck search:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    const bool later_search_item_outlet = hand_count(Card::MysteriousTreasure) > 0 ||
        hand_count(Card::QuickBall) > 0 || hand_count(Card::EarthenVessel) > 0 ||
        second_ultra_ball_can_discard_fetched_payload();
    const bool serena_outlet = supporter_allowed() && hand_count(Card::Serena) > 0;
    return later_search_item_outlet && !serena_outlet;
  }

  bool play_ultra_ball(const bool permit_payload) {
    // Ultra Ball cannot pay its two-card requirement when K1 proves that no Pokémon
    // target exists, or when fetching the final Dragon would make its promised
    // search-based discard outlet unplayable:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    return deck_search_available() && ultra_ball_has_legal_target() &&
           !ultra_ball_payload_search_would_strand_final_card(permit_payload) &&
           play_ultra_ball_empty_deck_original(permit_payload);
  }

  bool play_evolution_incense(const bool permit_payload = false) {
    // Evolution Incense is a search-only Item and is unplayable with an empty deck:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    if (!deck_search_available()) return false;

    // Evolution Incense can search only Evolution Pokemon. Preserve it after K1
    // proves every modeled Evolution payload absent unless a live Regidrago VSTAR
    // axis independently justifies the search:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv6-130
    // https://api.pokemontcg.io/v2/cards/swsh11-136
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    const bool vstar_axis_searchable = need_vstar() &&
        hand_count(Card::RegidragoVstar) == 0 && might_be_unseen(Card::RegidragoVstar);
    const bool evolution_payload_searchable = might_be_unseen(Card::MegaDragonite) ||
        might_be_unseen(Card::Dragapult) || might_be_unseen(Card::GoodraVstar);
    const bool payload_axis_requested = permit_payload && can_play_payload_this_turn() &&
        need_payload();
    if (!vstar_axis_searchable && payload_axis_requested &&
        !evolution_payload_searchable) {
      return false;
    }

    const bool later_search_item_outlet = hand_count(Card::MysteriousTreasure) > 0 ||
        hand_count(Card::QuickBall) > 0 || hand_count(Card::EarthenVessel) > 0 ||
        ultra_ball_can_discard_fetched_incense_payload();
    const bool serena_outlet = supporter_allowed() && hand_count(Card::Serena) > 0;
    // When the searched Dragon is the final deck card, every later search Item is
    // unplayable and cannot supply its planned discard cost. Preserve Incense when
    // payload is its sole live axis; Serena remains a legal non-search discard outlet:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool payload_search_would_strand_final_card = !vstar_axis_searchable &&
        payload_axis_requested && state_.deck.size() == 1U &&
        later_search_item_outlet && !serena_outlet;
    if (payload_search_would_strand_final_card) return false;

    return play_evolution_incense_empty_deck_original(permit_payload);
  }

  bool play_earthen_vessel(const bool permit_payload) {
    // Earthen Vessel cannot pay its discard requirement when K1 proves that its
    // required Basic Energy search has no legal target:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    return deck_search_available() && earthen_vessel_has_legal_target() &&
           play_earthen_vessel_empty_deck_original(permit_payload);
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
