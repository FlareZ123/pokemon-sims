#pragma once

// Pure bookkeeping for the issue-1016 Legacy Star route projection. The engine
// temporarily hides held Quick Ball copies only from the wrapped route view, then
// restores the same copies to the physical hand in the same append position used
// by the historical implementation. Route admission and DCI/JIT decisions remain
// with the Engine caller.
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
// Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
// Confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/1016
class LegacyStarQuickBallProjectionPolicy final {
 public:
  [[nodiscard]] static int hide_from_route_view(std::vector<Card>& hand) {
    const int held_quick_balls =
        static_cast<int>(std::count(hand.begin(), hand.end(), Card::QuickBall));
    std::erase(hand, Card::QuickBall);
    return held_quick_balls;
  }

  static void restore_to_hand(std::vector<Card>& hand,
                              const int held_quick_balls) {
    hand.insert(hand.end(), static_cast<std::size_t>(held_quick_balls),
                Card::QuickBall);
  }
};
