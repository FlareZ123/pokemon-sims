from pathlib import Path

source = Path('src/trace_engine_v2/part_014b.inc')
text = source.read_text(encoding='utf-8')
old_gate = '''  bool play_roseanne_energy_recovery() {
    if (!supporter_allowed() || hand_count(Card::RoseannesBackup) == 0 ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar || need_regi() || need_vstar() ||
        need_active_vstar() || need_payload()) {
      return false;
    }

    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed + active_fire_needed != 1) return false;
    const Card needed_energy = active_grass_needed == 1 ? Card::Grass : Card::Fire;

    // Roseanne's Backup may choose its Energy mode and shuffle one Energy from the
    // discard pile into the deck. Earthen Vessel can then search that restored Basic
    // Energy for the still-unused manual attachment in the same turn:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (count_of(state_.discard, needed_energy) == 0 || might_be_unseen(needed_energy) ||
        !has_payable_roseanne_energy_vessel()) {
      return false;
    }
'''
new_gate = '''  std::optional<Card> roseanne_finishing_basic_to_recover() const {
    if (!state_.active || state_.active->card != Card::RegidragoVstar ||
        pays_apex_energy_cost(*state_.active)) {
      return std::nullopt;
    }

    // Roseanne's Backup can restore one Energy from discard. Keep this recovery
    // route limited to a Basic that is absent from the searchable deck and whose
    // projected manual attachment actually completes Apex after accounting for DDE.
    // Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2444
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (count_of(state_.discard, basic) == 0 || might_be_unseen(basic)) continue;
      Pokemon projected = *state_.active;
      if (attach_energy_card(projected, basic) &&
          pays_apex_energy_cost(projected)) {
        return basic;
      }
    }
    return std::nullopt;
  }

  bool play_roseanne_energy_recovery() {
    if (!supporter_allowed() || hand_count(Card::RoseannesBackup) == 0 ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar || need_regi() || need_vstar() ||
        need_active_vstar() || need_payload()) {
      return false;
    }

    const auto finishing_energy = roseanne_finishing_basic_to_recover();
    if (!finishing_energy.has_value() || !has_payable_roseanne_energy_vessel()) {
      return false;
    }
    const Card recovered_energy = *finishing_energy;

    // Roseanne's Backup may choose its Energy mode and shuffle one Energy from the
    // discard pile into the deck. Earthen Vessel can then search that restored Basic
    // Energy for the still-unused manual attachment in the same turn:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2444
'''
old_resolution = '''    if (!remove_one(state_.discard, needed_energy)) {
      throw std::logic_error("Roseanne's Backup Energy target disappeared");
    }
    const auto turn_it = std::find(state_.discarded_this_turn.begin(),
                                   state_.discarded_this_turn.end(), needed_energy);
    if (turn_it != state_.discarded_this_turn.end()) state_.discarded_this_turn.erase(turn_it);
    state_.deck.push_back(needed_energy);
'''
new_resolution = '''    if (!remove_one(state_.discard, recovered_energy)) {
      throw std::logic_error("Roseanne's Backup Energy target disappeared");
    }
    const auto turn_it = std::find(state_.discarded_this_turn.begin(),
                                   state_.discarded_this_turn.end(), recovered_energy);
    if (turn_it != state_.discarded_this_turn.end()) state_.discarded_this_turn.erase(turn_it);
    state_.deck.push_back(recovered_energy);
'''
if text.count(old_gate) != 1:
    raise SystemExit(f'expected one Roseanne gate, found {text.count(old_gate)}')
if text.count(old_resolution) != 1:
    raise SystemExit(f'expected one Roseanne resolution, found {text.count(old_resolution)}')
source.write_text(text.replace(old_gate, new_gate).replace(old_resolution, new_resolution), encoding='utf-8')
