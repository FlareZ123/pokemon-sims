#include "part_issue_1199_steven_package_override.inc"
#include "part_issue_1478_arven_vessel_completion.inc"

  bool issue_1481_hold_serena_for_free_latias_completion() const {
    // Once Brilliant Blender has established the strict-JIT payload and a Benched
    // Regidrago VSTAR already pays GGF, Skyliner resolves the sole remaining Active
    // axis for zero. Preserve Serena, its mandatory discard, the Supporter action,
    // and the random draw state:
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Latias ex / Skyliner applies to every Basic Active: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest resource-preserving route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1481
    return supporter_allowed() && hand_count(Card::Serena) > 0 &&
        need_active_vstar() && !need_regi() && !need_vstar() &&
        !need_energy() && !need_payload() && !state_.retreat_used &&
        state_.active && is_basic(state_.active->card) &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        in_play(Card::LatiasEx) && benched_vstar_promotion_ready();
  }

  bool held_quick_ball_latias_completes_current_turn_without_arven() {
    if (!supporter_allowed() || hand_count(Card::Arven) == 0 || item_locked() ||
        !deck_seen_ || hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 || bench_space() == 0 ||
        !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        hand_count(Card::LatiasEx) > 0 || in_play(Card::LatiasEx) ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        need_energy() || !need_payload()) {
      return false;
    }

    const bool prepared_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.grass >= 2 &&
              pokemon.fire >= 1;
        });
    const auto quick_ball_cost =
        choose_discard(true, true, true, Card::QuickBall);
    if (!prepared_regidrago || !quick_ball_cost ||
        !is_payload(*quick_ball_cost)) {
      return false;
    }

    // The held Quick Ball and Dragon already satisfy the current-turn payload and
    // Latias ex promotion axes. Reserve Arven, project the remaining legal actions,
    // and hold the Supporter only when the direct route reaches the same ready state:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, Item, evolution, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest resource-preserving route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1208
    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    projected.state_.supporter_used = true;
    projected.run_turn();

    const bool arven_preserved =
        projected.hand_count(Card::Arven) == hand_count(Card::Arven);
    const bool quick_ball_resolved =
        count_of(projected.state_.discard, Card::QuickBall) >
        count_of(state_.discard, Card::QuickBall);
    const bool latias_benched = std::any_of(
        projected.state_.bench.begin(), projected.state_.bench.end(),
        [](const Pokemon& pokemon) { return pokemon.card == Card::LatiasEx; });
    const bool payload_discarded = std::any_of(
        projected.state_.discarded_this_turn.begin(),
        projected.state_.discarded_this_turn.end(), is_payload);
    return arven_preserved && quick_ball_resolved && latias_benched &&
        payload_discarded && projected.active_is_vstar() &&
        projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 && projected.payload_ready();
  }

  bool issue_1795_crispin_steven_vessel_route_available() const {
    const bool held_direct_vstar_connector =
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::EvolutionIncense) > 0 ||
        hand_count(Card::MysteriousTreasure) > 0 ||
        hand_count(Card::UltraBall) > 0 ||
        hand_count(Card::PokemonCommunication) > 0 ||
        hand_count(Card::ForestSealStone) > 0 ||
        hand_count(Card::Arven) > 0 || hand_count(Card::SecretBox) > 0;
    const bool held_vessel_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);

    if (scenario_.dci != DciProfile::StrictJit || !scenario_.going_first ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        state_.turn + 2 > scenario_.max_turn || !prizes_known() ||
        !supporter_allowed() || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 0 || state_.active->fire != 0 ||
        !need_energy() || !need_vstar() || !need_payload() ||
        hand_count(Card::Gladion) == 0 || hand_count(Card::Crispin) == 0 ||
        hand_count(Card::StevensResolve) == 0 || !held_vessel_payload ||
        held_direct_vstar_connector ||
        deck_count_after_search_started(Card::Grass) < 2 ||
        deck_count_after_search_started(Card::Fire) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::EarthenVessel) == 0) {
      return false;
    }

    // K1 exposes a complete Supporter schedule. Crispin searches different Basic
    // Energy types on T2, attaches one, and leaves the other for the still-unused
    // manual attachment. Steven's Resolve then banks Regidrago VSTAR and Earthen
    // Vessel on T3. On T4, Vessel discards the held Dragon payload, searches the
    // final Grass Energy, and the manual attachment completes GGF. This deterministic
    // route outranks Gladion's single-Prize exchange and a random Celestial Roar:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, search, attachment, evolution, Item, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1795
    return crispin_can_advance_energy_axis();
  }

  int issue_1795_steven_turn_ = 0;
  int issue_1795_vessel_turn_ = 0;

  bool issue_1795_banked_steven_route_available() const {
    const bool has_or_can_search_vstar =
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) > 0;
    const bool has_or_can_search_vessel =
        hand_count(Card::EarthenVessel) > 0 ||
        deck_count_after_search_started(Card::EarthenVessel) > 0;
    const bool still_needs_steven = hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::EarthenVessel) == 0;
    const bool has_final_grass = hand_count(Card::Grass) > 0 ||
        deck_count_after_search_started(Card::Grass) > 0;
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);

    return issue_1795_steven_turn_ == state_.turn &&
        scenario_.dci == DciProfile::StrictJit && scenario_.going_first &&
        scenario_.locks == LockMode::None && state_.turn + 1 <= scenario_.max_turn &&
        prizes_known() && supporter_allowed() && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        hand_count(Card::StevensResolve) > 0 && held_payload &&
        has_or_can_search_vstar && has_or_can_search_vessel &&
        still_needs_steven && has_final_grass;
  }

  bool play_issue_1795_banked_steven() {
    if (issue_1795_steven_turn_ != state_.turn) return false;
    if (!issue_1795_banked_steven_route_available()) {
      issue_1795_steven_turn_ = 0;
      return false;
    }

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-1795 schedule");

    std::vector<Card> found;
    if (hand_count(Card::RegidragoVstar) == 0) {
      if (!move_deck_to_hand(Card::RegidragoVstar)) {
        throw std::logic_error("Issue-1795 Regidrago VSTAR target disappeared");
      }
      found.push_back(Card::RegidragoVstar);
    }
    if (hand_count(Card::EarthenVessel) == 0) {
      if (!move_deck_to_hand(Card::EarthenVessel)) {
        throw std::logic_error("Issue-1795 Earthen Vessel target disappeared");
      }
      found.push_back(Card::EarthenVessel);
    }
    shuffle(state_.deck);
    state_.turn_ended = true;
    issue_1795_steven_turn_ = 0;
    issue_1795_vessel_turn_ = state_.turn + 1;

    // Steven may search up to three cards and ends the turn. The committed K1 route
    // takes only the missing Regidrago VSTAR and Earthen Vessel pieces, preserving
    // the held Dragon for Vessel's strict-JIT cost on the next turn:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter and search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1795
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01",
          "Continued the issue-1795 route and searched: " +
              join_cards(found) + ". Turn ended.");
    return true;
  }

  bool complete_issue_1795_vessel_finish() {
    if (issue_1795_vessel_turn_ != state_.turn) return false;

    std::optional<Card> payload;
    for (const Card card : state_.hand) {
      if (is_payload(card)) {
        payload = card;
        break;
      }
    }
    const bool has_final_grass = hand_count(Card::Grass) > 0 ||
        deck_count_after_search_started(Card::Grass) > 0;
    const bool route_available =
        scenario_.dci == DciProfile::StrictJit && scenario_.going_first &&
        scenario_.locks == LockMode::None && !state_.turn_ended &&
        prizes_known() && !state_.manual_energy_used && !item_locked() &&
        state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        payload.has_value() && has_final_grass;
    if (!route_available) {
      issue_1795_vessel_turn_ = 0;
      return false;
    }

    evolve_best_regi();
    if (!active_is_vstar()) {
      throw std::logic_error("Issue-1795 Active Regidrago V did not evolve");
    }

    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(*payload, "Earthen Vessel strict-JIT Dragon cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1795 Vessel payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-1795 finish");
    if (hand_count(Card::Grass) == 0 && !move_deck_to_hand(Card::Grass)) {
      throw std::logic_error("Issue-1795 final Grass target disappeared");
    }
    shuffle(state_.deck);
    trace("Earthen Vessel", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-JIT-01",
          "Discarded the held Dragon payload and searched the final Grass Energy.");

    if (!attach_manual()) {
      throw std::logic_error("Issue-1795 final Grass attachment became unavailable");
    }
    issue_1795_vessel_turn_ = 0;

    // The prior-turn Regidrago V legally evolves, Vessel pays the held Dragon as its
    // mandatory cost, and the searched Grass receives the turn's manual attachment.
    // The payload therefore enters discard on the same T4 ready-check turn:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official evolution, Item, discard, search, attachment, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Strict-JIT and success horizon: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#ready-state-and-t5-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1795
    return active_is_vstar() && state_.active->grass >= 2 &&
        state_.active->fire >= 1 && payload_ready();
  }

  int issue_1152_legacy_star_turn_ = 0;

  bool use_legacy_star_issue_1152() {
    // Compose the future-turn Treasure recovery before the existing post-Legacy
    // replay bridge so both confirmed routes remain active:
    // https://github.com/FlareZ123/pokemon-sims/issues/1417
    // https://github.com/FlareZ123/pokemon-sims/issues/1152
    const bool used = use_legacy_star_issue_1417();
    issue_1152_legacy_star_turn_ = used ? state_.turn : 0;
    return used;
  }

  void choose_supporter_issue_1152() {
    if (play_issue_1795_banked_steven()) return;
    if (issue_1481_hold_serena_for_free_latias_completion()) {
      trace("HOLD SUPPORTER", "P-DCI-01; P-COMPRESS-01",
            "Preserved Serena because Skyliner already completes the sole remaining Active axis.");
      return;
    }
    if (issue_1795_crispin_steven_vessel_route_available()) {
      // Play the first Supporter in the complete T2-T4 schedule before generic
      // Gladion selection consumes this turn's one Supporter action:
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
      // Official one-Supporter-per-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1795
      if (!play_crispin()) {
        throw std::logic_error(
            "Proven issue-1795 Crispin route became unavailable");
      }
      issue_1795_steven_turn_ = state_.turn + 1;
      trace("ROUTE SELECTION",
            "R-CRISPIN-01; R-STEVEN-01; R-EV-01; P-KNOWLEDGE-01; P-JIT-01; P-COMPRESS-01",
            "Selected Crispin over Gladion and committed the deterministic Steven and Earthen Vessel T4 finish.");
      return;
    }
    if (play_issue_1478_t3_arven_vessel_completion()) return;
    if (issue_1393_held_crispin_completion_available()) {
      // Crispin completes the proven GGF route before Gladion can consume the only
      // Supporter play for a slower Prize exchange:
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
      // Core Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest route and Supporter contention: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1393
      if (!play_crispin()) {
        throw std::logic_error(
            "Proven issue-1393 held Crispin completion became unavailable");
      }
      return;
    }
    if (play_issue_1199_route_after_attachment()) return;
    if (held_quick_ball_latias_completes_current_turn_without_arven()) {
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1208-held-quick-ball-latias", state_.turn,
            "HOLD SUPPORTER", "R-ARVEN-01; P-DCI-01; P-COMPRESS-01",
            "Retained Arven because held Quick Ball, a Dragon payload, and known Latias ex already complete the current-turn ready route.");
      }
      return;
    }
    choose_supporter();
    if (issue_1152_legacy_star_turn_ != state_.turn) {
      issue_1152_legacy_star_turn_ = 0;
      return;
    }
    issue_1152_legacy_star_turn_ = 0;
    if (!late_supporter_quick_ball_latias_completion_available()) return;

    // Legacy Star may recover a Supporter that searches Quick Ball after the
    // previous Item fixed point. Replay that newly live Item only through the
    // existing cost, GGF, payload, Bench, lock, Latias, Ability, and retreat gates:
    // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Core Supporter, Item, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route and strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1152
    play_items_until_stable(true);
    play_basics_from_hand();
  }

  void evolve_best_regi_issue_1224() {
    const int vstar_count_before = in_play_count(Card::RegidragoVstar);
    evolve_best_regi();
    if (in_play_count(Card::RegidragoVstar) <= vstar_count_before ||
        state_.vstar_power_used) {
      return;
    }

    // Forest Seal Stone remains attached when Regidrago V evolves, and its attached
    // Pokémon V may use Star Alchemy after that board-state transition. Re-evaluate
    // the still-unused Power once, then Bench a searched Basic through the existing
    // target, lock, Bench-space, K1, and once-per-game VSTAR Power guards:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Pokémon V and Forest Seal Stone ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
    // Official evolution, Ability, Bench, Supporter, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-route specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1224
    if (use_fss()) play_basics_from_hand();
  }

// Compose the active turn policy here so the evolution wrapper is scoped only to
// this replacement. The later include in part_014c.inc is suppressed by its guard.
#define evolve_best_regi evolve_best_regi_issue_1224
#define use_legacy_star use_legacy_star_issue_1152
#define choose_supporter choose_supporter_issue_1152
// Before the final shell attack phase, allow Skyliner to promote an energized
// Benched Regidrago V only when the existing Celestial Roar policy is live:
// Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
// Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
// Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
// Core Retreat and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
// Earliest legal ready-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
// Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1325
#define use_celestial_roar use_celestial_roar_after_latias_promotion
#define run_turn run_turn_issue1673_original
#include "part_014c_latias_bench_override.inc"
#undef run_turn
#include "part_issue_1673_secret_box_deadline_override.inc"
#undef use_celestial_roar
#undef choose_supporter
#undef use_legacy_star
#undef evolve_best_regi
