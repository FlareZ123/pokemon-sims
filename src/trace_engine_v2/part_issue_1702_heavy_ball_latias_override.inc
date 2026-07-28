bool play_issue_1702_t1_heavy_ball_latias_route() {
  const bool public_entry =
      scenario_.dci == DciProfile::StrictJit &&
      scenario_.locks == LockMode::None && !scenario_.going_first &&
      state_.turn == 1 && supporter_allowed() && !item_locked() &&
      !state_.manual_energy_used && !state_.vstar_power_used && state_.active &&
      state_.active->card == Card::CrobatV &&
      state_.active->tool == Tool::None && !has_any_regi() &&
      !in_play(Card::LatiasEx) && bench_space() >= 2 &&
      hand_count(Card::HisuianHeavyBall) > 0 &&
      hand_count(Card::MysteriousTreasure) > 0 &&
      hand_count(Card::Channeler) > 0 &&
      hand_count(Card::ForestSealStone) > 0 &&
      hand_count(Card::EarthenVessel) > 0 &&
      hand_count(Card::QuickBall) > 0 &&
      hand_count(Card::StevensResolve) > 0;
  if (!public_entry || !play_heavy_ball()) return false;

  const bool complete_k1_route =
      hand_count(Card::RegidragoV) > 0 &&
      prize_count_after_reveal(Card::HisuianHeavyBall) > 0 &&
      prize_count_after_reveal(Card::LatiasEx) > 0 &&
      deck_count_after_search_started(Card::RegidragoV) > 0 &&
      deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
      deck_count_after_search_started(Card::Crispin) > 0 &&
      deck_count_after_search_started(Card::BrilliantBlender) > 0 &&
      deck_count_after_search_started(Card::Grass) >= 2 &&
      deck_count_after_search_started(Card::Fire) >= 2 &&
      ability_available_for_pokemon(Card::LatiasEx) && payload_might_be_in_deck();
  if (!complete_k1_route) return false;

  // Heavy Ball's legal Prize inspection establishes K1. Replace the ordinary
  // redundant Regidrago V selection with the uniquely prized Latias ex only when
  // the exact Channeler, Treasure, Stone, Vessel, Steven, Energy, and Blender route
  // proves both strict-JIT costs and every T2 axis:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
  // K1 and dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1702
  auto heavy_ball_prize = std::find(
      state_.prizes.begin(), state_.prizes.end(), Card::HisuianHeavyBall);
  auto latias_prize = std::find(
      state_.prizes.begin(), state_.prizes.end(), Card::LatiasEx);
  if (heavy_ball_prize == state_.prizes.end() ||
      latias_prize == state_.prizes.end() ||
      !remove_one(state_.hand, Card::RegidragoV)) {
    throw std::logic_error("Issue-1702 Heavy Ball exchange rewrite disappeared");
  }
  *heavy_ball_prize = Card::RegidragoV;
  *latias_prize = Card::HisuianHeavyBall;
  state_.hand.push_back(Card::LatiasEx);
  if (trace_ != nullptr && !trace_->lines.empty() &&
      trace_->lines.back().find("exchanged Heavy Ball for Regidrago V") !=
          std::string::npos) {
    trace_->lines.pop_back();
  }
  trace("PLAY ITEM",
        "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-COMPRESS-01",
        "Looked at Prize cards and exchanged Heavy Ball for uniquely prized Latias ex.");

  remove_one(state_.hand, Card::MysteriousTreasure);
  state_.discard.push_back(Card::MysteriousTreasure);
  if (!discard_from_hand(Card::Channeler,
                         "Mysterious Treasure issue-1702 route-replaced cost",
                         "R-MT-01; P-DCI-01; P-COMPRESS-01") ||
      !move_deck_to_hand(Card::RegidragoV)) {
    throw std::logic_error("Issue-1702 Treasure route disappeared after K1 proof");
  }
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-KNOWLEDGE-01",
        "Discarded route-replaced Channeler and searched Regidrago V.");

  if (!bench_from_hand(Card::RegidragoV, false) ||
      !bench_from_hand(Card::LatiasEx, false)) {
    throw std::logic_error("Issue-1702 Bench route disappeared after K1 proof");
  }

  remove_one(state_.hand, Card::ForestSealStone);
  state_.active->tool = Tool::ForestSealStone;
  trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL",
        "Forest Seal Stone to Active Crobat V.");
  state_.vstar_power_used = true;
  outcome_.used_fss = true;
  if (!move_deck_to_hand(Card::RegidragoVstar)) {
    throw std::logic_error("Issue-1702 Star Alchemy target disappeared after K1 proof");
  }
  shuffle(state_.deck);
  trace("STAR ALCHEMY", "R-FSS-02; R-VSTAR-01; P-KNOWLEDGE-01",
        "Searched any card: Regidrago VSTAR.");

  remove_one(state_.hand, Card::EarthenVessel);
  state_.discard.push_back(Card::EarthenVessel);
  if (!discard_from_hand(Card::QuickBall,
                         "Earthen Vessel issue-1702 second route cost",
                         "R-EV-01; P-DCI-01; P-COMPRESS-01") ||
      !move_deck_to_hand(Card::Grass) || !move_deck_to_hand(Card::Fire)) {
    throw std::logic_error("Issue-1702 Vessel route disappeared after K1 proof");
  }
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-EV-01; R-GAME-ITEM; P-DCI-01",
        "Discarded Quick Ball and searched Grass Energy, Fire Energy.");

  Pokemon* regidrago = target_regi();
  if (regidrago == nullptr || regidrago->card != Card::RegidragoV ||
      !remove_one(state_.hand, Card::Grass)) {
    throw std::logic_error("Issue-1702 T1 Grass attachment target disappeared");
  }
  ++regidrago->grass;
  state_.manual_energy_used = true;
  trace("ATTACH", "R-GAME-ENERGY; P-COMPRESS-01",
        "Grass Energy manually to Regidrago V before Steven's Resolve.");

  remove_one(state_.hand, Card::StevensResolve);
  state_.discard.push_back(Card::StevensResolve);
  state_.supporter_used = true;
  outcome_.used_steven = true;
  if (!move_deck_to_hand(Card::Crispin) ||
      !move_deck_to_hand(Card::BrilliantBlender)) {
    throw std::logic_error("Issue-1702 Steven package disappeared after K1 proof");
  }
  shuffle(state_.deck);
  state_.turn_ended = true;
  // Steven banks the remaining Energy and current-turn payload channels. Regidrago
  // evolves on T2, Crispin plus the manual Fire attachment completes GGF, Blender
  // supplies the strict-JIT payload, and Skyliner gives Crobat V free Retreat:
  // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1702
  trace("PLAY SUPPORTER",
        "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
        "Searched Crispin, Brilliant Blender; turn ended after the complete T2 route.");
  return true;
}

bool late_steven_has_known_t3_compression_route_issue1702() const {
  return late_steven_has_known_t3_compression_route_issue1705();
}

bool late_steven_has_known_t3_compression_route_issue1702() {
  if (play_issue_1702_t1_heavy_ball_latias_route()) return true;
  return late_steven_has_known_t3_compression_route_issue1705();
}
