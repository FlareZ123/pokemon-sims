#undef play_mysterious_treasure
#define play_mysterious_treasure play_mysterious_treasure_issue2200_original
#include "part_issue_1356_fss_treasure_energy_override_base.inc"
#undef play_mysterious_treasure

  std::optional<Card> issue_2200_oricorio_payload_cost() const {
    // A held Dragon becomes DCI 1 when the same Mysterious Treasure action can
    // search Oricorio, whose Vital Dance exposes the missing Fire Energy for the
    // unused manual attachment on the strict-JIT ready turn:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Dynamic DCI and current-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2200
    for (const Card candidate : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::DialgaGX,
                                 Card::Appletun}) {
      if (hand_count(candidate) > 0) return candidate;
    }
    return std::nullopt;
  }

  bool issue_2200_treasure_oricorio_payload_route_available() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        state_.turn > scenario_.max_turn || !prizes_known() || item_locked() ||
        !active_is_vstar() || state_.manual_energy_used || !need_energy() ||
        grass_needed() != 0 || fire_needed() != 1 || !need_payload() ||
        bench_space() == 0 || hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::Oricorio) > 0 || in_play(Card::Oricorio) ||
        !ability_available_for_pokemon(Card::Oricorio) ||
        deck_count_after_search_started(Card::Oricorio) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0 ||
        !issue_2200_oricorio_payload_cost().has_value()) {
      return false;
    }

    // Exact K1 establishes both remaining deck targets. Treasure's mandatory
    // discard supplies the payload, Oricorio enters the Bench from that search,
    // Vital Dance searches Fire, and the unused manual attachment completes GGF:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Official Item, discard, search, Bench, Ability, shuffle, and Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2200
    return true;
  }

  bool play_issue_2200_treasure_oricorio_payload_route() {
    if (!issue_2200_treasure_oricorio_payload_route_available()) return false;
    const Card payload = *issue_2200_oricorio_payload_cost();

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(payload,
                           "Mysterious Treasure issue-2200 current-turn payload cost",
                           "R-MT-01; P-DCI-01; P-JIT-01; P-CONNECTOR-01")) {
      throw std::logic_error("Issue-2200 Treasure payload cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-2200 Oricorio route");
    if (!move_deck_to_hand(Card::Oricorio)) {
      throw std::logic_error("Issue-2200 Oricorio target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-ORICORIO-01; P-JIT-01; P-CONNECTOR-01",
          "Discarded " + std::string(name(payload)) +
              " as the current-turn payload and searched Oricorio for Vital Dance.");
    return true;
  }

  bool play_mysterious_treasure_issue1476_original(
      const bool permit_payload) {
    if (play_issue_2200_treasure_oricorio_payload_route()) return true;
    return play_mysterious_treasure_issue2200_original(permit_payload);
  }
