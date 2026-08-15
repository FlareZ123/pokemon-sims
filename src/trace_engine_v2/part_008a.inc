    int best_score = -100000;
    if (state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn) {
      best = &*state_.active;
      best_score = score_regi(*best, true);
    }
    for (Pokemon& pokemon : state_.bench) {
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn) {
        continue;
      }
      const int score = score_regi(pokemon, false);
      if (score > best_score) {
        best = &pokemon;
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

#include "core/tate_liza_supporter_policy.inc" // Canonical Supporter-mode owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate_liza_supporter_policy.inc
