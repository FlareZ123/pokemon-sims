#define play_mysterious_treasure play_mysterious_treasure_issue2271_original
#include "part_issue_2202_treasure_tapu_crispin_override.inc"

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
    if (play_issue_2202_treasure_tapu_crispin(permit_payload)) return true;
    if (play_mysterious_treasure_issue2323_original(permit_payload)) return true;

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
#include "part_issue_2271_surplus_regidrago_v_route_override.inc"
