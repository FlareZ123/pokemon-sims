    if (!supporter_allowed() || hand_count(Card::Crispin) == 0 || !need_energy() || target_regi() == nullptr) return false;
    const bool deck_has_grass = deck_count_after_search_started(Card::Grass) > 0;
    const bool deck_has_fire = deck_count_after_search_started(Card::Fire) > 0;
    const bool relevant_energy = (grass_needed() > 0 && deck_has_grass) ||
                                 (fire_needed() > 0 && deck_has_fire);
    if (deck_seen_ && !relevant_energy) return false;
    if (!deck_seen_ && !might_be_unseen(Card::Grass) && !might_be_unseen(Card::Fire)) return false;

    remove_one(state_.hand, Card::Crispin);
    state_.discard.push_back(Card::Crispin);
    state_.supporter_used = true;
    record_deck_search_knowledge("Crispin");

