#include "part_issue_1393_crispin_route_helper.inc"

  bool issue_1672_fss_vessel_latias_route_available() const {
    const auto live_fss_holder = [](const Pokemon& pokemon) {
      return pokemon.tool == Tool::ForestSealStone && is_pokemon_v(pokemon.card);
    };
    const bool forest_seal_stone_is_live = !state_.vstar_power_used &&
        ((state_.active && live_fss_holder(*state_.active)) ||
         std::any_of(state_.bench.begin(), state_.bench.end(), live_fss_holder));
    const Pokemon* setup_regi = target_regi();
    if (!forest_seal_stone_is_live ||
        scenario_.dci != DciProfile::MatchupFlexJit ||
        scenario_.locks != LockMode::None || !deck_seen_ || item_locked() ||
        !state_.supporter_used || state_.manual_energy_used ||
        state_.retreat_used || !state_.active ||
        !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV || bench_space() == 0 ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        setup_regi == nullptr || setup_regi->card != Card::RegidragoV ||
        setup_regi->entered_turn >= state_.turn ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const auto missing_energy = completing_basic_energy_for(
        *setup_regi, [this](const Card card) {
          return deck_count_after_search_started(card) > 0;
        });
    if (!missing_energy.has_value()) return false;
    // Star Alchemy is the only unrestricted connector in this public K1 state.
    // Earthen Vessel can discard the held Dragon payload, search the one missing
    // Basic Energy, and leave Star Alchemy free to find Latias ex. Skyliner then
    // gives the Basic Active no Retreat Cost after the held VSTAR evolves:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Ability, evolution, attachment, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, DCI/JIT, one-shot connector, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1672
    return deck_count_after_search_started(Card::LatiasEx) > 0;
  }

  bool issue_1356_fss_treasure_energy_split_available() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2073

    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        scenario_.max_turn < 3 || !prizes_known() || state_.vstar_power_used ||
        !state_.manual_energy_used || item_locked() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::MysteriousTreasure) < 2 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // The searched Basic is chosen by semantic Apex completion; DDE makes Grass
    // and Fire alternative legal targets. https://github.com/FlareZ123/pokemon-sims/issues/2427
    const auto star_target = completing_basic_energy_for(
        *state_.active, [this](const Card card) {
          return deck_count_after_search_started(card) > 0;
        });
    const bool held_surplus_basic = hand_count(Card::Grass) > 0 ||
        hand_count(Card::Fire) > 0;

    // Mysterious Treasure can pay the surplus off-type Energy and search the
    // missing Regidrago VSTAR. Star Alchemy is the costless connector for the
    // sole missing next-turn Basic Energy, while the second Treasure and held
    // Dragon remain the strict-JIT payload route:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, VSTAR Power, search, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356
    return star_target.has_value() && held_surplus_basic;
  }

  bool use_fss() {
    // Star Alchemy is a once-per-game VSTAR Power. If the only missing setup axis
    // is Energy and held DDE already completes Apex Dragon's GGF cost this turn,
    // searching another card cannot improve the earliest-ready turn. Scope this
    // preservation rule specifically to DDE so the temporary modeling recipe does
    // not alter canonical Basic-Energy deck policy:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Resource-preserving earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Enhancement audit: https://github.com/FlareZ123/pokemon-sims/issues/2238
    if (!state_.vstar_power_used && !state_.manual_energy_used && need_energy() &&
        !need_regi() && !need_vstar() && !need_active_vstar() && !need_payload()) {
      const Pokemon* target = target_regi();
      if (target != nullptr) {
        const auto held = preferred_manual_energy_from_hand(*target);
        if (held && *held == Card::DoubleDragonEnergy) {
          Pokemon projected = *target;
          if (attach_energy_card(projected, *held) &&
              pays_apex_energy_cost(projected)) {
            trace("HOLD VSTAR POWER", "R-FSS-02; R-GAME-ENERGY; P-COMPRESS-01",
                  "Preserved Star Alchemy because a held Energy already completes the sole missing Energy axis.");
            return false;
          }
        }
      }
    }
    const auto live_fss_holder = [](const Pokemon& pokemon) {
      return pokemon.tool == Tool::ForestSealStone && is_pokemon_v(pokemon.card);
    };
    const bool forest_seal_stone_is_live = !state_.vstar_power_used &&
        ((state_.active && live_fss_holder(*state_.active)) ||
         std::any_of(state_.bench.begin(), state_.bench.end(), live_fss_holder));
    if (forest_seal_stone_is_live &&
        issue_1393_held_crispin_completion_available()) {
      // A held Crispin can attach one searched Basic Energy and place the other
      // into hand for the unused manual attachment. When the same public K1 state
      // already contains the VSTAR card and a legal current-turn Dragon outlet,
      // Star Alchemy advances no setup axis and must remain unused:
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core Supporter, attachment, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Resource-preserving earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1393
      trace("HOLD VSTAR POWER", "R-FSS-02; R-CRISPIN-01; P-KNOWLEDGE-01; P-COMPRESS-01",
            "Retained Star Alchemy because held Crispin and the public payload outlet already complete the turn-two route.");
      return false;
    }

    if (issue_1672_fss_vessel_latias_route_available()) {
      state_.vstar_power_used = true;
      outcome_.used_fss = true;
      record_deck_search_knowledge("Star Alchemy");
      if (!move_deck_to_hand(Card::LatiasEx)) {
        throw std::logic_error("Issue-1672 Latias ex target disappeared");
      }
      shuffle(state_.deck);
      // Star Alchemy searches any one card, while the held Earthen Vessel and
      // Dragon payload cover the Energy and current-turn discard axes:
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/issues/1672
      trace("STAR ALCHEMY", "R-FSS-02; R-VSTAR-01; P-KNOWLEDGE-01; P-COMPRESS-01",
            "Searched Latias ex while Earthen Vessel retained the held Dragon payload and missing Basic Energy axes.");
      return true;
    }

    if (!issue_1356_fss_treasure_energy_split_available()) {
      return use_fss_issue1356_original();
    }

    const auto target = completing_basic_energy_for(
        *state_.active, [this](const Card card) {
          return deck_count_after_search_started(card) > 0;
        });
    if (!target.has_value()) {
      throw std::logic_error("issue-1356 semantic Star Alchemy Energy target disappeared");
    }
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    record_deck_search_knowledge("Star Alchemy");
    if (!move_deck_to_hand(*target)) {
      throw std::logic_error("projected issue-1356 Basic Energy target disappeared");
    }
    shuffle(state_.deck);
    trace("STAR ALCHEMY", "R-FSS-02; R-VSTAR-01; P-COMPRESS-01",
          "Searched the missing next-turn Basic Energy while Mysterious Treasure retained the Regidrago VSTAR axis: " +
              std::string(name(*target)) + ".");
    return true;
  }
