#include "part_issue_1199_steven_package_override.inc"

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
#include "part_014c_latias_bench_override.inc"
#undef use_celestial_roar
#undef choose_supporter
#undef use_legacy_star
#undef evolve_best_regi
