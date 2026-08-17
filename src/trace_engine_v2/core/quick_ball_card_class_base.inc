#ifndef SIM_TRACE_QUICK_BALL_CARD_CLASS_BASE_INCLUDED
#define SIM_TRACE_QUICK_BALL_CARD_CLASS_BASE_INCLUDED

// Card-class-backed Quick Ball base implementation. The historical part_009b1/2
// implementation remains compiled under a dormant compatibility name so all helper
// policy stays available while the live base routes printed resolution through
// cards::QuickBall.
// Single-inclusion boundary: https://eel.is/c++draft/cpp.include
// Confirmed composition defect: https://github.com/FlareZ123/pokemon-sims/issues/3341

  // A played Trainer is outside ordinary hand while its effect resolves and reaches
  // discard only after use. Keep this Engine-owned zone generic for later Trainer
  // migrations rather than encoding Quick Ball as a one-off state exception.
  // Item procedure B-01: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed resolving-source defect: https://github.com/FlareZ123/pokemon-sims/issues/4293
  std::vector<Card> resolving_trainer_sources_;

  rules::CardContext make_runtime_card_context() {
    return trace_engine_v2::make_card_context_adapter(
        this,
        trace_engine_v2::CardContextAdapterCallbacks{
            .hand_count = [](const void* opaque, const Card card) {
              return static_cast<const Engine*>(opaque)->hand_count(card);
            },
            .move_hand_to_discard = [](void* opaque, const Card card) {
              auto* engine = static_cast<Engine*>(opaque);
              const auto it = std::find(engine->state_.hand.begin(),
                                        engine->state_.hand.end(), card);
              if (it == engine->state_.hand.end()) return false;
              engine->state_.discard.push_back(*it);
              engine->state_.hand.erase(it);
              return true;
            },
            .discard_from_hand =
                [](void* opaque, const Card card, const std::string_view reason,
                   const std::string_view rules_reference) {
                  return static_cast<Engine*>(opaque)->discard_from_hand(
                      card, reason, rules_reference);
                },
            .search_deck_to_hand = [](void* opaque, const Card card) {
              return static_cast<Engine*>(opaque)->move_deck_to_hand(card);
            },
            .shuffle_deck = [](void* opaque) {
              auto* engine = static_cast<Engine*>(opaque);
              engine->shuffle(engine->state_.deck);
            },
            .is_basic_pokemon =
                [](const void*, const Card card) { return is_basic(card); },
            .begin_deck_search = [](void* opaque, const std::string_view reason) {
              static_cast<Engine*>(opaque)->record_deck_search_knowledge(reason);
            },
            .resolving_source =
                rules::CardContext::ResolvingSourceCallbacks{
                    .resolving_count = [](const void* opaque, const Card card) {
                      const auto* engine = static_cast<const Engine*>(opaque);
                      return static_cast<int>(std::count(
                          engine->resolving_trainer_sources_.begin(),
                          engine->resolving_trainer_sources_.end(), card));
                    },
                    .move_hand_to_resolving = [](void* opaque, const Card card) {
                      auto* engine = static_cast<Engine*>(opaque);
                      const auto it = std::find(engine->state_.hand.begin(),
                                                engine->state_.hand.end(), card);
                      if (it == engine->state_.hand.end()) return false;
                      engine->resolving_trainer_sources_.push_back(*it);
                      engine->state_.hand.erase(it);
                      return true;
                    },
                    .move_resolving_to_discard = [](void* opaque, const Card card) {
                      auto* engine = static_cast<Engine*>(opaque);
                      const auto it = std::find(
                          engine->resolving_trainer_sources_.begin(),
                          engine->resolving_trainer_sources_.end(), card);
                      if (it == engine->resolving_trainer_sources_.end()) return false;
                      engine->state_.discard.push_back(*it);
                      engine->resolving_trainer_sources_.erase(it);
                      return true;
                    }}});
  }

  cards::QuickBall::Resolution resolve_quick_ball_card_action(
      const Card discard, void* search_context,
      const cards::QuickBall::SearchTargetSelector selector,
      const std::string_view cost_reason = "Quick Ball cost",
      const std::string_view rules_reference = "R-QB-01",
      const std::string_view search_reason = "Quick Ball") {
    auto context = make_runtime_card_context();
    return cards::QuickBall::resolve(
        context,
        cards::QuickBall::Action{.discard = discard,
                                 .search_context = search_context,
                                 .choose_search_target = selector,
                                 .cost_reason = cost_reason,
                                 .rules_reference = rules_reference,
                                 .search_reason = search_reason});
  }

  struct QuickBallFixedSearchTarget {
    Card target;
  };

  static std::optional<Card> select_quick_ball_fixed_target(void* opaque) {
    return static_cast<QuickBallFixedSearchTarget*>(opaque)->target;
  }

  struct QuickBallBaseSearchPlan {
    Engine* engine;
    bool want_regi;
    bool want_latias;
    bool want_tapu;
    bool want_oricorio;
    bool want_crobat;
    bool want_payload;
  };

  static bool quick_ball_base_target_is_preferred(
      const QuickBallBaseSearchPlan& plan, const Card target) {
    // Strategy preference only; cards::QuickBall enforces the printed Basic target
    // filter during resolution. Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    return (target == Card::RegidragoV && plan.want_regi) ||
           (target == Card::LatiasEx && plan.want_latias) ||
           (target == Card::TapuLeleGX && plan.want_tapu) ||
           (target == Card::Oricorio && plan.want_oricorio) ||
           (target == Card::CrobatV && plan.want_crobat) ||
           (target == Card::DialgaGX && plan.want_payload);
  }

  static std::optional<Card> select_quick_ball_base_target(void* opaque) {
    auto& plan = *static_cast<QuickBallBaseSearchPlan*>(opaque);
    Engine& engine = *plan.engine;

    const std::array<Card, 6> targets{
        Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX,
        Card::Oricorio, Card::CrobatV, Card::DialgaGX};
    for (const Card target : targets) {
      if (quick_ball_base_target_is_preferred(plan, target) &&
          std::find(engine.state_.deck.begin(), engine.state_.deck.end(), target) !=
              engine.state_.deck.end()) {
        return target;
      }
    }

    // Preserve the legacy post-inspection Gladion fallback. The choice is made only
    // after QuickBall::resolve establishes deck-search knowledge, so K0/K1 timing is
    // unchanged by the card-class migration.
    const bool prized_regi_axis =
        engine.deck_count_after_search_started(Card::Gladion) > 0 &&
        ((engine.need_regi() &&
          engine.deck_count_after_search_started(Card::RegidragoV) == 0 &&
          engine.prize_count_after_reveal(Card::RegidragoV) > 0) ||
         (engine.need_vstar() &&
          engine.deck_count_after_search_started(Card::RegidragoVstar) == 0 &&
          engine.prize_count_after_reveal(Card::RegidragoVstar) > 0));
    if (prized_regi_axis && engine.bench_space() > 0 &&
        engine.hand_count(Card::TapuLeleGX) == 0 &&
        !engine.in_play(Card::TapuLeleGX) &&
        std::find(engine.state_.deck.begin(), engine.state_.deck.end(),
                  Card::TapuLeleGX) != engine.state_.deck.end()) {
      return Card::TapuLeleGX;
    }

    const Card fallback = engine.fallback_quick_ball_target_after_search_started();
    return std::find(engine.state_.deck.begin(), engine.state_.deck.end(), fallback) !=
                   engine.state_.deck.end()
        ? std::optional<Card>{fallback}
        : std::nullopt;
  }

  bool play_quick_ball_empty_deck_original(const bool permit_payload) {
    if (item_locked() || hand_count(Card::QuickBall) == 0) return false;

    // Keep strategy/policy selection in Engine. QuickBall::resolve receives only
    // the already-selected cost and a post-inspection target selector.
    const bool can_pay_payload_cost =
        permit_payload && can_play_payload_this_turn() &&
        blender_vstar_card_axis_can_finish_this_turn() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);

    const bool search_has_target = prizes_known()
        ? std::any_of(state_.deck.begin(), state_.deck.end(), is_basic)
        : std::any_of(recipe_.begin(), recipe_.end(), [this](const auto& entry) {
            return is_basic(entry.first) && might_be_unseen(entry.first);
          });
    if (!search_has_target) return false;

    const bool want_regi =
        need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV);
    const bool want_latias =
        (need_active_vstar() || quick_ball_seed23_latias_route_ready()) &&
        bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
        might_be_unseen(Card::LatiasEx);
    // The live card-class route uses #3458's shared semantic connector contract.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    const bool issue_1797_tapu_route =
        issue_3458_1797_quick_ball_tapu_steven_route_available();
    const bool want_tapu =
        hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
        bench_space() > 0 &&
        (needs_tapu_connector() || issue_1797_tapu_route) &&
        might_be_unseen(Card::TapuLeleGX);
    const bool want_oricorio =
        hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) &&
        bench_space() > 0 && needs_oricorio_connector() &&
        might_be_unseen(Card::Oricorio);
    const bool want_crobat =
        hand_count(Card::CrobatV) == 0 && bench_space() > 0 &&
        !state_.dark_asset_used && ability_available_for_pokemon(Card::CrobatV) &&
        state_.hand.size() <= 7U && might_be_unseen(Card::CrobatV) &&
        !want_regi && !want_latias && !want_tapu && !want_oricorio;

    const bool payload_already_held =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // The class owns Quick Ball's printed discard/search transaction, while Engine
    // owns route selection. Call the named #1237 outlet policy directly so this K1
    // Dialga-GX -> Earthen Vessel continuation no longer depends on the legacy
    // preprocessor alias that intentionally ends at part_009b2.inc.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // K1 and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed policy-dispatch bug: https://github.com/FlareZ123/pokemon-sims/issues/3348
    const bool want_payload =
        need_payload() && !payload_already_held &&
        might_be_unseen(Card::DialgaGX) && !can_pay_payload_cost &&
        has_payable_payload_outlet_after_costed_search_issue_1237(Card::QuickBall);
    const bool has_preferred_search_target =
        want_regi || want_latias || want_tapu || want_oricorio ||
        want_crobat || want_payload;
    const bool can_use_payload_only_outlet =
        can_pay_payload_cost && blender_energy_axis_can_finish_this_turn();
    if (!has_preferred_search_target && !can_use_payload_only_outlet) return false;

    std::optional<Card> cost;
    if (issue_1797_tapu_route) {
      // Preserve #1797's confirmed dynamic-DCI payment rule while removing the
      // historical seed coordinates from route admission. Tate & Liza is spent
      // before Lusamine; the VSTAR and two Grass remain protected for the bank and
      // finish sequence.
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
      // Lusamine: https://api.pokemontcg.io/v2/cards/sm4-96
      // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
      if (hand_count(Card::TateLiza) > 0) {
        cost = Card::TateLiza;
      } else if (hand_count(Card::Lusamine) > 0) {
        cost = Card::Lusamine;
      }
    }
    if (!cost) {
      // `choose_discard_issue1876` is the current named top of the established
      // Quick Ball discard-policy chain. It keeps #1869's Tapu/Burnet cost first,
      // then falls through #1740 and the #1237 wrapper instead of binding to the
      // generic discard selector after the historical macro lifetime ends.
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Tapu Lele-GX / Professor Burnet: https://api.pokemontcg.io/v2/cards/sm2-60 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Dialga-GX / Earthen Vessel: https://api.pokemontcg.io/v2/cards/sm5-100 https://api.pokemontcg.io/v2/cards/sv4-163
      // Current-turn JIT and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed policy-dispatch bug: https://github.com/FlareZ123/pokemon-sims/issues/3348
      cost = choose_discard_issue1876(
          can_pay_payload_cost, true, true, Card::QuickBall);
    }
    if (!cost && want_latias) {
      cost = quick_ball_final_surplus_energy_cost();
    }
    if (!cost && want_latias) {
      cost = quick_ball_latias_replaced_tate_cost();
    }
    if (!cost && want_latias &&
        issue_2343_burnet_quick_ball_latias_route_available() &&
        hand_count(Card::StevensResolve) > 0) {
      cost = Card::StevensResolve;
    }
    if (!cost && want_regi && issue_1605_arven_redundant_payload_route_) {
      cost = issue_1605_redundant_payload_cost();
    }
    if (!cost && want_regi && hand_count(Card::RegidragoVstar) >= 2) {
      cost = Card::RegidragoVstar;
    }
    if (!cost && want_regi) {
      cost = quick_ball_crispin_replaced_energy_cost();
    }
    if (!cost && want_regi) {
      cost = quick_ball_strict_deadlock_fallback_cost();
    }
    if (!cost) return false;

    QuickBallBaseSearchPlan plan{.engine = this,
                                 .want_regi = want_regi,
                                 .want_latias = want_latias,
                                 .want_tapu = want_tapu,
                                 .want_oricorio = want_oricorio,
                                 .want_crobat = want_crobat,
                                 .want_payload = want_payload};
    const cards::QuickBall::Resolution resolution =
        resolve_quick_ball_card_action(*cost, &plan,
                                       &Engine::select_quick_ball_base_target);
    if (!resolution.played) {
      throw std::logic_error("Quick Ball card-class resolution failed after legal planning");
    }
    issue_1605_arven_redundant_payload_route_ = false;

    if (resolution.found_target && resolution.search_target) {
      const bool preferred =
          quick_ball_base_target_is_preferred(plan, *resolution.search_target);
      trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM",
            preferred
                ? "Searched a Basic Pokémon: " +
                      std::string(name(*resolution.search_target)) + "."
                : "No preferred target remained; legally searched " +
                      std::string(name(*resolution.search_target)) + ".");
    } else {
      trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM",
            "No Basic Pokémon remained after search.");
    }
    return true;
  }

#endif  // SIM_TRACE_QUICK_BALL_CARD_CLASS_BASE_INCLUDED