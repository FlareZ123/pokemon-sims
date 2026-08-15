#include "../guzma_hala_policy.inc"

  bool issue_3040_play_turo_regidrago_staging_route() {
    if (!issue_3040_professor_turo_is_unique_active_escape() ||
        !strict_payload_timing() || !state_.active ||
        !is_basic(state_.active->card) || state_.active->tool != Tool::None ||
        need_regi() || !need_vstar() || need_energy() ||
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0) {
      return false;
    }

    const auto promoted = std::find_if(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
                 pokemon.entered_turn < state_.turn &&
                 pays_apex_energy_cost(pokemon);
        });
    if (promoted == state_.bench.end()) return false;

    // K1 has already established that Latias ex cannot supply the free-Retreat
    // connector and that a Regidrago VSTAR remains in the deck. The zero-resource
    // Basic Active cannot pay its printed Retreat Cost, while the prior-turn
    // Benched Regidrago V already pays GGF. Turo can therefore advance the public
    // Active-position axis now, during an otherwise unused Supporter action, without
    // consuming the later VSTAR search or the protected Dragon needed for a future
    // same-ready-turn JIT payload event. The staging turn does not require a current-
    // turn Dragon discard because strict JIT credits that discard only on the later
    // turn that actually reaches readiness.
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Active replacement, Retreat, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Advanced turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1, strict-JIT, and resource-priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
    const Pokemon returned = *state_.active;
    remove_one(state_.hand, Card::ProfessorTuro);
    state_.discard.push_back(Card::ProfessorTuro);
    state_.supporter_used = true;
    state_.hand.push_back(returned.card);
    state_.active = *promoted;
    state_.bench.erase(promoted);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-DCI-01",
          "Professor Turo returned the zero-resource Basic Active and promoted the powered prior-turn Regidrago V for its observable VSTAR staging route.");
    return true;
  }

  bool tate_celestial_roar_preservation_route_ready() const {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 || first_turn_going_first() ||
        !state_.active || state_.active->card == Card::RegidragoV ||
        state_.active->card == Card::RegidragoVstar ||
        can_free_retreat_with_latias()) {
      return false;
    }

    // Tate & Liza may switch the Active with any Benched Pokemon. A Basic played
    // this turn cannot evolve until the next turn, while Celestial Roar costs [C].
    // Restrict this selector to that exact temporal state so it cannot preempt an
    // immediately legal prior-turn Regidrago V -> VSTAR evolution route.
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Regidrago V / Celestial Roar [C]: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Evolution, Supporter, switching, attack, and turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Connector-domination and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3961
    return std::any_of(state_.bench.begin(), state_.bench.end(),
                       [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV &&
             pokemon.entered_turn == state_.turn &&
             total_energy_units(pokemon) >= 1 &&
             !pays_apex_energy_cost(pokemon);
    });
  }

  void choose_supporter_with_turo_staging_fallback() {
    if (supporter_allowed() && issue_3040_play_turo_regidrago_staging_route()) return;
    // G&H uses the normal selector path; its policy retains Supporter/DCI/JIT gates.
    // https://api.pokemontcg.io/v2/cards/sm12-229
    if (supporter_allowed() && play_guzma_hala()) return;
    if (tate_celestial_roar_preservation_route_ready() && play_tate_switch()) return;
    choose_supporter_issue1070_original();
  }

// Keep the issue-1070 selector as the single downstream owner and route only its
// final fallback through the confirmed Turo staging and Tate mode-selection policy.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
// Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
// Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
// Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
// Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/3040 https://github.com/FlareZ123/pokemon-sims/issues/3961
#define choose_supporter_issue1070_original choose_supporter_with_turo_staging_fallback
#include "../../part_issue_1070_tate_after_vstar_search_original.inc"
#undef choose_supporter_issue1070_original
