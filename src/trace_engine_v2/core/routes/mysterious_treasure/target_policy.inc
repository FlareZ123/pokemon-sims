#pragma once

// Canonical Mysterious Treasure strategic target-order owner.
// Keep target ordering in Engine strategy: the card's printed effect only defines
// Psychic-or-Dragon legality, while setup priority remains route policy.
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
// Strategy ownership: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  class MysteriousTreasureTargetPolicy final {
   public:
    using TargetOrder = std::array<Card, 10>;

    [[nodiscard]] static constexpr const TargetOrder& preferred_targets() {
      return kPreferredTargets;
    }

   private:
    // Mysterious Treasure searches Psychic or Dragon Pokémon, and Appletun is a legal Dragon target:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // Keep the established earliest-ready-route priority in one immutable policy value:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    inline static constexpr TargetOrder kPreferredTargets{
        Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx,
        Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
        Card::DialgaGX, Card::Appletun, Card::TapuLeleGX, Card::Oricorio};
  };
