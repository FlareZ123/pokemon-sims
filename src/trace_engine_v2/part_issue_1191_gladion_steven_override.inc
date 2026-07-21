  bool known_t2_steven_crispin_latias_route_available() const {
    if (!prizes_known() || state_.turn != 1 || scenario_.going_first ||
        scenario_.dci != DciProfile::NoDiscardControl ||
        scenario_.max_turn < 2 || !supporter_allowed() ||
        hand_count(Card::Gladion) == 0 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::Crispin) > 0 || hand_count(Card::LatiasEx) > 0 ||
        prize_count_after_reveal(Card::Oricorio) == 0 ||
        !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) || !payload_ready() ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    // Steven's Resolve supplies the three unresolved card axes on T1. On T2,
    // Crispin plus the legal manual attachment add one Grass and one Fire,
    // ordinary evolution supplies Regidrago VSTAR, and Skyliner gives the Basic
    // Active no Retreat Cost. The no-discard-control payload is already banked:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/1191
    return std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn <= state_.turn && pokemon.grass == 1 &&
              pokemon.fire == 0;
        });
  }

  bool play_gladion_issue1204_original() {
    if (!known_t2_steven_crispin_latias_route_available()) {
      return play_gladion_issue1191_original();
    }

    // Recovering prized Oricorio spends the only T1 Supporter and advances only
    // the Energy axis. Preserve Gladion and let the held Steven package complete
    // Evolution, Energy, and Active position on the earliest legal ready turn:
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/sv6-130
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://github.com/FlareZ123/pokemon-sims/issues/1191
    if (trace_ != nullptr) {
      trace_->add_policy_once(
          state_, "issue-1191-steven-before-prized-oricorio", state_.turn,
          "HOLD SUPPORTER", "R-GLADION-01; P-COMPRESS-01; P-KNOWLEDGE-01",
          "Retained Gladion because held Steven's Resolve has the complete T2 VSTAR, Crispin, and Latias package.");
    }
    return false;
  }

#include "part_issue_1204_gladion_treasure_override.inc"
