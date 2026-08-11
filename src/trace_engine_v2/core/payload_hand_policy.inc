#pragma once

#ifndef REGIDRAGO_PAYLOAD_HAND_POLICY_DEFINED
#define REGIDRAGO_PAYLOAD_HAND_POLICY_DEFINED

// Centralized payload-in-hand helpers shared by opening connector and DCI/JIT
// policies. Keep the preference order here so callers do not rebuild ad hoc
// payload scans with diverging discard semantics.
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
