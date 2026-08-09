from pathlib import Path


def replace_once(path: str, old: str, new: str) -> None:
    source = Path(path).read_text(encoding="utf-8")
    if source.count(old) != 1:
        raise SystemExit(f"Expected exactly one DDE composition target in {path}")
    Path(path).write_text(source.replace(old, new), encoding="utf-8")


replace_once(
    "src/trace_engine_v2/part_issue_1737_gladion_prized_steven_override.inc",
    '''      !supporter_allowed() || state_.manual_energy_used ||
      hand_count(Card::Gladion) == 0 || hand_count(energy) > 0 ||
      !state_.active || state_.active->card != Card::RegidragoVstar ||
      prize_count_after_reveal(energy) == 0 ||
      grass_needed() + fire_needed() != 1 || !payload_ready()) {
    return false;
  }

  // Gladion may exchange itself for any known Prize. When the Active Regidrago
  // VSTAR is exactly one manual attachment from GGF and this turn's strict-JIT
  // Dragon is already in discard, that prized Basic Energy is the immediate
  // winning-axis connector even if another copy of the same type remains in deck.
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Prize, Supporter, and manual Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, strict-JIT, and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2292
  return (energy == Card::Grass && grass_needed() == 1 && fire_needed() == 0) ||
         (energy == Card::Fire && grass_needed() == 0 && fire_needed() == 1);
''',
    '''      !supporter_allowed() || state_.manual_energy_used ||
      hand_count(Card::Gladion) == 0 || hand_count(energy) > 0 ||
      !state_.active || state_.active->card != Card::RegidragoVstar ||
      pays_apex_energy_cost(*state_.active) ||
      prize_count_after_reveal(energy) == 0 || !payload_ready() ||
      (energy != Card::Grass && energy != Card::Fire)) {
    return false;
  }

  Pokemon projected = *state_.active;
  const bool final_basic_completes_apex =
      attach_energy_card(projected, energy) && pays_apex_energy_cost(projected);

  // Gladion may exchange itself for any known Prize. Evaluate the final Basic
  // attachment by projecting the physical Energy card onto the current Active,
  // rather than summing typed Grass/Fire deficits. A previously attached Double
  // Dragon Energy contributes two flexible units to Apex Dragon, so additive
  // typed-deficit arithmetic can misclassify an otherwise complete GGF payment.
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Prize, Supporter, and manual Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, strict-JIT, and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original Gladion bug: https://github.com/FlareZ123/pokemon-sims/issues/2292
  // Confirmed DDE semantic migration: https://github.com/FlareZ123/pokemon-sims/issues/2368
  return final_basic_completes_apex;
''',
)

replace_once(
    "src/trace_engine_v2/part_012.inc",
    '''          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.grass >= 2 &&
              pokemon.fire >= 1;
''',
    '''          // Project readiness through the canonical Apex payment helper so a prior-turn
          // Double Dragon Energy counts as its printed two flexible Energy units:
          // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
          // https://api.pokemontcg.io/v2/cards/swsh12-136
          // Confirmed DDE migration: https://github.com/FlareZ123/pokemon-sims/issues/2368
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pays_apex_energy_cost(pokemon);
''',
)
