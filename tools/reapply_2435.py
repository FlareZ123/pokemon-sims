from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=root/'src/trace_engine_v2/part_issue_989_empty_deck_tapu_override.inc';s=p.read_text(encoding='utf-8')
old='''    const bool exactly_one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool needed_energy_in_deck =
        (grass_needed() == 1 &&
         deck_count_after_search_started(Card::Grass) > 0) ||
        (fire_needed() == 1 &&
         deck_count_after_search_started(Card::Fire) > 0);
    const bool duplicate_arven_would_repeat_vessel_route =
        exactly_one_energy_missing && needed_energy_in_deck &&
'''
new='''    const Pokemon* energy_target = target_regi();
    constexpr std::array<Card, 2> kBasicEnergy{Card::Grass, Card::Fire};
    const bool one_deck_basic_completes_apex = energy_target != nullptr &&
        std::any_of(kBasicEnergy.begin(), kBasicEnergy.end(),
                    [this, energy_target](const Card basic) {
          if (deck_count_after_search_started(basic) == 0) return false;
          Pokemon projected = *energy_target;
          if (!attach_energy_card(projected, basic)) return false;
          // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
          // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
          // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
          // Manual attachment rule: https://www.pokemon.com/us/pokemon-tcg/rules
          // DDE contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2435
          return pays_apex_energy_cost(projected);
        });
    const bool duplicate_arven_would_repeat_vessel_route =
        one_deck_basic_completes_apex &&
'''
if s.count(old)!=1: raise RuntimeError(f'first matches={s.count(old)}')
s=s.replace(old,new,1)
old2='''    const bool completed_without_second_tapu = projected.active_is_vstar() &&
        projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 && projected.payload_ready();
'''
new2='''    const bool completed_without_second_tapu = projected.active_is_vstar() &&
        projected.state_.active &&
        // DDE semantic readiness: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2435
        projected.pays_apex_energy_cost(*projected.state_.active) &&
        projected.payload_ready();
'''
if s.count(old2)!=1: raise RuntimeError(f'second matches={s.count(old2)}')
p.write_text(s.replace(old2,new2,1),encoding='utf-8')
