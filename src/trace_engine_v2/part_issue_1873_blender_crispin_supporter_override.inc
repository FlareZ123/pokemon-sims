  bool tate_switch_then_next_turn_burnet_route() const {
    if (!prizes_known() || !supporter_allowed() || !strict_payload_timing() ||
        state_.turn >= scenario_.max_turn || hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 || need_regi() || need_vstar() ||
        need_energy() || !need_active_vstar() || !need_payload() ||
        !benched_vstar_promotion_ready() || can_free_retreat_with_latias()) {
      return false;
    }

    // Tate & Liza can resolve the only missing board axis now. Professor Burnet
    // remains in hand for the next turn, where it can search and discard a Dragon
    // during the same turn that the already Active GGF Regidrago VSTAR is ready.
    // Project exactly that visible two-Supporter sequence without reading a future
    // draw, hidden deck order, opponent action, or unknown Prize identity:
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // One-Supporter-per-turn, switching, and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, supporter contention, strict JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2154
    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    projected.trace_ = nullptr;
    if (!projected.play_tate_switch()) return false;

    ++projected.state_.turn;
    projected.state_.supporter_used = false;
    projected.state_.manual_energy_used = false;
    projected.state_.retreat_used = false;
    projected.state_.stadium_used = false;
    projected.state_.dark_asset_used = false;
    projected.state_.turn_ended = false;
    projected.state_.discarded_this_turn.clear();
    return projected.professor_burnet_has_live_ready_turn_route();
  }

  bool issue_1873_blender_crispin_preempts_gladion() const {
    if (!strict_payload_timing() || !supporter_allowed() || item_locked() ||
        !prizes_known() || !active_is_vstar() || !need_energy() ||
        !need_payload() || hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        !has_live_blender_payload_line() ||
        !crispin_can_advance_energy_axis()) {
      return false;
    }

    // Project the complete visible route before spending the only Supporter play.
    // Public K1 may come from a legal deck inspection or a complete Prize
    // inspection, so this uses the repository-wide prizes_known() abstraction:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Official Supporter, Item, search, discard, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, Supporter contention, dynamic DCI, strict JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1873
    const std::mt19937_64 live_rng = rng_;
    Engine projected = *this;
    projected.trace_ = nullptr;
    const bool available = projected.play_crispin() &&
        projected.play_brilliant_blender() &&
        projected.active_is_vstar() &&
        projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 && projected.payload_ready();
    rng_ = live_rng;
    return available;
  }

  void choose_supporter() {
    if (tate_switch_then_next_turn_burnet_route()) {
      // The current Supporter action repairs Active position. Burnet remains held
      // for the following turn's same-turn payload, preserving both deterministic
      // connectors instead of shuffling them through Tate's draw mode:
      // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
      // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2154
      if (!play_tate_switch()) {
        throw std::logic_error("Issue-2154 Tate switch route became unavailable");
      }
      return;
    }
    if (!issue_1873_blender_crispin_preempts_gladion()) {
      choose_supporter_issue1873_original();
      return;
    }

    // Spend the Supporter action on the final Energy axis. The held ACE SPEC then
    // supplies the current-turn Dragon payload during the ordinary Item pass:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Official one-Supporter-per-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1873
    if (!play_crispin()) {
      throw std::logic_error("Issue-1873 Crispin route became unavailable");
    }
  }
