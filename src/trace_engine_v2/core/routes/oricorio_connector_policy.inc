#pragma once

// Canonical Oricorio energy-connector policy.
// Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
// Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

  class OricorioConnectorPolicy final {
   public:
    struct EnergyNeedProjection final {
      int amount_needed;
      bool card_might_be_unseen;
    };

    struct ConnectorProjection final {
      bool needed_energy_might_be_in_deck;
      bool original_connector_route;
      bool burnet_preserving_route;
    };

    [[nodiscard]] static bool needed_basic_energy_can_advance(
        const EnergyNeedProjection& projection) {
      // Vital Dance searches Basic Energy, so a type advances this connector only
      // while Apex still needs that type and the fixed-list knowledge state permits
      // a remaining deck copy: https://api.pokemontcg.io/v2/cards/sm2-55
      // Apex Dragon energy requirement: https://api.pokemontcg.io/v2/cards/swsh12-136
      // K0/K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
      return projection.amount_needed > 0 && projection.card_might_be_unseen;
    }

    [[nodiscard]] static bool connector_route_live(
        const ConnectorProjection& projection) {
      return projection.needed_energy_might_be_in_deck &&
             (projection.original_connector_route ||
              projection.burnet_preserving_route);
    }
  };

  [[nodiscard]] bool needed_oricorio_energy_might_be_in_deck() const {
    // Vital Dance can search Basic Energy, while Apex Dragon specifically requires
    // two Grass and one Fire. Keep the existing Grass-first short-circuit: Fire
    // visibility is queried only when Grass cannot already advance the route.
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // K0/K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    return OricorioConnectorPolicy::needed_basic_energy_can_advance(
               {grass_needed(), might_be_unseen(Card::Grass)}) ||
           OricorioConnectorPolicy::needed_basic_energy_can_advance(
               {fire_needed(), might_be_unseen(Card::Fire)});
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
    const bool needed_energy_might_be_in_deck =
        needed_oricorio_energy_might_be_in_deck();
    if (!needed_energy_might_be_in_deck) return false;
    const bool original_connector_route = needs_oricorio_connector_original();
    const OricorioConnectorPolicy::ConnectorProjection projection{
        needed_energy_might_be_in_deck,
        original_connector_route,
        burnet_preserving_route,
    };
    return OricorioConnectorPolicy::connector_route_live(projection);
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
