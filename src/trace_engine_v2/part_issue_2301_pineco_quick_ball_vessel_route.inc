  bool issue_2301_banked_t4_route_{false};

  std::optional<Card> issue_2301_quick_ball_timer_cost() const {
    // Seed 38 has exactly two distinct held Apex Dragon payloads. Quick Ball may
    // spend one only because Earthen Vessel preserves a second one for the future
    // ready turn, while the T3 Basic search establishes Regidrago V's evolution
    // timer and K1 one turn earlier. Every ordinary lower-DCI cost remains preferred.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, hidden-deck search, Bench, evolution, Energy, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K0/K1, strict-JIT, dynamic DCI, and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (scenario_.dci != DciProfile::StrictJit || scenario_.going_first ||
        scenario_.locks != LockMode::None || state_.turn != 3 ||
        state_.turn + 1 > scenario_.max_turn || prizes_known() || item_locked() ||
        !supporter_allowed() || state_.manual_energy_used || state_.retreat_used ||
        state_.vstar_power_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX || !state_.bench.empty() ||
        bench_space() < 2 || state_.stadium != Stadium::ForestOfVitality ||
        hand_count(Card::QuickBall) == 0 || hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::Grass) == 0 || hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::Crispin) == 0 || hand_count(Card::SecretBox) == 0 ||
        !need_regi() || !might_be_unseen(Card::RegidragoV) ||
        !might_be_unseen(Card::ForestSealStone) || !might_be_unseen(Card::Dawn) ||
        !might_be_unseen(Card::Pineco) || !might_be_unseen(Card::ForretressEx) ||
        !might_be_unseen(Card::Grass) || !might_be_unseen(Card::Fire)) {
      return std::nullopt;
    }

    int held_payloads = 0;
    int distinct_payloads = 0;
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      const int copies = hand_count(card);
      held_payloads += copies;
      distinct_payloads += copies > 0 ? 1 : 0;
    }
    if (held_payloads != 2 || distinct_payloads != 2) return std::nullopt;

    // An already dead card, surplus Energy, or any other ordinary legal Quick Ball
    // cost must be consumed before this future-route Dragon exception.
    // Battle VIP Pass dead-card precedent: https://api.pokemontcg.io/v2/cards/swsh8-225
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (choose_discard(false, true, true, Card::QuickBall).has_value()) {
      return std::nullopt;
    }

    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_2301_k1_t4_route_proven() const {
    // Quick Ball has now legally established K1. The bank is admitted only when
    // the exact remaining zones prove every T4 resource. Three Grass in deck are
    // sufficient against every mandatory T4 draw: if Grass is drawn, that copy is
    // the Secret Box Grass cost; otherwise Vessel searches one and leaves two for
    // Exploding Energy. The held VSTAR is a Box cost, so another VSTAR must remain
    // available in deck for Star Alchemy unless the mandatory draw moves it to hand.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // K1 and future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (!prizes_known() || state_.turn != 3 || scenario_.locks != LockMode::None ||
        item_locked() || !supporter_allowed() || state_.manual_energy_used ||
        state_.retreat_used || state_.vstar_power_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX || !state_.bench.empty() ||
        bench_space() < 2 || state_.stadium != Stadium::ForestOfVitality ||
        hand_count(Card::RegidragoV) == 0 || hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::Grass) == 0 || hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::Crispin) == 0 || hand_count(Card::SecretBox) == 0 ||
        !ability_available_for_pokemon(Card::ForretressEx)) {
      return false;
    }

    int held_payloads = 0;
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      held_payloads += hand_count(card);
    }
    if (held_payloads < 1) return false;

    const auto available = [this](const Card card) {
      return hand_count(card) > 0 || deck_count_after_search_started(card) > 0;
    };
    return available(Card::ForestSealStone) && available(Card::Dawn) &&
        available(Card::Pineco) && available(Card::ForretressEx) &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 3 &&
        available(Card::Fire);
  }

  bool play_issue_2301_quick_ball_timer_route() {
    const auto cost = issue_2301_quick_ball_timer_cost();
    if (!cost) return false;

    // Quick Ball's printed discard precedes its Basic search. The Dragon spent on
    // T3 is deliberately not the strict-JIT payload; it buys the missing Basic and
    // evolution timer. The real search then establishes K1 before the future route
    // is banked, so exact Prize/deck availability is never read at K0.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item-cost and hidden-deck search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K0/K1 and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (!remove_one(state_.hand, Card::QuickBall)) {
      throw std::logic_error("issue-2301 Quick Ball disappeared");
    }
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(*cost, "Quick Ball issue-2301 timer cost",
                           "R-QB-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("issue-2301 Dragon cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-2301 timer route");
    const bool found = move_deck_to_hand(Card::RegidragoV);
    shuffle(state_.deck);
    issue_2301_banked_t4_route_ = found && issue_2301_k1_t4_route_proven();
    trace("QUICK BALL", "R-QB-01; P-DCI-01; P-KNOWLEDGE-01",
          found ? (issue_2301_banked_t4_route_
                       ? "Discarded one of two distinct Dragons, searched Regidrago V, and proved the draw-independent T4 Vessel/Secret Box route at K1."
                       : "Discarded one of two distinct Dragons and searched Regidrago V; exact K1 targets did not prove the banked T4 route.")
                : "Discarded one of two distinct Dragons; Regidrago V was unavailable after the legal K0 search.");
    return true;
  }

  bool finish_issue_2301_t3_bank() {
    if (!issue_2301_banked_t4_route_ || state_.turn != 3) return false;
    // Bench the searched Basic and spend only the already-held Grass attachment.
    // Crispin, Earthen Vessel, Secret Box, Regidrago VSTAR, and the second Dragon
    // stay untouched for the deterministic T4 continuation.
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Core Bench and manual Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (!bench_from_hand(Card::RegidragoV, false)) {
      issue_2301_banked_t4_route_ = false;
      return false;
    }
    auto regi = std::find_if(state_.bench.begin(), state_.bench.end(),
                             [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV && pokemon.entered_turn == state_.turn;
    });
    if (regi == state_.bench.end() || !remove_one(state_.hand, Card::Grass)) {
      issue_2301_banked_t4_route_ = false;
      return false;
    }
    ++regi->grass;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; P-AXIS-01",
          "Grass Energy manually to the issue-2301 Regidrago V; preserved the T4 Vessel, Secret Box, Crispin, VSTAR, and second Dragon.");
    trace("BANK ROUTE", "P-KNOWLEDGE-01; P-DCI-01; P-JIT-01",
          "Banked the exact K1-proven T4 Vessel/Secret Box/Forest Seal Stone/Dawn/Forretress continuation without using a future draw identity.");
    return true;
  }

  bool complete_issue_2301_banked_t4_route() {
    if (!issue_2301_banked_t4_route_ || state_.turn != 4 || item_locked() ||
        !supporter_allowed() || state_.supporter_used || state_.manual_energy_used ||
        state_.retreat_used || state_.vstar_power_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX ||
        state_.stadium != Stadium::ForestOfVitality ||
        !ability_available_for_pokemon(Card::ForretressEx) ||
        hand_count(Card::EarthenVessel) == 0 || hand_count(Card::SecretBox) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 || hand_count(Card::Crispin) == 0) {
      return false;
    }
    auto find_regi = [this]() {
      return std::find_if(state_.bench.begin(), state_.bench.end(),
                          [this](const Pokemon& pokemon) {
        return pokemon.card == Card::RegidragoV &&
               pokemon.entered_turn < state_.turn;
      });
    };
    auto regi = find_regi();
    if (regi == state_.bench.end() || regi->grass != 1 || regi->fire != 0) return false;

    auto payload = std::find_if(state_.hand.begin(), state_.hand.end(), is_payload);
    if (payload == state_.hand.end()) return false;
    const Card payload_cost = *payload;

    // Earthen Vessel creates the real T4 strict-JIT payload. It searches only the
    // Energy copies still needed by the route. If the mandatory draw moved Grass
    // or Fire into hand, that copy is used in the same deterministic route and the
    // deck search simply takes fewer cards, as Vessel permits up to two different
    // Basic Energy types.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, and Energy procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(payload_cost, "Earthen Vessel issue-2301 T4 payload cost",
                           "R-EV-01; P-JIT-01; P-DCI-01")) {
      return false;
    }
    record_deck_search_knowledge("Earthen Vessel issue-2301 T4 route");
    if (hand_count(Card::Grass) == 0 && !move_deck_to_hand(Card::Grass)) return false;
    if (hand_count(Card::Fire) == 0 && !move_deck_to_hand(Card::Fire)) return false;
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-EV-01; R-GAME-ITEM; P-JIT-01",
          "Earthen Vessel discarded the reserved Dragon on T4 and made Grass plus Fire available for the proven finish.");

    // Secret Box pays only route-replaced cards: the original held VSTAR will be
    // replaced by Star Alchemy, Crispin is replaced by Dawn/Forretress plus manual
    // Fire, and Grass is surplus because Exploding Energy supplies the board Grass.
    // The mandatory T4 draw is never needed as one of the three costs.
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Dynamic DCI / resource replacement: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    remove_one(state_.hand, Card::SecretBox);
    state_.discard.push_back(Card::SecretBox);
    if (!discard_from_hand(Card::RegidragoVstar, "Secret Box issue-2301 replaced VSTAR cost",
                           "R-SECRET-BOX-01; P-DCI-01") ||
        !discard_from_hand(Card::Crispin, "Secret Box issue-2301 replaced Crispin cost",
                           "R-SECRET-BOX-01; P-DCI-01") ||
        !discard_from_hand(Card::Grass, "Secret Box issue-2301 surplus Grass cost",
                           "R-SECRET-BOX-01; P-DCI-01")) {
      return false;
    }
    record_deck_search_knowledge("Secret Box issue-2301 T4 route");
    if (hand_count(Card::ForestSealStone) == 0 &&
        !move_deck_to_hand(Card::ForestSealStone)) return false;
    if (hand_count(Card::Dawn) == 0 && !move_deck_to_hand(Card::Dawn)) return false;
    shuffle(state_.deck);
    outcome_.used_secret_box = true;
    ++outcome_.secret_box_combo_attempted;
    trace("PLAY ITEM", "R-SECRET-BOX-01; R-GAME-ITEM; P-DCI-01",
          "Secret Box spent the replaced VSTAR, Crispin, and Grass; searched Forest Seal Stone and Dawn without consuming the mandatory T4 draw as a cost.");

    regi = find_regi();
    if (regi == state_.bench.end() || regi->tool != Tool::None ||
        !remove_one(state_.hand, Card::ForestSealStone)) return false;
    regi->tool = Tool::ForestSealStone;
    trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL",
          "Attached Forest Seal Stone to the prior-turn Regidrago V for Star Alchemy.");

    // Star Alchemy searches a replacement VSTAR. If the mandatory draw moved that
    // known K1 target into hand, the search may legally find no copy; the same
    // replacement card is already present and the route remains draw-identity safe.
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Pokémon V includes Regidrago V: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
    // Hidden-deck search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    record_deck_search_knowledge("Star Alchemy issue-2301 replacement VSTAR");
    move_deck_to_hand(Card::RegidragoVstar);
    shuffle(state_.deck);
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    if (hand_count(Card::RegidragoVstar) == 0) return false;
    trace("USE VSTAR POWER", "R-FSS-01; P-KNOWLEDGE-01",
          "Star Alchemy made a replacement Regidrago VSTAR available after Secret Box spent the original held copy.");

    // Dawn supplies the Grass evolution line. Its hidden searches may use copies
    // already moved into hand by the mandatory draw, so success never depends on
    // which identity was drawn.
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Official Supporter and hidden-deck search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    remove_one(state_.hand, Card::Dawn);
    state_.discard.push_back(Card::Dawn);
    state_.supporter_used = true;
    record_deck_search_knowledge("Dawn issue-2301 Pineco line");
    if (hand_count(Card::Pineco) == 0 && !move_deck_to_hand(Card::Pineco)) return false;
    if (hand_count(Card::ForretressEx) == 0 && !move_deck_to_hand(Card::ForretressEx)) return false;
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-DAWN-01; R-GAME-SUPPORTER",
          "Dawn made Pineco and Forretress ex available for the Forest of Vitality line.");

    if (!bench_from_hand(Card::Pineco, false)) return false;
    auto pineco = std::find_if(state_.bench.begin(), state_.bench.end(),
                               [this](const Pokemon& pokemon) {
      return pokemon.card == Card::Pineco && pokemon.entered_turn == state_.turn;
    });
    if (pineco == state_.bench.end() || !remove_one(state_.hand, Card::ForretressEx)) return false;
    pineco->card = Card::ForretressEx;
    trace("EVOLVE", "R-FOREST-VITALITY-01; R-GAME-EVOLVE",
          "Forest of Vitality evolved the newly Benched Grass Pineco into Grass Forretress ex on T4.");

    // Exploding Energy attaches exactly the two Grass needed by this board, then
    // Knocks Out Forretress ex: one Grass completes Regidrago's second Grass, and
    // one Grass pays Tapu Lele-GX's printed one-Energy Retreat Cost.
    // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Tapu Lele-GX Retreat Cost 1: https://api.pokemontcg.io/v2/cards/sm2-60
    // Official Ability, Energy, Knock Out, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    regi = find_regi();
    pineco = std::find_if(state_.bench.begin(), state_.bench.end(),
                          [](const Pokemon& pokemon) {
      return pokemon.card == Card::ForretressEx;
    });
    if (regi == state_.bench.end() || pineco == state_.bench.end() ||
        !state_.active || state_.active->card != Card::TapuLeleGX ||
        std::count(state_.deck.begin(), state_.deck.end(), Card::Grass) < 2) {
      return false;
    }
    remove_one(state_.deck, Card::Grass);
    remove_one(state_.deck, Card::Grass);
    ++regi->grass;
    ++state_.active->grass;
    shuffle(state_.deck);
    outcome_.used_exploding_energy = true;
    trace("USE ABILITY", "R-FORRETRESS-01; R-GAME-ABILITY",
          "Exploding Energy attached one Grass to Regidrago V and one Grass to Active Tapu Lele-GX, then Knocked Out Forretress ex.");
    state_.discard.push_back(Card::ForretressEx);
    state_.bench.erase(pineco);

    regi = find_regi();
    if (regi == state_.bench.end() || !remove_one(state_.hand, Card::Fire)) return false;
    ++regi->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          "Fire Energy manually to the issue-2301 Regidrago V, completing GGF.");

    if (!remove_one(state_.hand, Card::RegidragoVstar)) return false;
    regi->card = Card::RegidragoVstar;
    trace("EVOLVE", "R-GAME-EVOLVE; R-RVS-01",
          "Evolved the prior-turn Regidrago V into Regidrago VSTAR after GGF was complete.");

    if (!state_.active || state_.active->card != Card::TapuLeleGX ||
        state_.active->grass < 1) return false;
    auto ready = std::find_if(state_.bench.begin(), state_.bench.end(),
                              [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoVstar &&
             pays_apex_energy_cost(pokemon);
    }); // DDE-aware: https://github.com/FlareZ123/pokemon-sims/issues/2368
    if (ready == state_.bench.end()) return false;
    --state_.active->grass;
    state_.discard.push_back(Card::Grass);
    std::swap(*state_.active, *ready);
    state_.retreat_used = true;
    issue_2301_banked_t4_route_ = false;
    trace("RETREAT", "R-GAME-RETREAT; R-TAPU-01",
          "Discarded Tapu Lele-GX's attached Grass and promoted the GGF Regidrago VSTAR.");
    trace("COMPLETE ROUTE", "P-JIT-01; P-AXIS-01",
          "Completed the deterministic issue-2301 T4 route with the Vessel-cost Dragon discarded this turn.");
    return active_is_vstar() && !need_energy() && !need_payload();
  }

  bool play_quick_ball(const bool permit_payload) {
    if (play_issue_2301_quick_ball_timer_route()) return true;
    return play_quick_ball_issue2301_original(permit_payload);
  }
