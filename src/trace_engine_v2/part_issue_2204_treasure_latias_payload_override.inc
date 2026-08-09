  std::optional<Card> issue_2204_treasure_latias_payload_cost() const {
    const bool ready_benched_vstar =
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
                      return pokemon.card == Card::RegidragoVstar &&
                             pays_apex_energy_cost(pokemon);
                    });
    const bool route_available =
        strict_payload_timing() && prizes_known() && !item_locked() &&
        need_active_vstar() && need_payload() && state_.active &&
        is_basic(state_.active->card) && !state_.retreat_used &&
        ready_benched_vstar && bench_space() > 0 &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        hand_count(Card::LatiasEx) == 0 && !in_play(Card::LatiasEx) &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0;
    if (!route_available) return std::nullopt;

    // Mysterious Treasure's mandatory discard is the current-turn Apex Dragon
    // payload, while the same search finds Latias ex. Skyliner removes the Basic
    // Active's Retreat Cost so the already powered Benched Regidrago VSTAR can be
    // promoted immediately.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Official Item, discard, search, Bench, Ability, Retreat, and promotion procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, current-turn JIT, Active-position, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed stale-claim bug: https://github.com/FlareZ123/pokemon-sims/issues/2204
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_1876_treasure_payload_cost() const {
    // Preserve every pre-existing selector priority, then insert #2204 at the
    // exact point used by the reviewed stale-claim patch, immediately after the
    // #2232 route and before the generic GF/Crispin fallback.
    // Existing selector contract: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1876_treasure_payload_cost_override.inc
    // Confirmed stale-claim bug: https://github.com/FlareZ123/pokemon-sims/issues/2204
    if (const auto cost = issue_2203_treasure_vstar_payload_cost()) return cost;
    if (const auto cost = issue_2226_treasure_gf_oricorio_payload_cost()) return cost;
    if (const auto cost = issue_2227_treasure_gg_tapu_crispin_payload_cost()) return cost;
    if (const auto cost = issue_2232_treasure_tapu_crispin_two_energy_cost()) return cost;
    if (const auto cost = issue_2204_treasure_latias_payload_cost()) return cost;
    return issue_1876_treasure_payload_cost_issue2204_original();
  }
