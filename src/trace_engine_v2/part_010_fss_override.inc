  bool fss_should_take_crispin_over_redundant_blender() const {
    // Full Prize inspection establishes the same K1 composition knowledge as a legal deck search:
    // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Official Prize and search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2013
    if (!prizes_known() || !need_energy() || !supporter_allowed() ||
        state_.manual_energy_used || !crispin_can_advance_energy_axis() ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        !has_live_one_discard_hand_payload_line()) {
      return false;
    }

    // DDE makes Grass and Fire alternative one-card completions; do not add the
    // typed helpers as independent deficits. https://github.com/FlareZ123/pokemon-sims/issues/2427
    const Pokemon* regi = target_regi();
    const bool held_manual_already_completes_energy = regi != nullptr &&
        completing_basic_energy_for(*regi, [this](const Card card) {
          return hand_count(card) > 0;
        }).has_value();
    if (held_manual_already_completes_energy) return false;

    const bool grass_available = deck_count_after_search_started(Card::Grass) > 0;
    const bool fire_available = deck_count_after_search_started(Card::Fire) > 0;
    const bool crispin_completes_energy = regi != nullptr &&
        (minimum_basic_attachments_to_apex(
             *regi, grass_available ? 1 : 0, fire_available ? 1 : 0) <=
             (grass_available && fire_available ? 2 : 1));
    if (!crispin_completes_energy) return false;
    const bool vstar_can_exist = regi != nullptr &&
        (regi->card == Card::RegidragoVstar ||
         (regi->card == Card::RegidragoV && regi->entered_turn < state_.turn &&
          hand_count(Card::RegidragoVstar) > 0));
    const bool vstar_can_be_active = state_.active &&
        (state_.active->card == Card::RegidragoV ||
         state_.active->card == Card::RegidragoVstar ||
         can_free_retreat_with_latias());

    // A held target-legal one-discard Item can put the Dragon payload into discard
    // during the ready turn. When Crispin plus the unused manual attachment also
    // completes GGF, Star Alchemy should search the direct Energy connector instead
    // of the unused one-copy ACE SPEC:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/976
    return vstar_can_exist && vstar_can_be_active;
  }

  bool fss_should_hold_for_complete_quick_ball_route() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2097

    const bool powered_prior_turn_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pays_apex_energy_cost(pokemon);
        });
    const bool held_payload =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);

    // Arven has already consumed the Supporter play in this exact K1 window. A held
    // VSTAR, Quick Ball plus Dragon cost, deck-resident Latias ex, and the completed
    // GGF Regidrago line solve evolution, strict-JIT payload, and Active position.
    // The held one-copy Blender confirms a second complete payload route, so searching
    // an unusable Burnet loses the once-per-game Star Alchemy without advancing setup.
    // The VSTAR-card and Active-position axes are already represented by held VSTAR,
    // the prior-turn Benched Regidrago V, and the Latias route. Their generic missing-
    // axis helpers therefore read false before those held cards resolve:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/979
    return prizes_known() && !supporter_allowed() && !item_locked() &&
        !state_.retreat_used && can_play_payload_this_turn() &&
        need_payload() && !need_energy() && powered_prior_turn_regi &&
        bench_space() > 0 && state_.active && is_basic(state_.active->card) &&
        ability_available_for_pokemon(Card::LatiasEx) && !in_play(Card::LatiasEx) &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 && hand_count(Card::QuickBall) > 0 &&
        held_payload && hand_count(Card::BrilliantBlender) > 0 &&
        deck_count_after_search_started(Card::BrilliantBlender) == 0;
  }

  bool fss_should_hold_for_observable_complete_route() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2080

    if (!prizes_known() || state_.vstar_power_used) return false;

    // Project the remainder of the current turn with Star Alchemy unavailable and
    // with an isolated RNG copy. The shadow begins from the exact public K1 state,
    // so every paid search, discard, evolution, attachment, and promotion must remain
    // legal without adding the selector's proposed target:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/990
    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    projected.state_.vstar_power_used = true;
    const auto entry_connector_count = [](const State& candidate) {
      const auto is_entry_connector = [](const Card card) {
        return card == Card::TapuLeleGX || card == Card::Oricorio;
      };
      const std::size_t active_count = candidate.active &&
              is_entry_connector(candidate.active->card)
          ? 1U
          : 0U;
      return active_count + static_cast<std::size_t>(std::count_if(
          candidate.bench.begin(), candidate.bench.end(),
          [is_entry_connector](const Pokemon& pokemon) {
            return is_entry_connector(pokemon.card);
          }));
    };
    const std::size_t current_entry_connector_count =
        entry_connector_count(state_);
    projected.run_turn();

    // Preserve established direct Star Alchemy routes when the shadow would replace
    // that one-shot resource by Bench-spending a new entry-Ability connector. Tapu
    // Lele-GX consumes Wonder Tag and exposes a two-Prize support Pokémon. Oricorio
    // consumes Vital Dance and a Bench slot. Those are alternative scarce routes,
    // rather than proof that the held non-connector cards complete every axis:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Existing direct Crispin route: https://github.com/FlareZ123/pokemon-sims/issues/976
    // Existing Oricorio compression route: https://github.com/FlareZ123/pokemon-sims/issues/187
    const bool spends_additional_entry_connector =
        entry_connector_count(projected.state_) > current_entry_connector_count;

    return !spends_additional_entry_connector && projected.active_is_vstar() &&
        projected.pays_apex_energy_cost(*projected.state_.active) &&
        projected.payload_ready();
  }

  bool fss_should_bank_tate_for_next_turn_blender_route() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2088

    const bool powered_prior_turn_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pays_apex_energy_cost(pokemon);
        });
    const bool latias_promotion_live_now = state_.active &&
        is_basic(state_.active->card) && !state_.retreat_used &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        (in_play(Card::LatiasEx) ||
         (bench_space() > 0 &&
          (hand_count(Card::LatiasEx) > 0 ||
           deck_count_after_search_started(Card::LatiasEx) > 0)));

    // Arven has already consumed this turn's Supporter play. When a prior-turn
    // Benched Regidrago V already carries GGF and Regidrago VSTAR is held, Star
    // Alchemy may bank Tate & Liza for the next modeled turn. Tate then promotes
    // the evolved attacker while held Brilliant Blender independently supplies
    // the strict-JIT payload. Professor Burnet duplicates Blender's axis and would
    // consume the same next-turn Supporter play. Reject the route when Latias ex
    // remains available for a faster current-turn promotion, an Item lock disables
    // Blender, or any required evolution, Energy, payload, or horizon axis is absent:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, Item, evolution, switching, and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest legal route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // T5 diagnostic recovery: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/T5_FAILURE_POLICY.md
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1139
    return prizes_known() && strict_payload_timing() && !supporter_allowed() &&
        !item_locked() && state_.turn < scenario_.max_turn &&
        state_.active &&
        state_.active->card != Card::RegidragoV &&
        state_.active->card != Card::RegidragoVstar && !state_.retreat_used &&
        !has_vstar() && powered_prior_turn_benched_regi &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::BrilliantBlender) > 0 &&
        deck_count_after_search_started(Card::TateLiza) > 0 &&
        payload_might_be_in_deck() && !latias_promotion_live_now;
  }

  bool fss_should_take_grass_for_seed23_latias_burnet_route() const {
    // Historical name retained for regression compatibility. The route is
    // state-semantic: Star Alchemy searches one Grass, Crispin searches a second
    // Grass plus Fire and attaches one, and the unused manual attachment supplies
    // the other current-turn Grass. Quick Ball replaces Tate & Liza with Latias ex.
    // This override is only an arbitration against the historical duplicate-Crispin
    // search. If no second Crispin is searchable, the ordinary selector may retain
    // Star Alchemy or choose another stronger route instead of spending it on Grass.
    // On the projected next turn the established Regidrago V can evolve, the held
    // Fire attachment completes GGF, and Professor Burnet supplies the JIT payload.
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced evolution, Item, Supporter, attachment, and Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1, shared JIT timing, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI/AMR approximation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Original route and state-generic paired Quick Ball route: https://github.com/FlareZ123/pokemon-sims/issues/1403 https://github.com/FlareZ123/pokemon-sims/issues/2704
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2891
    const int projected_ready_turn = state_.turn + 1;
    if (!prizes_known() || !strict_payload_timing() ||
        scenario_.max_turn < projected_ready_turn ||
        !supporter_allowed() || state_.manual_energy_used ||
        state_.retreat_used || item_locked() || !state_.active ||
        !is_basic(state_.active->card) || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        std::count(state_.deck.begin(), state_.deck.end(), Card::LatiasEx) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        hand_count(Card::QuickBall) == 0 || hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        std::count(state_.deck.begin(), state_.deck.end(), Card::Grass) < 2 ||
        std::count(state_.deck.begin(), state_.deck.end(), Card::Fire) == 0 ||
        !std::any_of(state_.deck.begin(), state_.deck.end(), is_payload)) {
      return false;
    }

    if (state_.active->card == Card::RegidragoV ||
        state_.active->card == Card::RegidragoVstar) {
      return false;
    }

    return std::any_of(
        state_.bench.begin(), state_.bench.end(),
        [this, projected_ready_turn](const Pokemon& pokemon) {
          // A Regidrago V already in play before the projected ready turn may
          // legally evolve on that next turn. Literal seed turn numbers do not
          // create an evolution requirement:
          // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
          // Advanced evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2891
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < projected_ready_turn &&
              total_energy_units(pokemon) == 0;
        });
  }

  Card fss_target_after_search_started() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2080

    const bool appletun_burnet_bank = prizes_known() && !supporter_allowed() &&
        !state_.vstar_power_used && state_.turn < scenario_.max_turn &&
        need_payload() && !need_regi() && !need_vstar() && !need_energy() &&
        !need_active_vstar() && can_play_payload_this_turn() &&
        hand_count(Card::ProfessorBurnet) == 0 &&
        deck_count_after_search_started(Card::Appletun) > 0 &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0;
    // Arven has consumed this turn's Supporter play while every setup axis except
    // the Appletun payload is complete. Star Alchemy should bank Professor Burnet
    // for the next modeled turn, where Burnet can search and discard Appletun for
    // Apex Dragon. This remains legal through Item lock and Rule Box Ability lock:
    // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR and Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Tool, Supporter, search, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Forest Seal Stone ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1293
    if (appletun_burnet_bank) return Card::ProfessorBurnet;

    if (fss_should_bank_tate_for_next_turn_blender_route()) return Card::TateLiza;

    // Star Alchemy can search Tate & Liza to promote a GGF-complete Benched
    // Regidrago VSTAR. A payable held one-discard Item can establish the strict-JIT
    // payload afterward, provided Tate leaves at least one card for that Item's
    // required deck search:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_tate_blender_tate_override.inc#L1-L22
    const bool tate_held_payload_route = need_active_vstar() &&
        benched_vstar_promotion_ready() && !can_free_retreat_with_latias() &&
        supporter_allowed() && need_payload() && state_.deck.size() > 1U &&
        has_live_one_discard_hand_payload_line() &&
        deck_count_after_search_started(Card::TateLiza) > 0;
    if (tate_held_payload_route) return Card::TateLiza;

    if (fss_should_hold_for_complete_quick_ball_route()) {
      // Brilliant Blender is already held and is a singleton. Returning that absent
      // K1 target lets the active Forest Seal Stone guard retain Star Alchemy because
      // no searched card is needed to advance the complete current-turn route:
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_011_fss_latias_override.inc#L89-L100
      // https://github.com/FlareZ123/pokemon-sims/issues/979
      return Card::BrilliantBlender;
    }

    if (fss_should_take_grass_for_seed23_latias_burnet_route()) return Card::Grass;
    if (fss_should_take_crispin_over_redundant_blender()) return Card::Crispin;

    const Card original = fss_target_after_search_started_original();

    // At K1, a held Dragon payload plus a target-legal one-discard Item is a
    // complete current-turn payload route. Prefer the direct Item over fallback
    // Arven or Serena while preserving direct deck-payload routes such as Blender:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Strict-JIT contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool held_payload_is_only_missing_axis = prizes_known() && need_payload() &&
        !need_regi() && !need_vstar() && !need_energy() && !need_active_vstar() &&
        can_play_payload_this_turn() && !item_locked() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (held_payload_is_only_missing_axis && original != Card::BrilliantBlender &&
        original != Card::ProfessorBurnet) {
      if (deck_count_after_search_started(Card::MysteriousTreasure) > 0 &&
          std::any_of(state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic)) {
        return Card::MysteriousTreasure;
      }
      if (deck_count_after_search_started(Card::QuickBall) > 0 &&
          std::any_of(state_.deck.begin(), state_.deck.end(), is_basic)) {
        return Card::QuickBall;
      }
      if (deck_count_after_search_started(Card::EarthenVessel) > 0 &&
          (deck_count_after_search_started(Card::Grass) > 0 ||
           deck_count_after_search_started(Card::Fire) > 0)) {
        return Card::EarthenVessel;
      }
    }

    const bool needed_energy_in_deck =
        (grass_needed() == 1 && deck_count_after_search_started(Card::Grass) > 0) ||
        (fire_needed() == 1 && deck_count_after_search_started(Card::Fire) > 0);
    const bool oricorio_in_deck =
        bench_space() > 0 && !in_play(Card::Oricorio) &&
        hand_count(Card::Oricorio) == 0 &&
        deck_count_after_search_started(Card::Oricorio) > 0;
    // #616 defines the shared exact-hand test for whether a held Dragon has a legal
    // same-turn Item discard route. Star Alchemy should take Oricorio when Vital
    // Dance supplies the final Energy and preserves Professor Burnet's one Supporter
    // play for the only live deck-to-discard payload bridge:
    // Forest Seal Stone https://api.pokemontcg.io/v2/cards/swsh12-156
    // Oricorio https://api.pokemontcg.io/v2/cards/sm2-55
    // Crispin https://api.pokemontcg.io/v2/cards/sv7-133
    // Professor Burnet https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Apex Dragon https://api.pokemontcg.io/v2/cards/swsh12-136
    // Shared outlet policy https://github.com/FlareZ123/pokemon-sims/issues/616
    // Parent route https://github.com/FlareZ123/pokemon-sims/issues/187
    // Core turn limits https://www.pokemon.com/us/pokemon-tcg/rules
    // Decision priority https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool oricorio_preserves_burnet_with_stranded_payload =
        original == Card::Crispin && needed_energy_in_deck && oricorio_in_deck &&
        oricorio_preserves_burnet_with_current_hand();
    if (oricorio_preserves_burnet_with_stranded_payload) return Card::Oricorio;

    if (original != Card::Crispin || supporter_allowed()) return original;

    // Crispin is a Supporter, so it cannot resolve after Arven has already used the
    // turn's one Supporter play. Star Alchemy may instead take any card, including
    // the missing Basic Energy for the still-unused manual attachment:
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (need_energy() && can_fetch_oricorio_for_compression()) return Card::Oricorio;
    if (need_energy() && deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        is_legal_cost_available(can_play_payload_this_turn(), true)) {
      return Card::EarthenVessel;
    }
    if (need_energy() && !state_.manual_energy_used) {
      if (grass_needed() > 0 && deck_count_after_search_started(Card::Grass) > 0) return Card::Grass;
      if (fire_needed() > 0 && deck_count_after_search_started(Card::Fire) > 0) return Card::Fire;
    }
    if (deck_count_after_search_started(Card::Arven) > 0) return Card::Arven;
    if (deck_count_after_search_started(Card::MysteriousTreasure) > 0) return Card::MysteriousTreasure;
    if (deck_count_after_search_started(Card::QuickBall) > 0) return Card::QuickBall;
    return Card::Serena;
  }
