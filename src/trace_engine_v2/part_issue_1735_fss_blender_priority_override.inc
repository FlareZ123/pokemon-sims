  bool fss_blender_bank_with_complete_manual_schedule_available() const {
    // A resolved deck search or complete Prize inspection establishes the same
    // fixed-list K1. This route reads exact remaining-deck inventory, so it must
    // use the repository-wide knowledge predicate rather than one provenance:
    // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2067
    if (state_.turn != 1 || scenario_.going_first ||
        scenario_.dci != DciProfile::NoDiscardControl ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !prizes_known() || supporter_allowed() || !state_.manual_energy_used ||
        item_locked() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::RegidragoVstar) == 0 ||
        !need_payload() || !can_play_payload_this_turn() ||
        !payload_might_be_in_deck() ||
        deck_count_after_search_started(Card::BrilliantBlender) == 0) {
      return false;
    }

    const int remaining_grass = grass_needed();
    const int remaining_fire = fire_needed();
    const bool complete_two_turn_manual_schedule =
        remaining_grass + remaining_fire == 2 &&
        hand_count(Card::Grass) >= remaining_grass &&
        hand_count(Card::Fire) >= remaining_fire;
    // No-discard-control permits Brilliant Blender to bank the Dragon payload now.
    // A held Regidrago VSTAR plus the two visible future manual attachments then
    // guarantees readiness by T3, while Steven's Resolve would end a later turn
    // without advancing any unresolved axis in the current public state:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // One manual Energy attachment per turn and evolution timing:
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // No-discard-control and earliest-route policy:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1735
    return complete_two_turn_manual_schedule;
  }

  Card fss_target_after_search_started() const {
    const Card original = fss_target_after_search_started_issue1735_original();
    if (original == Card::StevensResolve &&
        fss_blender_bank_with_complete_manual_schedule_available()) {
      return Card::BrilliantBlender;
    }
    return original;
  }

#include "part_issue_1749_redundant_regi_override.inc"
