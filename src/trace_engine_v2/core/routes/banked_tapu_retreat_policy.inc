#pragma once

// Canonical paid-retreat continuation for a banked Tapu Lele-GX Active.
// The parent composition includes this owner directly at the proven post-search
// class-member boundary; keep route behavior and rule sources centralized here.
// Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
// Advanced Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

bool banked_tapu_regidrago_ready(const Pokemon* target) {
  if (target == nullptr || target->card != Card::RegidragoVstar) return false;

  // Preserve the historical two-Grass promotion state while admitting physical
  // DDE payment shapes that already satisfy Apex Dragon. Double Dragon Energy
  // supplies two Energy of every type while attached to a Dragon Pokemon, so a
  // DDE-ready VSTAR cannot be classified from raw Basic counters alone:
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Official Retreat, evolution, and attack procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // DDE physical-Energy model: https://github.com/FlareZ123/pokemon-sims/issues/2238
  // Historical proactive-Tapu route: https://github.com/FlareZ123/pokemon-sims/issues/1845
  // Earliest deterministic route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed DDE regression: https://github.com/FlareZ123/pokemon-sims/issues/3089
  const bool historical_two_grass_state = target->grass >= 2 && target->fire == 0;
  return historical_two_grass_state || pays_apex_energy_cost(*target);
}

Card banked_tapu_retreat_payment() {
  // The route preflight guarantees a Basic Energy is attached. Prefer Grass when
  // both physical Basic types are available, preserving the historical payment
  // order while giving the mutation helper one canonical choice point:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Official Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  return state_.active->grass > 0 ? Card::Grass : Card::Fire;
}

void pay_banked_tapu_retreat_cost() {
  // Pay exactly one physical Energy for Tapu Lele-GX's one-Colorless printed
  // Retreat Cost using the canonical payment choice above:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Official Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  const Card payment = banked_tapu_retreat_payment();
  if (payment == Card::Grass) {
    --state_.active->grass;
  } else {
    --state_.active->fire;
  }
  state_.discard.push_back(payment);
}

bool banked_tapu_paid_retreat_preserves_burnet_finish() const {
  if (!state_.active || state_.active->card != Card::TapuLeleGX ||
      state_.active->grass + state_.active->fire < 1 || state_.retreat_used ||
      !supporter_allowed() || state_.supporter_used ||
      hand_count(Card::ProfessorBurnet) == 0) {
    return false;
  }

  // Project only the current public/K1 state. A paid Retreat is a distinct
  // once-per-turn action, so it leaves the one Supporter action available for
  // Professor Burnet. Tate & Liza would consume that Supporter action instead:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Advanced Retreat and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed route-selection bug: https://github.com/FlareZ123/pokemon-sims/issues/4157
  // Same-turn Burnet projection precedent: https://github.com/FlareZ123/pokemon-sims/issues/2158
  std::mt19937_64 shadow_rng = rng_;
  Engine projected(scenario_, recipe_, shadow_rng);
  projected.state_ = state_;
  projected.deck_seen_ = deck_seen_;
  projected.prizes_revealed_ = prizes_revealed_;
  projected.trace_ = nullptr;

  Pokemon* target = projected.target_regi();
  if (!projected.banked_tapu_regidrago_ready(target)) return false;

  projected.pay_banked_tapu_retreat_cost();
  std::swap(*projected.state_.active, *target);
  projected.state_.retreat_used = true;
  return projected.professor_burnet_has_live_ready_turn_route();
}

bool banked_tapu_paid_retreat_priority_open() {
  // Free Skyliner remains the dominant zero-cost route. Tate & Liza remains the
  // preferred switch when spending the Supporter does not destroy a stronger
  // same-turn Professor Burnet finish. Paid Retreat wins only in that exact
  // supporter-contention state:
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Advanced Retreat and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Earliest deterministic route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed route-selection bug: https://github.com/FlareZ123/pokemon-sims/issues/4157
  if (can_free_retreat_with_latias()) return false;
  if (!(supporter_allowed() && hand_count(Card::TateLiza) > 0)) return true;
  return banked_tapu_paid_retreat_preserves_burnet_finish();
}

bool banked_tapu_paid_retreat_available() {
  // Tapu's printed one-Colorless Retreat Cost may be paid with its banked Basic
  // Energy after higher-priority zero-cost and route-superior options are absent:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Official Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  return !state_.retreat_used && state_.active &&
         state_.active->card == Card::TapuLeleGX &&
         state_.active->grass + state_.active->fire >= 1 &&
         banked_tapu_paid_retreat_priority_open();
}

bool retreat_banked_tapu_to_regidrago() {
  if (!banked_tapu_paid_retreat_available()) return false;

  Pokemon* target = target_regi();
  if (!banked_tapu_regidrago_ready(target)) return false;

  pay_banked_tapu_retreat_cost();
  std::swap(*state_.active, *target);
  state_.retreat_used = true;

  // Tapu's banked Basic Energy pays its printed one-Colorless Retreat Cost.
  // Free Latias stays preferred; a live same-turn Burnet finish may supersede Tate:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Official Retreat and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed DDE regression: https://github.com/FlareZ123/pokemon-sims/issues/3089
  // Confirmed supporter-contention regression: https://github.com/FlareZ123/pokemon-sims/issues/4157
  trace("RETREAT", "R-GAME-RETREAT; R-TAPU-01; P-DCI-01",
        "Paid Tapu Lele-GX's banked one-Energy Retreat Cost and promoted Regidrago.");
  return true;
}
