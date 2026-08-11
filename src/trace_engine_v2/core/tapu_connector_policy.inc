#pragma once

  bool has_redundant_tapu_connector_copy(const Card card) const {
    return card == Card::TapuLeleGX && has_known_hand_or_deck_copy(card);
  }

  bool tapu_connector_copy_aware_in_play(const Card card) const {
    // Wonder Tag belongs to the individual Tapu Lele-GX played from hand onto the
    // Bench. Ignore species-level board presence only when another physical copy is
    // proven in hand or in an inspected deck. K0 uncertainty alone must not displace
    // a stronger deterministic route such as Team Yell's Cheer into Latias ex:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/746
    return in_play(card) && !has_redundant_tapu_connector_copy(card);
  }

// Keep the legacy Tapu-adjacent aliases with the policy that consumes them so the
// opening composition has one owner for this connector family.
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
// Route history: https://github.com/FlareZ123/pokemon-sims/issues/1726
#define bench_crobat_if_useful bench_crobat_if_useful_issue_1726_original

// Preserve the completed #1605 implementation as the historical baseline while
// #2645 installs the state-driven route after part_006.inc is parsed:
// https://github.com/FlareZ123/pokemon-sims/issues/1605
// https://github.com/FlareZ123/pokemon-sims/issues/2645
#define issue_1605_arven_crobat_route_available \
  issue_1605_arven_crobat_route_available_issue2645_original

// Payload-in-hand selection is part of the same opening connector policy surface.
// Keeping these helpers here removes a separate composition dependency while
// preserving the established DCI/JIT preference order.
#ifndef REGIDRAGO_PAYLOAD_HAND_POLICY_DEFINED
#define REGIDRAGO_PAYLOAD_HAND_POLICY_DEFINED
  inline static constexpr std::array<Card, 5> kPayloadHandPreference{{
      Card::Appletun,
      Card::MegaDragonite,
      Card::Dragapult,
      Card::GoodraVstar,
      Card::DialgaGX,
  }};

  static int payload_card_count(const std::vector<Card>& zone) {
    return static_cast<int>(std::count_if(zone.begin(), zone.end(), is_payload));
  }

  template <typename Present>
  static std::optional<Card> first_payload_by_preference(Present present) {
    for (const Card card : kPayloadHandPreference) {
      if (present(card)) return card;
    }
    return std::nullopt;
  }

  int payload_cards_in_hand_count() const {
    return payload_card_count(state_.hand);
  }

  std::optional<Card> first_payload_card_in_hand() const {
    // Keep the simulator's established payload preference order while centralizing
    // repeated DCI/JIT hand scans:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    return first_payload_by_preference(
        [this](const Card card) { return hand_count(card) > 0; });
  }
#endif  // REGIDRAGO_PAYLOAD_HAND_POLICY_DEFINED
