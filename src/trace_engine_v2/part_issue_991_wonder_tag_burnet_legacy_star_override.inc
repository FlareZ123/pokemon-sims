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
        // Semantic Apex payment includes a legally attached DDE plus either Basic:
        // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://github.com/FlareZ123/pokemon-sims/issues/2418
        pays_apex_energy_cost(*state_.active);

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

    return std::any_of(state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
      // The Turo shortcut owns the Active-position axis when the Benched VSTAR
      // semantically pays Apex Dragon, including DDE + either Basic Energy:
      // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/issues/2418
      return pokemon.card == Card::RegidragoVstar && pays_apex_energy_cost(pokemon);
    });
  }

bool issue_1514_hold_tapu_until_public_search() const {
  if (deck_seen_ || item_locked() || !supporter_allowed() ||
      hand_count(Card::TapuLeleGX) != 1 ||
      hand_count(Card::Crispin) == 0 || !need_regi() || !need_energy() ||
      bench_space() <= 0 ||
      !ability_available_for_pokemon(Card::TapuLeleGX) ||
      hand_count(Card::QuickBall) == 0 ||
      !might_be_unseen(Card::RegidragoV)) {
    return false;
  }

  // Quick Ball may be ranked from public K0 facts: it has a legal ordinary DCI
  // cost and a Regidrago V may remain unseen. Hold the physical Tapu Lele-GX until
  // that real Item resolves and establishes K1, then re-run the existing Wonder
  // Tag policy with legally known deck and Prize identities. This preflight never
  // executes a shadow search or inspects the hidden deck or Prizes:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Core Item, search, Bench, Ability, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K0/K1 and future-card-oracle contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // DCI and connector-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1514
  return choose_discard(false, true, true, Card::QuickBall).has_value();
}

bool bench_tapu_if_useful() {
  if (issue_1514_hold_tapu_until_public_search()) {
    trace_tapu_hold_once(
        "issue-1514-k0-public-search-first",
        "R-QB-01; R-TAPU-01; R-CRISPIN-01; P-CONNECTOR-01; P-KNOWLEDGE-01; P-DCI-01",
        "Retained Tapu at K0 until a legal Quick Ball search establishes K1 and the duplicate-held-Crispin Wonder Tag can be evaluated from public information.");
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
