#pragma once

// Canonical Oricorio energy-connector policy.
// Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
// Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

  [[nodiscard]] bool oricorio_needed_basic_energy_might_be_in_deck(
      const Card energy, const int amount_needed) const {
    return amount_needed > 0 && might_be_unseen(energy);
  }

  [[nodiscard]] bool needed_oricorio_energy_might_be_in_deck() const {
    // Vital Dance can search Basic Energy, while Apex Dragon specifically requires
    // two Grass and one Fire. At K1, an off-type Energy cannot advance the missing
    // component; K0 keeps using public fixed-list plausibility through might_be_unseen():
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    return oricorio_needed_basic_energy_might_be_in_deck(
               Card::Grass, grass_needed()) ||
           oricorio_needed_basic_energy_might_be_in_deck(
               Card::Fire, fire_needed());
  }

  [[nodiscard]] bool needs_oricorio_connector() const {
    // A connector whose entry Ability is suppressed cannot advance the Energy axis,
    // so do not spend Bench space admitting it. Garbotoxin suppresses other Pokemon
    // Abilities while its Tool condition is met; Vital Dance only supplies value when
    // Oricorio is played from hand to the Bench and that Ability can resolve:
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    if (!ability_available_for_pokemon_garbodor(Card::Oricorio)) return false;

    // The original #180 predicate covers hands with no payload. #616 extends the
    // same final-slot policy to an in-hand payload that has no legal Item discard
    // outlet, while preserving Tapu Lele-GX when such an outlet is live:
    // https://github.com/FlareZ123/pokemon-sims/issues/180
    // https://github.com/FlareZ123/pokemon-sims/issues/616
    const bool burnet_preserving_route =
        oricorio_preserves_burnet_with_current_hand();
    return needed_oricorio_energy_might_be_in_deck() &&
           (needs_oricorio_connector_original() || burnet_preserving_route);
  }

  bool bench_oricorio_if_useful() {
    if (hand_count(Card::Oricorio) == 0 || in_play(Card::Oricorio) ||
        bench_space() <= 0 || !needs_oricorio_connector()) {
      return false;
    }
    // Playing Oricorio from hand to the Bench triggers the optional Vital Dance
    // search only after the target-aware preflight has found a needed type:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    return bench_from_hand(Card::Oricorio, true);
  }
