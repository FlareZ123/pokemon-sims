  std::optional<std::array<Card, 2>> issue_1879_treasure_quick_ball_plan() const {
    // The route relies only on public K1 composition. Full deck inspection and
    // full Prize inspection are equivalent knowledge states for this decision.
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1879
    if (!strict_payload_timing() || state_.turn > scenario_.max_turn ||
        item_locked() || !supporter_allowed() || !prizes_known() ||
        !active_is_vstar() || need_regi() || need_vstar() ||
        need_active_vstar() || !need_energy() || !need_payload() ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::TapuLeleGX) > 0 ||
        hand_count(Card::Crispin) > 0 || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0) {
      return std::nullopt;
    }

    std::optional<Card> payload;
    // Search choice follows a fixed public priority and never hidden deck order.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Future-card oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    for (const Card candidate : {Card::Appletun, Card::MegaDragonite,
                                 Card::Dragapult, Card::GoodraVstar,
                                 Card::DialgaGX}) {
      if (deck_count_after_search_started(candidate) > 0) {
        payload = candidate;
        break;
      }
    }
    if (!payload) return std::nullopt;

    // Reserve Quick Ball while evaluating Treasure's independent printed cost.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dynamic DCI and connector contention: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1879
    Engine projected = *this;
    projected.trace_ = nullptr;
    if (!remove_one(projected.state_.hand, Card::QuickBall)) {
      return std::nullopt;
    }
    auto treasure_cost = projected.choose_discard(
        false, true, true, Card::MysteriousTreasure);
    if (!treasure_cost && hand_count(Card::EarthenVessel) > 0) {
      // Crispin plus the unused manual attachment fully replaces Vessel's Energy
      // channel on this proven route, making Vessel a route-specific DCI-1 cost.
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      treasure_cost = Card::EarthenVessel;
    }
    if (!treasure_cost) return std::nullopt;

    const bool grass_in_deck =
        deck_count_after_search_started(Card::Grass) > 0;
    const bool fire_in_deck =
        deck_count_after_search_started(Card::Fire) > 0;
    int remaining_grass = grass_needed();
    int remaining_fire = fire_needed();
    bool grass_for_manual = hand_count(Card::Grass) > 0;
    bool fire_for_manual = hand_count(Card::Fire) > 0;

    // Crispin attaches one Energy only when two different Basic Energy types are
    // found. With one searchable type, it puts that card into hand and the route
    // still needs the unused manual attachment.
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Official attachment and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    if (grass_in_deck && fire_in_deck) {
      if (remaining_grass > 0) {
        --remaining_grass;
        fire_for_manual = true;
      } else if (remaining_fire > 0) {
        --remaining_fire;
        grass_for_manual = true;
      }
    } else if (grass_in_deck) {
      // An unneeded one-type search cannot justify consuming the Supporter.
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Trainer no-effect ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
      if (remaining_grass == 0) return std::nullopt;
      grass_for_manual = true;
    } else if (fire_in_deck) {
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Trainer no-effect ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
      if (remaining_fire == 0) return std::nullopt;
      fire_for_manual = true;
    } else {
      return std::nullopt;
    }

    if (remaining_grass == 0 && remaining_fire == 0) {
      return std::array<Card, 2>{*treasure_cost, *payload};
    }
    if (state_.manual_energy_used ||
        remaining_grass + remaining_fire != 1) {
      return std::nullopt;
    }
    if ((remaining_grass == 1 && !grass_for_manual) ||
        (remaining_fire == 1 && !fire_for_manual)) {
      return std::nullopt;
    }
    return std::array<Card, 2>{*treasure_cost, *payload};
  }

  std::optional<Card> issue_2201_tapu_crispin_payload_cost() const {
    const bool route_available =
        strict_payload_timing() && prizes_known() && !item_locked() &&
        supporter_allowed() && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        (state_.manual_energy_used || hand_count(Card::Grass) == 0) &&
        need_payload() && hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::Crispin) == 0 && bench_space() > 0 &&
        hand_count(Card::TapuLeleGX) == 0 &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
    if (!route_available) return std::nullopt;

    // Quick Ball's mandatory discard is the current-turn JIT outlet. This route
    // is live only when a held manual Grass attachment cannot already complete GGF.
    // Tapu Lele-GX then Wonder Tags Crispin, which attaches the missing Grass.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Bench, Ability, Supporter, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, current-turn JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2201
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool play_issue_2201_quick_ball_tapu_crispin_route() {
    const auto cost = issue_2201_tapu_crispin_payload_cost();
    if (!cost) return false;

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(*cost,
                           "Quick Ball issue-2201 Tapu-Crispin payload cost",
                           "R-QB-01; P-DCI-01; P-JIT-01; P-CONNECTOR-01")) {
      throw std::logic_error("Issue-2201 Quick Ball payload cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-2201 Tapu-Crispin route");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error("Issue-2201 Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-TAPU-01; R-CRISPIN-01; P-JIT-01",
          "Discarded the current-turn Dragon payload and searched Tapu Lele-GX for Crispin.");
    return true;
  }

  bool complete_issue_1879_treasure_quick_ball_tapu_crispin_route() {
    // The active Quick Ball wrapper calls this composed-route hook before generic
    // Quick Ball handling. Admit #2201 first because it needs only Quick Ball,
    // whereas #1879 additionally requires Mysterious Treasure.
    // Route ordering policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2201
    if (play_issue_2201_quick_ball_tapu_crispin_route()) return true;

    const auto plan = issue_1879_treasure_quick_ball_plan();
    if (!plan) return false;
    const Card treasure_cost = (*plan)[0];
    const Card searched_payload = (*plan)[1];

    // Treasure converts a route-replaced card into a Dragon payload. Quick Ball
    // pays its separate cost with that Dragon and searches Tapu Lele-GX. Wonder
    // Tag obtains Crispin, then Crispin plus the unused manual attachment completes
    // GGF during the same turn.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Bench, Ability, Supporter, attachment, and shuffle procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, supporter contention, dynamic DCI, and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1879
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(treasure_cost,
                           "Mysterious Treasure issue-1879 first cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("Issue-1879 Treasure cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1879 route");
    if (!move_deck_to_hand(searched_payload)) {
      throw std::logic_error("Issue-1879 Dragon payload target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          "Spent " + std::string(name(treasure_cost)) +
              " and searched " + std::string(name(searched_payload)) +
              " for Quick Ball's current-turn payload cost.");

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(searched_payload,
                           "Quick Ball issue-1879 payload cost",
                           "R-QB-01; P-JIT-01; P-COMPRESS-01")) {
      throw std::logic_error("Issue-1879 Quick Ball payload disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-1879 Tapu route");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error("Issue-1879 Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-JIT-01; P-COMPRESS-01",
          "Discarded the searched Dragon payload and searched Tapu Lele-GX for Wonder Tag.");

    if (!bench_from_hand(Card::TapuLeleGX, false)) {
      throw std::logic_error("Issue-1879 Tapu Lele-GX Bench action failed");
    }
    record_deck_search_knowledge("Wonder Tag issue-1879 Crispin route");
    if (!move_deck_to_hand(Card::Crispin)) {
      throw std::logic_error("Issue-1879 Crispin target disappeared");
    }
    shuffle(state_.deck);
    trace("WONDER TAG", "R-TAPU-01; R-CRISPIN-01; P-COMPRESS-01",
          "Searched and revealed Crispin for the same-turn GGF completion.");
    if (!play_crispin()) {
      throw std::logic_error("Issue-1879 Crispin completion disappeared");
    }
    if (need_energy() && !attach_manual()) {
      throw std::logic_error("Issue-1879 manual Energy completion disappeared");
    }
    if (!active_is_vstar() || need_energy() || !payload_ready()) {
      throw std::logic_error("Issue-1879 route failed its proven ready state");
    }
    return true;
  }
