  bool attach_live_fss() {
    // Forest Seal Stone grants a VSTAR Power, and a player may use only one VSTAR
    // Power during a game. Once that resource is spent, attaching the Stone cannot
    // advance setup and would consume the holder's one Tool slot:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    if (state_.vstar_power_used) return false;
    return attach_fss();
  }

  bool attach_powerglass() {
    // Powerglass is a Pokémon Tool, so it can be attached during the turn even
    // through Item lock. Its delayed effect matters only on the Active holder:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    if (hand_count(Card::Powerglass) == 0 || !state_.active || state_.active->tool != Tool::None ||
        (state_.active->card != Card::RegidragoV && state_.active->card != Card::RegidragoVstar)) return false;

    // Powerglass attaches Energy to its own Active holder, not to the policy's
    // separately selected Regidrago line: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed == 0 && active_fire_needed == 0) return false;
    const bool needed_grass_in_discard = active_grass_needed > 0 && count_of(state_.discard, Card::Grass) > 0;
    const bool needed_fire_in_discard = active_fire_needed > 0 && count_of(state_.discard, Card::Fire) > 0;
    if (!needed_grass_in_discard && !needed_fire_in_discard) return false;

    remove_one(state_.hand, Card::Powerglass);
    state_.active->tool = Tool::Powerglass;
    trace("ATTACH TOOL", "R-POWERGLASS-01; R-GAME-TOOL",
          "Attached Powerglass to the Active " + std::string(name(state_.active->card)) + ".");
    return true;
  }

  bool resolve_powerglass_end_turn() {
    // Powerglass resolves at the end of the turn after the attack timing point and
    // only while its holder is Active: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    if (!state_.active || state_.active->tool != Tool::Powerglass) return false;

    // The effect can attach only to the attached Active Pokémon. Determine the
    // Energy type from that holder's own GGF deficit: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed == 0 && active_fire_needed == 0) return false;

    Card energy = Card::Grass;
    if (active_grass_needed > 0 && count_of(state_.discard, Card::Grass) > 0) {
      energy = Card::Grass;
    } else if (active_fire_needed > 0 && count_of(state_.discard, Card::Fire) > 0) {
      energy = Card::Fire;
    } else {
      return false;
    }

    if (!remove_one(state_.discard, energy)) throw std::logic_error("Powerglass Energy target disappeared");
    if (energy == Card::Grass) ++state_.active->grass; else ++state_.active->fire;
    trace("POWERGLASS", "R-POWERGLASS-01",
          "Attached " + std::string(name(energy)) + " from discard at end of turn.");
    return true;
  }

  bool play_chaotic_swell() {
    // Chaotic Swell is a Stadium, so Item lock does not prevent it from being played.
    // A different Stadium coming into play discards the Stadium already in play:
    // https://api.pokemontcg.io/v2/cards/sm12-187
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // Path to the Peak suppresses Rule Box Pokémon Abilities only while it remains
    // in play: https://api.pokemontcg.io/v2/cards/swsh6-148
    if (state_.stadium_used || !rule_box_abilities_locked() ||
        hand_count(Card::ChaoticSwell) == 0 ||
        state_.stadium == Stadium::ChaoticSwell) return false;

    remove_one(state_.hand, Card::ChaoticSwell);
    state_.stadium_used = true;
    state_.stadium = Stadium::ChaoticSwell;
    state_.path_lock_removed = true;
    trace("PLAY STADIUM", "R-CHAOTIC-SWELL-01; R-PATH-01",
          "Played Chaotic Swell, replacing the modeled Path to the Peak and restoring Rule Box Pokémon Abilities.");
    return true;
  }

  bool play_field_blower() {
    if (item_locked() || hand_count(Card::FieldBlower) == 0 || !rule_box_abilities_locked()) return false;

    // Field Blower may choose a Stadium in play and discard it. The Rule Box lock
    // scenario represents Path to the Peak, whose suppression ends when it leaves play:
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    remove_one(state_.hand, Card::FieldBlower);
    state_.discard.push_back(Card::FieldBlower);
    state_.path_lock_removed = true;
    trace("PLAY ITEM", "R-GAME-ITEM",
          "Field Blower discarded the modeled Path to the Peak; Rule Box Pokémon Abilities are available again.");
    return true;
  }

  void run_turn() {
    trace("POLICY", "P-AXIS-01", "Start: " + state_line());
    play_chaotic_swell();
    play_field_blower();
    // Forest of Vitality is a Stadium, not an Item. Play it before on-play Rule
    // Box Pokémon such as Tapu Lele-GX so replacing Path to the Peak can restore
    // Wonder Tag before that Pokémon enters the Bench:
    // https://api.pokemontcg.io/v2/cards/me1-117
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/1096
    advance_forretress_combo();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());
    attach_live_fss();
    use_fss();
    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());

    attach_manual();
    // Steven's Resolve ends the turn, so attach a live Powerglass before choosing
    // that Supporter: https://api.pokemontcg.io/v2/cards/sm7-145
    if (should_play_steven()) attach_powerglass();
    choose_supporter();
    if (state_.turn_ended) {
      trace("TURN END", "R-STEVEN-01 or R-RV-01", state_line());
      return;
    }

    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    attach_live_fss();
    use_fss();
    play_chaotic_swell();
    play_field_blower();
    play_items_until_stable(!strict_payload_timing());
    attach_manual();
    evolve_best_regi();
    play_basics_from_hand();
    play_items_until_stable(true);
    // A strict-JIT Item route may now discard an in-hand Dragon payload and search
    // Regidrago VSTAR through Mysterious Treasure (https://api.pokemontcg.io/v2/cards/sm6-113).
    // Retry evolution so a Regidrago V that entered play on a prior turn can evolve
    // before this turn's ready check under the core evolution procedure (https://www.pokemon.com/us/pokemon-tcg/rules).
    evolve_best_regi();
    // A late Item search can put Tapu Lele-GX into hand, and Bench play can then
    // fetch a new Supporter with Wonder Tag. Reconsider that unused Supporter slot
    // before the ready check: https://api.pokemontcg.io/v2/cards/sm2-60
    // Professor Burnet can discard a current-turn deck payload after this chain:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    if (!state_.supporter_used && setup_axis_missing()) choose_supporter();
    if (use_legacy_star()) {
      // Core procedure source: https://www.pokemon.com/us/pokemon-tcg/rules
      // Recovered Items may be played later in the same turn.
      play_chaotic_swell();
      play_field_blower();
      play_basics_from_hand();
      play_items_until_stable(true);
      // Legacy Star can return a Supporter from discard, including Crispin:
      // https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/sv7-133
      // A player may use one available Supporter during the turn before attacking:
      // https://www.pokemon.com/us/pokemon-tcg/rules
      if (!state_.supporter_used && setup_axis_missing()) choose_supporter();
    }
    attach_manual();
    retreat_to_benched_vstar_with_latias();
    if (need_active_vstar() && !state_.supporter_used) play_tate_switch();
    // Attach only after the final modeled switch decision, while the intended holder
    // is known to remain Active for Powerglass's end-of-turn condition:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    attach_powerglass();
    use_celestial_roar();
    trace("POLICY", "P-AXIS-01", "End: " + state_line());
  }

  std::optional<std::array<Card, 2>> grant_recovery_strict_jit_costs() {
    // Grant's discard-pile recovery is available only during the player's turn.
    // Once an attack or another turn-ending effect sets turn_ended, readiness
    // observation must not create a new gameplay action:
    // Grant: https://api.pokemontcg.io/v2/cards/swsh10-144
    // Official turn and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Existing turn-ended action gate: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L157-L161
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1829
    if (state_.turn_ended || !strict_payload_timing() || !need_payload() ||
        !active_is_vstar() || need_energy() ||
        count_of(state_.discard, Card::Grant) == 0) {
      return std::nullopt;
    }

    const std::vector<Card> original_hand = state_.hand;
    const bool flex_fodder = scenario_.dci == DciProfile::MatchupFlexJit;
    const auto payload = choose_discard(true, flex_fodder);
    if (!payload || !is_payload(*payload) || !remove_one(state_.hand, *payload)) {
      state_.hand = original_hand;
      return std::nullopt;
    }

    // Grant's auxiliary effect forbids Grant itself as either of the two costs.
    // Remove any held copy only from this policy projection, then require the
    // ordinary cost to pass the existing UDP/DCI selector:
    // Grant: https://api.pokemontcg.io/v2/cards/swsh10-144
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1790
    state_.hand.erase(std::remove(state_.hand.begin(), state_.hand.end(),
                                  Card::Grant),
                      state_.hand.end());
    const auto ordinary = choose_discard(false, flex_fodder);
    state_.hand = original_hand;
    if (!ordinary || *ordinary == Card::Grant) return std::nullopt;
    return std::array<Card, 2>{*payload, *ordinary};
  }

  bool recover_grant_for_strict_jit_payload() {
    const auto costs = grant_recovery_strict_jit_costs();
    if (!costs) return false;

    // This action is admitted only when Active Regidrago VSTAR already pays GGF
    // and the current-turn Dragon payload is the sole missing ready-state axis.
    // Grant returns from discard without consuming the turn's Supporter action:
    // Grant: https://api.pokemontcg.io/v2/cards/swsh10-144
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Trainer and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Strict-JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1790
    if (!discard_from_hand((*costs)[0], "Grant strict-JIT Dragon cost",
                           "R-GRANT-01; P-DCI-01; P-JIT-01") ||
        !discard_from_hand((*costs)[1], "Grant ordinary DCI cost",
                           "R-GRANT-01; P-DCI-01")) {
      throw std::logic_error("Grant recovery costs disappeared");
    }
    if (!recover_discard_to_hand(Card::Grant)) {
      throw std::logic_error("Grant recovery target disappeared");
    }
    trace("GRANT RECOVERY", "R-GRANT-01; P-DCI-01; P-JIT-01",
          "Discarded " + std::string(name((*costs)[0])) + " and " +
              std::string(name((*costs)[1])) +
              "; returned Grant to hand without using the Supporter action.");
    return true;
  }

  void record_ready() {
    recover_grant_for_strict_jit_payload();
    const bool active_vstar = active_is_vstar();
    const bool energy_ready = active_vstar && pays_apex_energy_cost(*state_.active);
    // Preserve historical per-type T2 diagnostics for canonical Basic-Energy decks.
    // Only a physical DDE changes those predicates: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Enhancement audit: https://github.com/FlareZ123/pokemon-sims/issues/2238
    const bool has_dde = active_vstar && state_.active->double_dragon > 0;
    const bool grass_ready = active_vstar &&
        (has_dde ? energy_ready : state_.active->grass >= 2);
    const bool fire_ready = active_vstar &&
        (has_dde ? energy_ready : state_.active->fire >= 1);
    const bool payload_is_ready = payload_ready();
    const bool ready = state_.turn >= 2 && active_vstar && energy_ready &&
        payload_is_ready;  // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ ; Apex GGF: https://api.pokemontcg.io/v2/cards/swsh12-136 ; https://github.com/FlareZ123/pokemon-sims/issues/2238
    if (!ready) {
      if (state_.turn == 2) {
        outcome_.t2_missing_active_vstar = !active_vstar;
        outcome_.t2_missing_grass = !grass_ready;
        outcome_.t2_missing_fire = !fire_ready;
        outcome_.t2_missing_payload = !payload_is_ready;
        outcome_.t2_eligible_missed_active =
            outcome_.t2_eligible_regi_start && !active_vstar;
        outcome_.t2_eligible_held_vstar_missed_active =
            outcome_.t2_eligible_regi_start && outcome_.t2_vstar_held_start &&
            !active_vstar;
      }
      return;
    }
    if (outcome_.first_ready_turn == 0) {
      outcome_.first_ready_turn = state_.turn;
      trace("READY", "R-RVS-01; P-JIT-01", "Active Regidrago VSTAR has GGF and a permitted Dragon payload in discard.");
    }
    if (state_.turn <= 2) outcome_.ready_by_2 = true;
    if (state_.turn <= 3) outcome_.ready_by_3 = true;
    if (state_.turn <= 4) outcome_.ready_by_4 = true;
    if (state_.turn <= 5) outcome_.ready_by_5 = true;
  }

  // The original run_turn() above is macro-renamed by the active composition. End
  // that rename and compose the Latias helper and replacement while Engine is open:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/regidrago_sim.cpp#L165-L169
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c_latias_bench_override.inc#L1-L105
#undef run_turn
#include "part_014c_issue_1152_bridge.inc"
#define use_legacy_star use_legacy_star_issue_1152
#define choose_supporter choose_supporter_issue_1152
#include "part_014c_latias_bench_override.inc"
#undef choose_supporter
#undef use_legacy_star
};

// Pineco and Forretress ex mechanics are shared by exact-state fixtures and the
// registered regidrago-pineco recipe. Qualified definitions live after Engine closes:
// https://api.pokemontcg.io/v2/cards/sv4pt5-1
// https://api.pokemontcg.io/v2/cards/sv4pt5-2
// https://github.com/FlareZ123/pokemon-sims/issues/972
#include "part_forretress_ex_combo.inc"

DeckRecipe baseline_recipe() {
  return DeckRecipe(kDeckRecipe.begin(), kDeckRecipe.end());
}

DeckRecipe double_dragon_modeling_recipe() {
  // Validation-only recipe: replace one Basic Grass and one Basic Fire with two DDE.
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  DeckRecipe recipe = baseline_recipe();
  for (auto& [card, copies] : recipe) {
    if (card == Card::Grass) --copies;
    if (card == Card::Fire) --copies;
  }
  recipe.push_back({Card::DoubleDragonEnergy, 2});
  return recipe;
}

DeckRecipe pineco_recipe() {
  // Exact issue-1118 Secret Box recipe. Appletun is a Stage 1 Dragon payload and
  // remains unplayable because the recipe intentionally contains no Applin:
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://github.com/FlareZ123/pokemon-sims/issues/1118
  return {
      {Card::Grant, 1},              {Card::FieldBlower, 1},
      {Card::ErikasInvitation, 1},   {Card::ProfessorBurnet, 1},
      {Card::Dawn, 2},               {Card::QuickBall, 3},
      {Card::Pineco, 2},             {Card::ForretressEx, 2},
      {Card::TapuLeleGX, 4},         {Card::Guzma, 1},
      {Card::WishfulBaton, 1},       {Card::StevensResolve, 1},
      {Card::ForestSealStone, 1},    {Card::ForestOfVitality, 2},
      {Card::Gladion, 2},            {Card::HisuianHeavyBall, 1},
      {Card::Crispin, 1},            {Card::Arven, 2},
      {Card::MysteriousTreasure, 4}, {Card::RegidragoV, 4},
      {Card::RegidragoVstar, 3},     {Card::DialgaGX, 1},
      {Card::Dragapult, 2},          {Card::MegaDragonite, 2},
      {Card::GoodraVstar, 1},        {Card::Appletun, 1},
      {Card::ProfessorTuro, 1},      {Card::EarthenVessel, 2},
      {Card::Grass, 6},              {Card::Fire, 3},
      {Card::SecretBox, 1},
  };
}

struct NamedDeck {
  std::string id;
  DeckRecipe recipe;
};

bool validate_recipe(const NamedDeck& deck, std::string* error = nullptr) {
  const auto fail = [&](const std::string& message) {
    if (error != nullptr) *error = deck.id + ": " + message;
    return false;
  };
  if (deck.id.empty()) return fail("deck ID is empty");

  int total = 0;
  int ace_specs = 0;
  std::map<Card, int> copies_by_card;
  for (const auto& [card, copies] : deck.recipe) {
    if (copies <= 0) return fail("card copy count must be positive");
    total += copies;
    copies_by_card[card] += copies;
    // Basic Energy has no four-copy limit; DDE is Special Energy and does.
    // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/2238
    if (!is_basic_energy(card) && copies_by_card[card] > 4) {
      return fail(std::string(name(card)) + " exceeds the four-copy limit");
    }
    if (is_ace_spec(card)) ace_specs += copies;
  }
  if (total != 60) return fail("recipe must contain exactly 60 cards");
  if (ace_specs > 1) return fail("recipe contains more than one ACE SPEC");
  if (copies_by_card[Card::RegidragoVstar] > 0 &&
      copies_by_card[Card::RegidragoV] == 0) {
    return fail("Regidrago VSTAR has no Regidrago V");
  }
  if (copies_by_card[Card::ForretressEx] > 0 &&
      copies_by_card[Card::Pineco] == 0) {
    return fail("Forretress ex has no Pineco");
  }
  // Appletun has no Applin in either named recipe. That is legal because the
  // simulator represents it solely as an Apex Dragon discard payload and never
  // admits it as a Basic or an in-play evolution:
  // https://api.pokemontcg.io/v2/cards/sv8-140
  if (copies_by_card[Card::Appletun] > 0 &&
      (is_basic(Card::Appletun) || !is_payload(Card::Appletun))) {
    return fail("Appletun must remain a non-Basic Dragon payload");
  }
  return true;
}

const std::vector<NamedDeck>& deck_registry() {
  static const std::vector<NamedDeck> decks = [] {
    std::vector<NamedDeck> result{
        {"regidrago-shell", baseline_recipe()},
        {"regidrago-pineco", pineco_recipe()},
    };
    for (const NamedDeck& deck : result) {
      std::string error;
      if (!validate_recipe(deck, &error)) throw std::logic_error(error);
    }
    return result;
  }();
  return decks;
}

const NamedDeck* deck_by_id(const std::string_view id) {
  const std::string_view normalized =
      id == "regidrago-pineco-secret-box" ? "regidrago-pineco" : id;
  if (normalized == "regidrago-dde-model") {
    static const NamedDeck dde_model = [] {
      NamedDeck deck{"regidrago-dde-model", double_dragon_modeling_recipe()};
      std::string error;
      if (!validate_recipe(deck, &error)) throw std::logic_error(error);
      return deck;
    }();
    return &dde_model;  // Validation-only recipe: https://github.com/FlareZ123/pokemon-sims/issues/2238 ; DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  }
  const auto& decks = deck_registry();
  const auto found = std::find_if(
      decks.begin(), decks.end(), [normalized](const NamedDeck& deck) {
        return deck.id == normalized;
      });
  return found == decks.end() ? nullptr : &*found;
}

std::vector<Scenario> all_scenarios() {
  std::vector<Scenario> result;
  // Preserve historical surviving-row order. Seed slots for the two retired
  // full-turn-one Item-lock rows remain reserved by the aggregate writers:
  // https://github.com/FlareZ123/pokemon-sims/issues/1118
  // https://github.com/FlareZ123/pokemon-sims/issues/2247
  for (const bool first : {true, false}) {
    const std::string seat = first ? "go-first" : "go-second";
    result.push_back({"strict-jit/" + seat, DciProfile::StrictJit, LockMode::None, first, 5});
    result.push_back({"matchup-flex-jit/" + seat, DciProfile::MatchupFlexJit, LockMode::None, first, 5});
    result.push_back({"no-discard-control/" + seat, DciProfile::NoDiscardControl, LockMode::None, first, 5});
    result.push_back({"strict-jit-turn2-item-lock/" + seat, DciProfile::StrictJit, LockMode::TurnTwoItem, first, 5});
    // Full-turn-one Item lock is intentionally absent from current-paper aggregate
    // scenarios. The starting player cannot attack on the first turn, and Forest
    // of Giant Plants is banned in Expanded. Keep FullItem only for synthetic tests:
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf
    // https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/
    // https://github.com/FlareZ123/pokemon-sims/issues/2247
    result.push_back({"strict-jit-rulebox-ability-lock/" + seat, DciProfile::StrictJit, LockMode::FullRuleBoxAbility, first, 5});
    // FullCombined keeps Rule Box Ability suppression from turn 1, while its Item
    // component starts on turn 2 in Engine::item_locked():
    // https://github.com/FlareZ123/pokemon-sims/issues/2247
    result.push_back({"strict-jit-combined-lock/" + seat, DciProfile::StrictJit, LockMode::FullCombined, first, 5});
  }
  result.push_back({"strict-jit-supporter-lock/go-first",
                    DciProfile::StrictJit, LockMode::FullSupporter, true, 5});
  result.push_back({"strict-jit-supporter-lock/go-second",
                    DciProfile::StrictJit, LockMode::FullSupporter, false, 5});
  return result;
}

std::optional<Scenario> scenario_by_label(const std::string& label) {
  for (const Scenario& scenario : all_scenarios()) if (scenario.label == label) return scenario;