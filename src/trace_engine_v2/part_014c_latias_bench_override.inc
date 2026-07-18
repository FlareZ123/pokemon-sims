// This fragment is composed inside Engine by part_014c.inc. The later top-level
// compatibility include is intentionally inert after this first expansion:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc#L168-L179
// The following part_015.inc remains the continuation of the free scenario lookup:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_015.inc#L1-L4
#ifndef POKEMON_SIMS_PART_014C_LATIAS_BENCH_OVERRIDE_INCLUDED
#define POKEMON_SIMS_PART_014C_LATIAS_BENCH_OVERRIDE_INCLUDED

// part_014c.inc composes this fragment while Engine remains open. Replace the
// macro-renamed legacy Field Blower policy at the same valid member boundary:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc#L168-L179
// https://github.com/FlareZ123/pokemon-sims/issues/791
#undef play_field_blower
#include "part_014c_field_blower_override.inc"

  bool held_item_can_find_pending_vstar() {
    if (item_locked() || !might_be_unseen(Card::RegidragoVstar)) return false;

    // A held direct Pokémon-search Item can make Regidrago VSTAR available later in
    // this turn. Pokémon Communication is live only when its exact exchange plan
    // returns a safe held Pokémon and selects the pending VSTAR:
    // Evolution Incense: https://api.pokemontcg.io/v2/cards/swsh1-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/687
    // https://github.com/FlareZ123/pokemon-sims/issues/701
    if (hand_count(Card::EvolutionIncense) > 0) return true;
    if (hand_count(Card::PokemonCommunication) > 0) {
      const auto plan = pokemon_communication_plan(false);
      if (plan && plan->target == Card::RegidragoVstar) return true;
    }

    const bool permit_payload_cost = can_play_payload_this_turn();
    if (hand_count(Card::MysteriousTreasure) > 0 &&
        choose_discard(permit_payload_cost, true, true,
                       Card::MysteriousTreasure).has_value()) {
      return true;
    }
    if (hand_count(Card::UltraBall) == 0) return false;

    const std::vector<Card> original_hand = state_.hand;
    const auto first = choose_discard(permit_payload_cost, true, true, Card::UltraBall);
    if (!first || !remove_one(state_.hand, *first)) {
      state_.hand = original_hand;
      return false;
    }
    const auto second = choose_discard(
        can_play_payload_this_turn() && !is_payload(*first), true, true, Card::UltraBall);
    state_.hand = original_hand;
    return second.has_value();
  }

  bool bench_latias_for_pending_vstar_promotion() {
    if (bench_space() != 1 || hand_count(Card::LatiasEx) == 0 ||
        in_play(Card::LatiasEx) || !ability_available_for_pokemon(Card::LatiasEx) ||
        state_.retreat_used || !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV ||
        (hand_count(Card::RegidragoVstar) == 0 &&
         !held_item_can_find_pending_vstar())) {
      return false;
    }

    // Latias ex gives a Basic Active no Retreat Cost. Preserve the final Bench slot
    // when a prior-turn Benched Regidrago V can evolve now, then use Skyliner to
    // promote that VSTAR. Tate & Liza is an alternate switch route and does not need
    // this scarce Bench slot:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool prior_turn_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
                 pokemon.entered_turn < state_.turn;
        });
    const bool tate_switch_available = supporter_allowed() &&
        hand_count(Card::TateLiza) > 0;
    if (!prior_turn_benched_regi || tate_switch_available) return false;

    return bench_from_hand(Card::LatiasEx, false);
  }

  void run_turn() {
    trace("POLICY", "P-AXIS-01", "Start: " + state_line());
    play_chaotic_swell();
    play_field_blower();
    // A redundant Regidrago V must not consume the final Bench slot before the
    // immediately live Latias ex promotion route:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    bench_latias_for_pending_vstar_promotion();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());
    attach_live_fss();
    use_fss();
    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());

    attach_manual();
    // Steven's Resolve ends the turn, so attach a live Powerglass before choosing
    // that Supporter: https://api.pokemontcg.io/v2/cards/sm7-145
    if (should_play_steven()) attach_powerglass();
    choose_supporter();
    if (state_.turn_ended) {
      trace("TURN END", "R-STEVEN-01 or R-RV-01", state_line());
      return;
    }

    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    attach_live_fss();
    use_fss();
    play_chaotic_swell();
    play_field_blower();
    play_items_until_stable(!strict_payload_timing());
    attach_manual();
    evolve_best_regi();
    play_basics_from_hand();
    play_items_until_stable(true);
    // A strict-JIT Item route may now discard an in-hand Dragon payload and search
    // Regidrago VSTAR through Mysterious Treasure (https://api.pokemontcg.io/v2/cards/sm6-113).
    // Retry evolution so a Regidrago V that entered play on a prior turn can evolve
    // before this turn's ready check under the core evolution procedure (https://www.pokemon.com/us/pokemon-tcg/rules).
    evolve_best_regi();
    // A late Item search can put Tapu Lele-GX into hand, and Bench play can then
    // fetch a new Supporter with Wonder Tag. Reconsider that unused Supporter slot
    // before the ready check: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Professor Burnet can discard a current-turn deck payload after this chain:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    if (!state_.supporter_used && setup_axis_missing()) choose_supporter();
    if (use_legacy_star()) {
      // Core procedure source: https://www.pokemon.com/us/pokemon-tcg/rules
      // Recovered Items may be played later in the same turn.
      play_chaotic_swell();
      play_field_blower();
      play_basics_from_hand();
      play_items_until_stable(true);
      // Legacy Star can return a Supporter from discard, including Crispin:
      // https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/sv7-133
      // A player may use one available Supporter during the turn before attacking:
      // https://www.pokemon.com/us/pokemon-tcg/rules
      if (!state_.supporter_used && setup_axis_missing()) choose_supporter();
    }
    attach_manual();
    retreat_to_benched_vstar_with_latias();
    if (need_active_vstar() && !state_.supporter_used) play_tate_switch();
    // Attach only after the final modeled switch decision, while the intended holder
    // is known to remain Active for Powerglass's end-of-turn condition:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    attach_powerglass();
    use_celestial_roar();
    trace("POLICY", "P-AXIS-01", "End: " + state_line());
  }

#endif
