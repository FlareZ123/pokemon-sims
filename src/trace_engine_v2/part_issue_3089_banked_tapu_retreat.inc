bool retreat_banked_tapu_to_regidrago() {
  if (state_.retreat_used || !state_.active ||
      state_.active->card != Card::TapuLeleGX ||
      state_.active->grass + state_.active->fire < 1 ||
      can_free_retreat_with_latias() ||
      (supporter_allowed() && hand_count(Card::TateLiza) > 0)) {
    return false;
  }

  Pokemon* target = target_regi();
  // Hold Tapu Active until the promotion target is an evolved Regidrago VSTAR
  // that already pays Apex Dragon's GGF cost. Double Dragon Energy supplies two
  // Energy of every type while attached to a Dragon Pokemon, so readiness must
  // use the shared physical-card Energy predicate rather than raw Basic counters:
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Official Retreat, evolution, and attack procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // DDE physical-Energy model: https://github.com/FlareZ123/pokemon-sims/issues/2238
  // Earliest deterministic route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3089
  if (target == nullptr || target->card != Card::RegidragoVstar ||
      !pays_apex_energy_cost(*target)) {
    return false;
  }

  const Card payment = state_.active->grass > 0 ? Card::Grass : Card::Fire;
  if (payment == Card::Grass) {
    --state_.active->grass;
  } else {
    --state_.active->fire;
  }
  state_.discard.push_back(payment);
  std::swap(*state_.active, *target);
  state_.retreat_used = true;

  // Tapu's banked Basic Energy pays its printed one-Colorless Retreat Cost.
  // Free Latias and Tate routes stay preferred by the guards above:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Official Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3089
  trace("RETREAT", "R-GAME-RETREAT; R-TAPU-01; P-DCI-01",
        "Paid Tapu Lele-GX's banked one-Energy Retreat Cost and promoted Regidrago.");
  return true;
}
