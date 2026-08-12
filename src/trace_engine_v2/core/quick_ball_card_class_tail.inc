// Final Quick Ball card-class bridge. Historical route-admission helpers remain in
// part_issue_2272_route_replaced_arven_quick_ball_override.inc, while every active
// Quick Ball transaction below resolves its printed Item lifecycle through
// cards::QuickBall and CardContext.

  bool play_quick_ball(const bool permit_payload) {
    if (issue_2199_quick_ball_latias_payload_route_available()) {
      const Card payload = *issue_2199_quick_ball_latias_payload_cost();
      QuickBallFixedSearchTarget target{Card::LatiasEx};
      const auto resolution = resolve_quick_ball_card_action(
          payload, &target, &Engine::select_quick_ball_fixed_target,
          "Quick Ball issue-2199 current-turn payload cost",
          "R-QB-01; P-DCI-01; P-JIT-01; P-CONNECTOR-01",
          "Quick Ball issue-2199 Latias route");
      if (!resolution.played || !resolution.found_target) {
        throw std::logic_error(
            "issue-2199 Quick Ball card-class resolution lost its Latias target");
      }
      trace("QUICK BALL", "R-QB-01; R-LATIAS-01; P-JIT-01; P-CONNECTOR-01",
            "Discarded " + std::string(name(payload)) +
                " as the current-turn payload and searched Latias ex for promotion.");
      return true;
    }

    if (issue_2289_quick_ball_tapu_arven_route_available()) {
      const Card payload = *issue_2289_quick_ball_tapu_arven_payload_cost();
      QuickBallFixedSearchTarget target{Card::TapuLeleGX};
      const auto resolution = resolve_quick_ball_card_action(
          payload, &target, &Engine::select_quick_ball_fixed_target,
          "Quick Ball issue-2289 current-turn payload cost",
          "R-QB-01; P-DCI-01; P-JIT-01; P-CONNECTOR-01",
          "Quick Ball issue-2289 Tapu-Arven route");
      if (!resolution.played || !resolution.found_target) {
        throw std::logic_error(
            "issue-2289 Quick Ball card-class resolution lost its Tapu target");
      }
      trace("PLAY ITEM", "R-QB-01; R-TAPU-01; P-JIT-01; P-CONNECTOR-01",
            "Discarded the current-turn Dragon payload and searched Tapu Lele-GX for the current-turn Arven completion route.");

      if (!bench_from_hand(Card::TapuLeleGX, false)) {
        throw std::logic_error("issue-2289 Tapu Lele-GX Bench action failed");
      }
      record_deck_search_knowledge("Wonder Tag issue-2289 Arven route");
      if (!move_deck_to_hand(Card::Arven)) {
        throw std::logic_error("issue-2289 Arven target disappeared");
      }
      shuffle(state_.deck);
      trace("WONDER TAG", "R-TAPU-01; R-ARVEN-01; P-CONNECTOR-01",
            "Searched and revealed Arven for Forest Seal Stone plus Earthen Vessel.");

      if (!remove_one(state_.hand, Card::Arven)) {
        throw std::logic_error("issue-2289 Arven disappeared before play");
      }
      state_.discard.push_back(Card::Arven);
      state_.supporter_used = true;
      record_deck_search_knowledge("Arven issue-2289 complete route");
      if (!move_deck_to_hand(Card::ForestSealStone) ||
          !move_deck_to_hand(Card::EarthenVessel)) {
        throw std::logic_error("issue-2289 Arven targets disappeared");
      }
      shuffle(state_.deck);
      trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-CONNECTOR-01",
            "Arven searched Forest Seal Stone and Earthen Vessel for the proven current-turn route.");

      if (!remove_one(state_.hand, Card::EarthenVessel)) {
        throw std::logic_error("issue-2289 Earthen Vessel disappeared before play");
      }
      state_.discard.push_back(Card::EarthenVessel);
      if (!discard_from_hand(
              Card::Fire,
              "Earthen Vessel issue-2289 route-surplus Fire cost",
              "R-EV-01; P-DCI-01; P-CONNECTOR-01")) {
        throw std::logic_error("issue-2289 Earthen Vessel Fire cost disappeared");
      }
      record_deck_search_knowledge("Earthen Vessel issue-2289 Grass route");
      if (!move_deck_to_hand(Card::Grass)) {
        throw std::logic_error("issue-2289 Grass target disappeared");
      }
      shuffle(state_.deck);
      trace("EARTHEN VESSEL", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-CONNECTOR-01",
            "Discarded route-surplus Fire Energy and searched Grass Energy for the unused manual attachment.");
      return true;
    }

    if (issue_2272_route_replaced_arven_quick_ball_available()) {
      QuickBallFixedSearchTarget target{Card::LatiasEx};
      const auto resolution = resolve_quick_ball_card_action(
          Card::Arven, &target, &Engine::select_quick_ball_fixed_target,
          "route-replaced Quick Ball cost", "R-QB-01; P-DCI-01",
          "Quick Ball issue-2272 route");
      if (!resolution.played || !resolution.found_target) {
        throw std::logic_error(
            "issue-2272 Quick Ball card-class resolution lost its Latias target");
      }
      trace("QUICK BALL", "R-QB-01; R-GAME-ITEM; P-DCI-01; P-CONNECTOR-01",
            "Discarded route-replaced Arven; searched Latias ex for the Blender-backed Skyliner finish.");
      return true;
    }

    return play_quick_ball_issue2272_original(permit_payload);
  }
