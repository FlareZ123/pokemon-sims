#pragma once

// Canonical Engine-member owner for Forest Seal Stone holder selection and attachment.
// Keep strategic Tool-slot reservation in Engine while the printed attachment boundary
// follows Forest Seal Stone and the general Pokemon Tool procedure:
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Advanced Pokemon Tool procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
// Architecture contract: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#error "Forest Seal Stone attachment policy requires canonical card classification to be composed first"
#endif

  bool active_powerglass_tool_slot_needed() const {
    if (!state_.active || state_.active->tool != Tool::None ||
        !is_regidrago_v_line(state_.active->card)) {
      return false;
    }

    const bool powerglass_held = hand_count(Card::Powerglass) > 0;
    // Arven can search a Pokemon Tool. When the manual attachment is already spent
    // and Energy is the only missing setup axis, a Powerglass still in deck is the
    // immediate live route while Forest Seal Stone would occupy the same Tool slot:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    const bool energy_is_only_missing_axis = need_energy() && !need_regi() &&
        !need_vstar() && !need_active_vstar() && !need_payload();
    const bool arven_searches_live_powerglass = supporter_allowed() &&
        hand_count(Card::Arven) > 0 && state_.manual_energy_used &&
        energy_is_only_missing_axis && might_be_unseen(Card::Powerglass);
    if (!powerglass_held && !arven_searches_live_powerglass) return false;

    const bool needed_grass_in_discard =
        state_.active->grass < 2 && count_of(state_.discard, Card::Grass) > 0;
    const bool needed_fire_in_discard =
        state_.active->fire < 1 && count_of(state_.discard, Card::Fire) > 0;
    return needed_grass_in_discard || needed_fire_in_discard;
  }

  Pokemon* first_open_fss_v_holder(const bool allow_active) {
    // Strategy prefers a Pokemon V while Star Alchemy is still available. This is
    // a route preference, separate from the broader physical Tool legality below:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Architecture contract: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
    if (allow_active && state_.active && is_pokemon_v(state_.active->card) &&
        state_.active->tool == Tool::None) {
      return &*state_.active;
    }
    for (Pokemon& pokemon : state_.bench) {
      if (is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None) return &pokemon;
    }
    return nullptr;
  }

  Pokemon* first_open_fss_holder(const bool allow_active) {
    // Forest Seal Stone is a Pokemon Tool, so physical attachment only requires a
    // Pokemon with an open Tool slot. Its separate Pokemon V clause grants access
    // to Star Alchemy and does not restrict which Pokemon may hold the Tool:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed legality bug: https://github.com/FlareZ123/pokemon-sims/issues/4158
    if (Pokemon* holder = first_open_fss_v_holder(allow_active)) return holder;
    if (allow_active && state_.active && state_.active->tool == Tool::None) {
      return &*state_.active;
    }
    for (Pokemon& pokemon : state_.bench) {
      if (pokemon.tool == Tool::None) return &pokemon;
    }
    return nullptr;
  }

  void attach_fss_to(Pokemon& target) {
    // Physical resolution accepts any legal holder selected by strategy:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    remove_one(state_.hand, Card::ForestSealStone);
    target.tool = Tool::ForestSealStone;
  }

  bool attach_fss() {
    if (hand_count(Card::ForestSealStone) == 0) return false;

    // Preserve Star Alchemy access while the VSTAR Power is unused. Powerglass
    // reservation may move FSS to another Pokemon V, but must not strand the Stone
    // on a non-V merely to empty the hand:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Architecture contract: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
    if (!state_.vstar_power_used && active_powerglass_tool_slot_needed()) {
      Pokemon* target = first_open_fss_v_holder(false);
      if (target != nullptr) {
        attach_fss_to(*target);
        trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL; R-POWERGLASS-01",
              "Attached Forest Seal Stone to Benched " + std::string(name(target->card)) +
                  " and reserved the Active Tool slot for Powerglass.");
        return true;
      }
      return false;
    }

    // While Star Alchemy remains unused, choose only a Pokemon V holder so this
    // strategic attachment preserves the printed VSTAR Power. Once the one-per-game
    // VSTAR Power is already spent, any legal open Tool slot is equivalent for FSS
    // hand-thinning purposes and the broader physical legality becomes usable:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed legality/strategy separation: https://github.com/FlareZ123/pokemon-sims/issues/4158
    Pokemon* target = state_.vstar_power_used
        ? first_open_fss_holder(true)
        : first_open_fss_v_holder(true);
    if (target == nullptr) return false;

    attach_fss_to(*target);
    trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL",
          "Attached Forest Seal Stone to " + std::string(name(target->card)) + ".");
    return true;
  }
