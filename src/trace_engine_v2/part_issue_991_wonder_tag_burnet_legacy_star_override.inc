#define bench_tapu_if_useful bench_tapu_if_useful_issue_1513_original
#include "part_issue_991_wonder_tag_burnet_legacy_star_override_base.inc"
#undef bench_tapu_if_useful

  bool held_turo_direct_active_promotion_route() const {
    // Professor Turo's Scenario directly removes a Basic Active and forces a
    // Benched replacement. When a complete Regidrago VSTAR is already Benched,
    // preserve Tapu Lele-GX and its one-shot Wonder Tag instead of creating a
    // longer Tapu -> Tate & Liza route for the same Active-position axis:
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter and Active-replacement procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Direct-connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1513
    const bool active_regidrago_has_ready_direct_evolution = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0 &&
        state_.active->grass >= 2 && state_.active->fire >= 1;

    // Under Item lock, Turo may solve the Active axis one turn before Burnet solves
    // the strict-JIT payload axis. Preserve the existing delayed-Burnet contract:
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Prior route contract: https://github.com/FlareZ123/pokemon-sims/issues/900
    const bool delayed_burnet_payload_route = strict_payload_timing() && item_locked() &&
        need_payload() && state_.turn < scenario_.max_turn &&
        hand_count(Card::ProfessorBurnet) > 0 && payload_might_be_in_deck();
    if (!supporter_allowed() || hand_count(Card::ProfessorTuro) == 0 || !state_.active ||
        !is_basic(state_.active->card) || active_regidrago_has_ready_direct_evolution ||
        !need_active_vstar() || need_regi() || need_vstar() || need_energy() ||
        (need_payload() && !delayed_burnet_payload_route) || can_free_retreat_with_latias() ||
        hand_count(Card::TateLiza) > 0) {
      return false;
    }

    return std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoVstar && pokemon.grass >= 2 && pokemon.fire >= 1;
    });
  }

  bool issue_1514_k0_duplicate_held_crispin_search_first_progresses() const {
    if (deck_seen_ || !supporter_allowed() ||
        hand_count(Card::TapuLeleGX) != 1 ||
        hand_count(Card::Crispin) == 0 || !need_energy() ||
        bench_space() <= 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        !might_be_unseen(Card::Crispin)) {
      return false;
    }

    std::mt19937_64 original_rng = rng_;
    Engine original_route(scenario_, recipe_, original_rng);
    original_route.state_ = state_;
    original_route.deck_seen_ = false;
    original_route.prizes_revealed_ = false;
    const int crispin_before = original_route.hand_count(Card::Crispin);
    const bool benched_tapu =
        original_route.bench_tapu_if_useful_issue_1513_original();
    const bool selected_duplicate_crispin = benched_tapu &&
        original_route.hand_count(Card::Crispin) > crispin_before;
    if (!selected_duplicate_crispin) return false;

    std::mt19937_64 search_rng = rng_;
    Engine search_route(scenario_, recipe_, search_rng);
    search_route.state_ = state_;
    search_route.deck_seen_ = false;
    search_route.prizes_revealed_ = false;
    const int regidrago_before =
        search_route.hand_count(Card::RegidragoV);
    const bool resolved_quick_ball =
        search_route.play_quick_ball_issue1209_original(
            search_route.can_play_payload_this_turn());
    const bool legal_search_first_progress = resolved_quick_ball &&
        search_route.deck_seen_ &&
        search_route.hand_count(Card::RegidragoV) > regidrago_before;

    // Wonder Tag consumes the physical Tapu Lele-GX and a Bench slot to
    // search one Supporter. At K0, Prize identities remain unavailable until
    // a legal search resolves. Preserve the duplicate-Crispin connector when
    // the public search-first or held-Crispin route already advances setup:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // One Supporter and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K0/K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Resource-preserving priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1514
    return legal_search_first_progress;
  }

  bool bench_tapu_if_useful() {
    if (issue_1514_k0_duplicate_held_crispin_search_first_progresses()) {
      trace_tapu_hold_once(
          "issue-1514-k0-duplicate-held-crispin",
          "R-TAPU-01; R-GAME-SUPPORTER; P-CONNECTOR-01; P-KNOWLEDGE-01",
          "Retained Tapu at K0 because Wonder Tag selected a duplicate held Crispin while a legal search-first or held-Crispin route already advanced setup.");
      return false;
    }
    if (held_turo_direct_active_promotion_route()) {
      // Held Professor Turo already removes the Basic Active and promotes the
      // complete Benched Regidrago VSTAR. Do not spend Tapu Lele-GX, its Bench
      // slot, Wonder Tag, and Tate & Liza on the same Active-position axis:
      // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
      // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
      // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
      // Active-replacement procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Direct-connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1513
      trace_tapu_hold_once(
          "issue-1513-held-turo-active-promotion",
          "R-TURO-01; R-TAPU-01; R-TATE-01; R-GAME-SUPPORTER; P-CONNECTOR-01",
          "Retained Tapu because held Professor Turo directly promotes the complete Benched Regidrago VSTAR.");
      return false;
    }
    return bench_tapu_if_useful_issue_1513_original();
  }
