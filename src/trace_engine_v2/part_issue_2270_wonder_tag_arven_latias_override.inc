  bool wonder_tag_arven_held_payload_route_available() const {
    if (wonder_tag_arven_held_payload_route_available_issue2270_original()) {
      return true;
    }

    // #648's original Wonder Tag -> Arven -> one-discard Item bridge rejected every
    // state with a missing Active-position axis. Quick Ball can legally solve that
    // axis at the same time as its held Dragon cost solves strict-JIT: search Latias
    // ex, Bench it, then Skyliner gives the Basic Active no Retreat Cost.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Hisuian Goodra VSTAR payload witness: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Bench, Ability, Supporter, Item, discard, search, and Retreat procedure:
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and earliest-complete-route contracts:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Original confirmed route family: https://github.com/FlareZ123/pokemon-sims/issues/648
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
    if (!prizes_known() || item_locked() || !can_play_payload_this_turn() ||
        !need_payload() || need_regi() || need_vstar() || need_energy() ||
        !need_active_vstar() || hand_count(Card::Arven) > 0 ||
        deck_count_after_search_started(Card::Arven) == 0 ||
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        !state_.active || !is_basic(state_.active->card) || state_.retreat_used ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        !ability_available_for_pokemon(Card::LatiasEx) || in_play(Card::LatiasEx) ||
        deck_count_after_search_started(Card::QuickBall) == 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0) {
      return false;
    }

    const bool benched_ggf_vstar = std::any_of(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoVstar && pokemon.grass >= 2 &&
                 pokemon.fire >= 1;
        });
    if (!benched_ggf_vstar) return false;

    // Before Wonder Tag resolves, one Bench slot is needed for Tapu Lele-GX and a
    // second for Latias ex. While Wonder Tag is resolving, Tapu is already in play,
    // so only the Latias slot must remain. This keeps the same preflight valid on
    // both sides of the on-Bench Ability resolution.
    // Bench procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
    const int required_open_slots = in_play(Card::TapuLeleGX) ? 1 : 2;
    return bench_space() >= required_open_slots;
  }
