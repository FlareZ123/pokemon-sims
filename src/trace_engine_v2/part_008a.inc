    int best_score = -100000;
    if (state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn) {
      best = &*state_.active;
      best_score = score_regi(*best, true);
    }
    for (Pokemon& p : state_.bench) {
      if (p.card != Card::RegidragoV || p.entered_turn >= state_.turn) continue;
      const int score = score_regi(p, false);
      if (score > best_score) {
        best = &p;
        best_score = score;
      }
    }
    if (best == nullptr) return false;
    remove_one(state_.hand, Card::RegidragoVstar);
    best->card = Card::RegidragoVstar;
    trace("EVOLVE", "R-GAME-EVOLVE",
          "Evolved " + std::string(name(Card::RegidragoV)) +
              " into Regidrago VSTAR.");
    return true;
  }

#include "core/routes/latias_vstar_promotion_policy.inc"

#include "core/supporter_source_lifecycle.inc"


  bool play_tate_switch() {
    // Card text source: https://api.pokemontcg.io/v2/cards/sm7-148
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 ||
        !need_active_vstar()) {
      return false;
    }
    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr) return false;
    // Stage Tate & Liza before its switch effect and discard it afterward.
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Supporter procedure B-03: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    if (!begin_supporter_resolution(Card::TateLiza)) return false;
    std::swap(*state_.active, *target);
    if (!finish_supporter_resolution(Card::TateLiza)) return false;
    // Tate & Liza may switch any Benched Pokémon Active. Select the Regidrago
    // VSTAR that best satisfies Apex Dragon's GGF cost before spending Supporter:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER",
          "Tate & Liza switch mode: promoted the best-powered Benched Regidrago "
          "VSTAR.");
    return true;
  }

  bool play_tate_draw() {
    // Tate & Liza has no hand-size condition: shuffle the whole hand into the deck,
    // then draw 5 cards: https://api.pokemontcg.io/v2/cards/sm7-148
    // This preserves legal five-or-more-card dead-hand refreshes after direct connectors fail.
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0) return false;
    // After the played Supporter leaves hand, an empty deck plus no remaining hand
    // cards makes this draw mode guaranteed to do nothing, so the Trainer is illegal:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    if (state_.deck.empty() && state_.hand.size() == 1U) return false;
    // Its own source must be outside the hand being shuffled.
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Supporter procedure B-03: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    if (!begin_supporter_resolution(Card::TateLiza)) return false;
    state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
    shuffle(state_.deck);
    for (int draw = 0; draw < 5; ++draw) draw_card("Tate & Liza");
    if (!finish_supporter_resolution(Card::TateLiza)) return false;
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER",
          "Tate & Liza draw mode: shuffled remaining hand and drew 5.");
    return true;
  }
