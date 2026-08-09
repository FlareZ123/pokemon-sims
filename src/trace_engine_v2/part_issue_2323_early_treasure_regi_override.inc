#define play_mysterious_treasure play_mysterious_treasure_issue2271_original
#include "part_issue_2202_treasure_tapu_crispin_override.inc"

  bool pre_steven_treasure_has_public_payload() const {
    for (const Card card : {Card::MegaDragonite, Card::Dragapult,
                            Card::GoodraVstar, Card::DialgaGX,
                            Card::Appletun}) {
      if (issue_1419_publicly_possible_count(card) > 0) return true;
    }
    return false;
  }

  std::optional<Card> pre_steven_treasure_regidrago_cost() const {
    if (!first_turn_going_first() || supporter_allowed() ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        prizes_known() || item_locked() || state_.supporter_used ||
        state_.manual_energy_used || bench_space() <= 0 || !state_.active ||
        state_.active->card != Card::LatiasEx || !need_regi() ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoV) > 0 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::ErikasInvitation) == 0 ||
        hand_count(Card::Grass) == 0 || hand_count(Card::Fire) == 0 ||
        issue_1419_publicly_possible_count(Card::RegidragoV) == 0) {
      return std::nullopt;
    }

    const bool needs_vstar = hand_count(Card::RegidragoVstar) == 0;
    const bool needs_final_grass = hand_count(Card::Grass) < 2;
    const bool needs_blender = hand_count(Card::BrilliantBlender) == 0;
    const int steven_targets = static_cast<int>(needs_vstar) +
                               static_cast<int>(needs_final_grass) +
                               static_cast<int>(needs_blender);
    if (steven_targets > 3 ||
        (needs_vstar &&
         issue_1419_publicly_possible_count(Card::RegidragoVstar) == 0) ||
        (needs_final_grass &&
         issue_1419_publicly_possible_count(Card::Grass) == 0) ||
        (needs_blender &&
         issue_1419_publicly_possible_count(Card::BrilliantBlender) == 0) ||
        !pre_steven_treasure_has_public_payload()) {
      return std::nullopt;
    }

    // On the first player's opening turn, Supporters cannot be played, while Items,
    // Benching a Basic, and the once-per-turn manual Energy attachment remain legal.
    // Spending route-replaced Erika's Invitation on Mysterious Treasure therefore
    // establishes Regidrago V one turn earlier and uses an otherwise-lost attachment.
    // Steven's Resolve can then bank the at-most-three missing T3 axes before ending
    // T2: Regidrago VSTAR, the final Grass Energy, and Brilliant Blender. Latias ex
    // guarantees the Basic Active's zero-Retreat promotion route, while Blender
    // supplies a same-turn Dragon payload for strict/matchup-flex JIT.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V / VSTAR and Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Erika's Invitation: https://api.pokemontcg.io/v2/cards/sv3pt5-160
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Official first-turn, Item, Bench, attachment, evolution, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K0/K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed systemic route bug: https://github.com/FlareZ123/pokemon-sims/issues/2658
    return Card::ErikasInvitation;
  }

  bool play_pre_steven_treasure_regidrago() {
    const std::optional<Card> cost = pre_steven_treasure_regidrago_cost();
    if (!cost) return false;

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(*cost, "Mysterious Treasure pre-Steven route cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected pre-Steven Treasure cost disappeared");
    }

    // The K0 decision above only uses publicly possible inventory. Treasure's first
    // legal deck inspection establishes K1 after its printed discard cost is paid.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2658
    record_deck_search_knowledge("Mysterious Treasure pre-Steven route");
    if (move_deck_to_hand(Card::RegidragoV)) {
      shuffle(state_.deck);
      trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
            "Spent route-replaced Erika's Invitation and searched Regidrago V before the first legal Steven turn.");
      if (!bench_from_hand(Card::RegidragoV, false)) {
        throw std::logic_error("projected pre-Steven Regidrago V Bench action disappeared");
      }
      return true;
    }

    const Card fallback = fallback_mysterious_target_after_search_started();
    const bool found_fallback = move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
          found_fallback
              ? "K1 proved Regidrago V unavailable; Mysterious Treasure searched " +
                    std::string(name(fallback)) + "."
              : "K1 proved no Psychic or Dragon Pokemon remained in deck.");
    return true;
  }

  std::optional<Card> issue_2323_redundant_payload_cost() const {
    if (scenario_.dci != DciProfile::StrictJit || scenario_.going_first ||
        scenario_.locks != LockMode::None || state_.turn != 1 ||
        scenario_.max_turn < 3 || prizes_known() || item_locked() ||
        !supporter_allowed() || state_.supporter_used || state_.manual_energy_used ||
        bench_space() <= 0 || !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV ||
        state_.active->card == Card::RegidragoVstar || !need_regi() ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoV) > 0 || hand_count(Card::Crispin) == 0 ||
        hand_count(Card::Grass) == 0 ||
        issue_1419_publicly_possible_count(Card::RegidragoV) == 0 ||
        issue_1419_publicly_possible_count(Card::Grass) == 0 ||
        issue_1419_publicly_possible_count(Card::Fire) == 0) {
      return std::nullopt;
    }

    std::array<Card, 5> held_payloads{Card::MegaDragonite, Card::Dragapult,
                                      Card::GoodraVstar, Card::DialgaGX,
                                      Card::Appletun};
    int distinct_payloads = 0;
    for (const Card card : held_payloads) {
      if (hand_count(card) > 0) ++distinct_payloads;
    }
    if (distinct_payloads < 2) return std::nullopt;

    // Mysterious Treasure pays one other card before searching a Psychic or Dragon
    // Pokemon. With two distinct held A/S Dragons, one Dragon is route-redundant:
    // spending it starts the Regidrago V evolution timer and legally establishes K1
    // while a second held Dragon remains protected for the later strict-JIT ready
    // turn. Held Grass plus Crispin and publicly possible Grass/Fire copies give the
    // searched Basic an observable same-turn Energy progression without consulting
    // Prize identities or future draws. The ordinary Treasure policy has already
    // declined before this fallback runs, so lower-DCI costs retain precedence.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mega Dragonite ex / Dragapult ex: https://api.pokemontcg.io/v2/cards/me2pt5-152 https://api.pokemontcg.io/v2/cards/sv6-130
    // Official Item, discard, search, Bench, Supporter, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K0/K1, strict-JIT, dynamic DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug and source-bound seed: https://github.com/FlareZ123/pokemon-sims/issues/2323 https://github.com/FlareZ123/pokemon-sims/pull/2321
    for (const Card card : held_payloads) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (play_issue_2202_treasure_tapu_crispin()) return true;
    if (play_mysterious_treasure_issue2323_original(permit_payload)) return true;
    if (play_pre_steven_treasure_regidrago()) return true;

    const std::optional<Card> cost = issue_2323_redundant_payload_cost();
    if (!cost) return false;

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(*cost,
                           "Mysterious Treasure issue-2323 redundant-payload cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-2323 redundant Dragon cost disappeared");
    }

    // The first legal deck inspection establishes K1 only after Treasure has paid
    // its printed cost. Exact deck and Prize information is therefore unavailable
    // to issue_2323_redundant_payload_cost() and becomes usable only from here.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // K0/K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Hidden-information contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2323
    record_deck_search_knowledge("Mysterious Treasure");
    if (move_deck_to_hand(Card::RegidragoV)) {
      shuffle(state_.deck);
      trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
            "Spent one of two distinct held Dragon payloads and searched Regidrago V to start the earliest evolution timer while preserving another Dragon for strict-JIT.");
      if (!bench_from_hand(Card::RegidragoV, false)) {
        throw std::logic_error("projected issue-2323 Regidrago V Bench action disappeared");
      }
      return true;
    }

    const Card fallback = fallback_mysterious_target_after_search_started();
    const bool found_fallback = move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
          found_fallback
              ? "Spent one redundant held Dragon; K1 proved Regidrago V unavailable, so Mysterious Treasure searched " + std::string(name(fallback)) + "."
              : "Spent one redundant held Dragon; K1 proved no Psychic or Dragon Pokemon remained in deck.");
    return true;
  }
#undef play_mysterious_treasure
#define play_mysterious_treasure play_mysterious_treasure_issue2269_original
#include "part_issue_2271_surplus_regidrago_v_route_override.inc"
#undef play_mysterious_treasure
#include "part_issue_2269_oricorio_latias_blender_override.inc"

  const Pokemon* issue_2658_banked_regidrago() const {
    const auto found = std::find_if(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn;
        });
    return found == state_.bench.end() ? nullptr : &*found;
  }

  bool issue_2658_banked_steven_t3_route_available() const {
    const Pokemon* regi = issue_2658_banked_regidrago();
    if (state_.turn != 2 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !supporter_allowed() || !prizes_known() || state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::LatiasEx ||
        !ability_available_for_pokemon(Card::LatiasEx) || state_.retreat_used ||
        !regi || regi->grass != 1 || regi->fire != 0 ||
        hand_count(Card::Fire) == 0 || hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::BrilliantBlender) > 0 ||
        !need_vstar() || !need_energy() || !need_payload() ||
        !payload_might_be_in_deck() ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::BrilliantBlender) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0) {
      return false;
    }

    // T1 Mysterious Treasure has already established K1 and the evolution timer.
    // The held Fire plus the T2 manual attachment guarantees that one more T3
    // attachment can complete GGF regardless of whether T2's incidental draw is
    // Grass. Steven therefore reserves the still-missing VSTAR and Blender, plus a
    // Grass only when the actual T2 attachment leaves the Benched Regidrago at GF.
    // This preserves the deterministic T3 route instead of consuming the T2
    // Supporter slot on a newly drawn Crispin and depending on a later Arven draw.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V / VSTAR and Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Official attachment, Supporter, evolution, Item, Retreat, and first-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, and earliest deterministic route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2658
    return true;
  }

  bool banked_steven_has_known_t3_blender_route_issue2658() const {
    return banked_steven_has_known_t3_blender_route() ||
           issue_2658_banked_steven_t3_route_available();
  }

  bool play_issue_2658_banked_steven_t3_route() {
    if (!issue_2658_banked_steven_t3_route_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    const Pokemon* regi = issue_2658_banked_regidrago();
    if (!regi) {
      throw std::logic_error("projected issue-2658 Regidrago V disappeared");
    }

    std::vector<Card> targets{Card::RegidragoVstar, Card::BrilliantBlender};
    if (regi->grass == 1 && regi->fire >= 1) targets.push_back(Card::Grass);

    std::vector<Card> found;
    for (const Card target : targets) {
      if (!move_deck_to_hand(target)) {
        throw std::logic_error("Known issue-2658 Steven target disappeared");
      }
      found.push_back(target);
    }
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Steven searches up to three cards and ends the turn. VSTAR and Blender are
    // mandatory for the deterministic T3 continuation. A searched Grass is required
    // only after the actual T2 attachment produced GF; if the Regidrago is already
    // at GG, the held Fire is the deterministic T3 attachment instead.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Official search, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest deterministic route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2658
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01",
          "Searched the deterministic T3 package: " + join_cards(found) +
              ". Turn ended.");
    return true;
  }

  bool play_banked_steven_for_known_t3_blender_route_issue2658() {
    if (banked_steven_has_known_t3_blender_route()) {
      return play_banked_steven_for_known_t3_blender_route();
    }
    return play_issue_2658_banked_steven_t3_route();
  }

// The final run-turn selector is defined after this search-policy include. Route its
// existing early banked-Steven hook through the state-driven #2658 extension while
// preserving the completed #946/#1351 implementation for every prior state.
// Existing banked-Steven route: https://github.com/FlareZ123/pokemon-sims/issues/946
// No-discard extension: https://github.com/FlareZ123/pokemon-sims/issues/1351
// Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2658
#define banked_steven_has_known_t3_blender_route \
  banked_steven_has_known_t3_blender_route_issue2658
#define play_banked_steven_for_known_t3_blender_route \
  play_banked_steven_for_known_t3_blender_route_issue2658
