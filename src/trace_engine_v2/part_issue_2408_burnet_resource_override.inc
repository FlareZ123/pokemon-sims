  bool issue_2408_supporter_preserving_item_payload_route() const {
    if (item_locked()) return false;

    const auto completes_payload_without_supporter = [this](const Card item) {
      std::mt19937_64 shadow_rng = rng_;
      Engine projected(scenario_, recipe_, shadow_rng);
      projected.state_ = state_;
      projected.deck_seen_ = deck_seen_;
      projected.prizes_revealed_ = prizes_revealed_;
      projected.trace_ = nullptr;

      bool played = false;
      switch (item) {
        case Card::BrilliantBlender:
          played = projected.play_brilliant_blender();
          break;
        case Card::MysteriousTreasure:
          played = projected.play_mysterious_treasure(true);
          break;
        case Card::QuickBall:
          played = projected.play_quick_ball(true);
          break;
        case Card::EarthenVessel:
          played = projected.play_earthen_vessel(true);
          break;
        case Card::UltraBall:
          played = projected.play_ultra_ball(true);
          break;
        default:
          break;
      }
      return played && projected.payload_ready() &&
          !projected.state_.supporter_used;
    };

    // Project the actual legal Item action instead of treating a generic connector
    // predicate as proof that it can establish this turn's payload. This preserves
    // the Supporter action only when the Item really pays its printed cost, has a
    // legal target/effect, and leaves a Dragon in discard for Apex Dragon.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, DCI, JIT, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Existing Item-outlet boundary: https://github.com/FlareZ123/pokemon-sims/issues/1341
    // Confirmed Burnet-vs-Serena refinement: https://github.com/FlareZ123/pokemon-sims/issues/2408
    for (const Card item : {Card::BrilliantBlender, Card::MysteriousTreasure,
                            Card::QuickBall, Card::EarthenVessel,
                            Card::UltraBall}) {
      if (hand_count(item) > 0 && completes_payload_without_supporter(item)) {
        return true;
      }
    }
    return false;
  }

  void choose_supporter() {
    const bool payload_is_only_missing_axis =
        strict_payload_timing() && need_payload() && !need_regi() &&
        !need_vstar() && !need_energy() && !need_active_vstar() &&
        can_play_payload_this_turn();
    const bool held_dragon =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool live_item_payload_outlet =
        held_dragon && issue_2408_supporter_preserving_item_payload_route();

    // At K1, Burnet and Serena can each complete the same remaining payload axis.
    // Prefer Burnet only when its deck-resident Dragon route is proven live and no
    // Item route can preserve the Supporter action. Item-outlet states continue
    // through the established selector stack so their route-specific hold policy,
    // trace provenance, and Supporter-contention controls remain authoritative.
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, search, and discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, same-turn JIT, DCI, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Existing Item-outlet boundary: https://github.com/FlareZ123/pokemon-sims/issues/1341
    // Confirmed Burnet-vs-Serena bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
    const bool burnet_preserves_serena_and_held_dragon =
        payload_is_only_missing_axis && prizes_known() && held_dragon &&
        !live_item_payload_outlet &&
        hand_count(Card::ProfessorBurnet) > 0 && hand_count(Card::Serena) > 0 &&
        professor_burnet_has_live_ready_turn_route();
    if (burnet_preserves_serena_and_held_dragon && play_professor_burnet()) return;

    choose_supporter_issue2408_original();
  }