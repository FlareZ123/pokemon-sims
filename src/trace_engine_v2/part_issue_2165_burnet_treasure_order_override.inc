#define play_mysterious_treasure play_mysterious_treasure_issue2323_original
  bool issue_2165_burnet_treasure_vstar_route_available() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 1 ||
        item_locked() || state_.supporter_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        hand_count(Card::UltraBall) == 0 ||
        hand_count(Card::Dragapult) == 0 ||
        hand_count(Card::RegidragoVstar) > 0) {
      return false;
    }
    const bool tapu_benched = std::any_of(
        state_.bench.begin(), state_.bench.end(),
        [](const Pokemon& pokemon) { return pokemon.card == Card::TapuLeleGX; });
    if (!tapu_benched) return false;

    // Mysterious Treasure pays one discard and searches a Dragon or Psychic
    // Pokemon. In this public singleton-heavy hand, Ultra Ball is the redundant
    // lower-AMR connector because it requires two later discards, while Treasure
    // immediately establishes the missing Regidrago VSTAR axis and K1 before the
    // one-per-turn Supporter decision. Professor Burnet remains available for a
    // later payload decision rather than preempting the advancing Item route:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Trainer, search, discard-cost, and one-Supporter-per-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K0/K1 and earliest public route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI and AMR assumptions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2165
    return issue_1419_publicly_possible_count(Card::RegidragoVstar) > 0;
  }

  bool issue_2265_observable_direct_route_should_hold_treasure() const {
    const bool jit_profile = scenario_.dci == DciProfile::StrictJit ||
        scenario_.dci == DciProfile::MatchupFlexJit;
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const bool deck_payload = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_payload);
    const bool evolution_ready_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.double_dragon == 0 &&
              pokemon.grass >= 2 && pokemon.fire == 0;
        });

    // At K1, the held Fire, VSTAR, Arven, surplus Grass, Blender, known Quick Ball,
    // known Latias ex, and a deck Dragon prove a complete same-turn continuation.
    // Spending Mysterious Treasure before the scheduled attachment consumes an
    // extra card and can spend a Tapu Lele-GX Bench slot without improving T3.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Attachment, evolution, Supporter, Item, search, Bench, Ability, and retreat rules: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, DCI/JIT, resource preservation, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
    return jit_profile && !held_payload && deck_payload && prizes_known() &&
        supporter_allowed() && !item_locked() && !state_.manual_energy_used &&
        !state_.retreat_used && state_.active && is_basic(state_.active->card) &&
        state_.active->card != Card::RegidragoV && evolution_ready_benched_regi &&
        hand_count(Card::MysteriousTreasure) > 0 && hand_count(Card::Arven) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 && hand_count(Card::Fire) > 0 &&
        hand_count(Card::Grass) > 0 && hand_count(Card::QuickBall) == 0 &&
        hand_count(Card::BrilliantBlender) > 0 &&
        deck_count_after_search_started(Card::QuickBall) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 && bench_space() > 0 &&
        ability_available_for_pokemon(Card::LatiasEx) && !in_play(Card::LatiasEx) &&
        hand_count(Card::LatiasEx) == 0;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (issue_2265_observable_direct_route_should_hold_treasure() ||
        issue_2265_hold_treasure_for_direct_route()) {
      // Preserve Treasure until the scheduled Fire attachment and evolution have
      // changed the DCI/axis state. The direct T3 continuation is already proven.
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Core procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Earliest-route/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
      return false;
    }
    if (!issue_2165_burnet_treasure_vstar_route_available()) {
      return play_mysterious_treasure_issue2165_original(permit_payload);
    }

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::UltraBall, "Mysterious Treasure cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-2165 Ultra Ball cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");

    if (move_deck_to_hand(Card::RegidragoVstar)) {
      shuffle(state_.deck);
      trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
            "Discarded redundant Ultra Ball and searched Regidrago VSTAR before the Turn 1 Supporter decision.");
      return true;
    }

    const Card fallback = fallback_mysterious_target_after_search_started();
    const bool found_fallback = move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          found_fallback
              ? "Discarded redundant Ultra Ball; Regidrago VSTAR was unavailable after K1 inspection, so Mysterious Treasure searched " +
                    std::string(name(fallback)) + "."
              : "Discarded redundant Ultra Ball; no Dragon or Psychic Pokemon remained after K1 inspection.");
    return true;
  }
#undef play_mysterious_treasure
#include "part_issue_2323_early_treasure_regi_override.inc"
