  bool issue_1873_blender_crispin_preempts_gladion() const {
    if (!strict_payload_timing() || !supporter_allowed() || item_locked() ||
        !deck_seen_ || !prizes_revealed_ || !active_is_vstar() ||
        !need_energy() || !need_payload() ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        !has_live_blender_payload_line() ||
        !crispin_can_advance_energy_axis()) {
      return false;
    }

    // Project only the visible held route. Crispin uses the one Supporter play to
    // complete GGF, then the still-held ACE SPEC establishes the same-turn Dragon
    // payload during the later Item fixed point. This strict-JIT supporter policy
    // remains disjoint from the matchup-flex Treasure-cost selector in issue 1876:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Official Supporter, Item, search, discard, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, Supporter contention, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1873 https://github.com/FlareZ123/pokemon-sims/issues/1876
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
    if (!issue_1873_blender_crispin_preempts_gladion()) {
      choose_supporter_issue1873_original();
      return;
    }

    // Spend the Supporter action on the Energy axis that completes readiness this
    // turn. Brilliant Blender remains protected for the following Item pass:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Official one-Supporter-per-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1873
    if (!play_crispin()) {
      throw std::logic_error("Issue-1873 Crispin route became unavailable");
    }
  }
