  std::optional<Card> choose_discard_issue1740(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    const auto ordinary = choose_discard(
        permit_payload, flex_fodder, allow_heavy_ball, excluded_from_cost,
        allow_redundant_payload);
    const bool duplicate_treasure_is_ordinary_cost =
        ordinary && *ordinary == Card::MysteriousTreasure;
    const bool preserve_second_treasure_for_vstar =
        duplicate_treasure_is_ordinary_cost &&
        scenario_.dci == DciProfile::StrictJit && excluded_from_cost &&
        *excluded_from_cost == Card::MysteriousTreasure &&
        hand_count(Card::MysteriousTreasure) >= 2 &&
        hand_count(Card::Channeler) > 0 && need_regi() && bench_space() > 0 &&
        hand_count(Card::RegidragoV) == 0 && might_be_unseen(Card::RegidragoV) &&
        !has_vstar() && hand_count(Card::RegidragoVstar) == 0 &&
        might_be_unseen(Card::RegidragoVstar);
    if (preserve_second_treasure_for_vstar) {
      // Replace only the existing duplicate-Treasure fallback. Every ordinary
      // lower-DCI cost keeps its established priority. The first Treasure must
      // establish the missing Basic, while the distinct second copy remains the
      // one-discard VSTAR connector and setup-inert Channeler pays the current cost:
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
      // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official Item, discard-cost, search, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // K0/K1, dynamic DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#card-text-outside-initial-setup https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug and source-bound projection: https://github.com/FlareZ123/pokemon-sims/issues/1740 https://github.com/FlareZ123/pokemon-sims/actions/runs/30390502968
      return Card::Channeler;
    }
    return ordinary;
  }

#undef choose_discard
#define choose_discard choose_discard_issue1740
#include "part_issue_1876_treasure_payload_cost_override.inc"
