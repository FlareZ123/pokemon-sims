#ifndef SIM_TRACE_QUICK_BALL_CARD_CLASS_BASE_INCLUDED
#define SIM_TRACE_QUICK_BALL_CARD_CLASS_BASE_INCLUDED

// Card-class-backed Quick Ball base implementation. The historical part_009b1/2
// implementation remains compiled under a dormant compatibility name so all helper
// policy stays available while the live base routes printed resolution through
// cards::QuickBall.
// Single-inclusion boundary: https://eel.is/c++draft/cpp.include
// Confirmed composition defect: https://github.com/FlareZ123/pokemon-sims/issues/3341

  rules::CardContext make_runtime_card_context() {
    return trace_engine_v2::make_card_context_adapter(
        this,
        [](const void* opaque, const Card card) {
          return static_cast<const Engine*>(opaque)->hand_count(card);
        },
        [](void* opaque, const Card card) {
          auto* engine = static_cast<Engine*>(opaque);
          const auto it = std::find(engine->state_.hand.begin(),
                                    engine->state_.hand.end(), card);
          if (it == engine->state_.hand.end()) return false;
          engine->state_.discard.push_back(*it);
          engine->state_.hand.erase(it);
          return true;
        },
        [](void* opaque, const Card card, const std::string_view reason,
           const std::string_view rules_reference) {
          return static_cast<Engine*>(opaque)->discard_from_hand(
              card, reason, rules_reference);
        },
        [](void* opaque, const Card card) {
          return static_cast<Engine*>(opaque)->move_deck_to_hand(card);
        },
        [](void* opaque) {
          auto* engine = static_cast<Engine*>(opaque);
          engine->shuffle(engine->state_.deck);
        },
        [](const void*, const Card card) { return is_basic(card); },
        [](void* opaque, const std::string_view reason) {
          static_cast<Engine*>(opaque)->record_deck_search_knowledge(reason);
        });
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

  static std::optional<Card> select_quick_ball_base_target(void* opaque) {
    auto& plan = *static_cast<QuickBallBaseSearchPlan*>(opaque);
    Engine& engine = *plan.engine;

    const std::array<Card, 7> targets{
        Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX,
        Card::Oricorio, Card::CrobatV, Card::DialgaGX, Card::MawileGX};
    for (const Card target : targets) {
      const bool desired =
          (target == Card::RegidragoV && plan.want_regi) ||
          (target == Card::LatiasEx && plan.want_latias) ||
          (target == Card::TapuLeleGX && plan.want_tapu) ||
          (target == Card::Oricorio && plan.want_oricorio) ||
          (target == Card::CrobatV && plan.want_crobat) ||
          (target == Card::DialgaGX && plan.want_payload);
      if (desired &&
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

  static bool quick_ball_base_target_was_preferred(
      const QuickBallBaseSearchPlan& plan, const Card target) {
    return (target == Card::RegidragoV && plan.want_regi) ||
           (target == Card::LatiasEx && plan.want_latias) ||
           (target == Card::TapuLeleGX && plan.want_tapu) ||
           (target == Card::Oricorio && plan.want_oricorio) ||
           (target == Card::CrobatV && plan.want_crobat) ||
           (target == Card::DialgaGX && plan.want_payload);
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
    const bool issue_1797_tapu_route =
        issue_1797_quick_ball_tapu_steven_route_available();
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
    // The legacy Quick Ball policy aliases are intentionally released before Ultra Ball,
    // so the card-class planner calls the already-composed wrapper function explicitly.
    // Existing wrapper chain: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_discard_recovery_provenance_override.inc
    // Confirmed migration regression: https://github.com/FlareZ123/pokemon-sims/issues/3348
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
      cost = issue_1797_quick_ball_cost();
    }
    if (!cost) {
      // Preserve the complete route-conditioned Quick Ball cost chain after the
      // macro lifetime ends at the historical Ultra Ball boundary.
      // Quick Ball printed cost: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Existing wrapper chain: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_discard_recovery_provenance_override.inc
      // Confirmed migration regression: https://github.com/FlareZ123/pokemon-sims/issues/3348
      cost = choose_discard_issue_1237(
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
          quick_ball_base_target_was_preferred(plan, *resolution.search_target);
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
