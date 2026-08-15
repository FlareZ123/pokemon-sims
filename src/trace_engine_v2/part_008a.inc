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

  bool promote_best_benched_vstar() {
    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr) return false;
    std::swap(*state_.active, *target);
    return true;
  }

  bool retreat_to_benched_vstar_with_latias() {
    if (!need_active_vstar() || !can_free_retreat_with_latias()) return false;
    // Latias ex removes the Basic Active's Retreat Cost. Promote the Benched
    // Regidrago VSTAR that best satisfies Apex Dragon's GGF cost:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (!promote_best_benched_vstar()) return false;
    state_.retreat_used = true;
    trace("RETREAT", "R-LATIAS-01; R-GAME-RETREAT",
          "Latias ex gives the Basic Active no Retreat Cost; promoted the "
          "best-powered Benched Regidrago VSTAR.");
    return true;
  }

// Tate & Liza owns its complete Supporter consume/switch/draw policy under the
// route namespace while preserving this established Engine member boundary.
// Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/tate_liza_policy.inc
#include "core/routes/tate_liza_policy.inc"
