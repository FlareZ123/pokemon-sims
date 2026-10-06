// Canonical Mysterious Treasure strategic target-order owner.
// Mysterious Treasure searches Psychic or Dragon Pokémon:
// https://api.pokemontcg.io/v2/cards/sm6-113
// Appletun is a legal Dragon target:
// https://api.pokemontcg.io/v2/cards/sv8-140
// Strategy ordering remains Engine-owned:
// https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

  class MysteriousTreasureTargetPolicy final {
   public:
    using TargetOrder = std::array<Card, 10>;

    [[nodiscard]] static constexpr const TargetOrder& preferred_targets() {
      return kPreferredTargets;
    }

   private:
    inline static constexpr TargetOrder kPreferredTargets{
        Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx,
        Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
        Card::DialgaGX, Card::Appletun, Card::TapuLeleGX, Card::Oricorio};
  };
