#include "core/regidrago_line_helpers.inc"

  bool powerglass_energy_is_only_missing_axis() const {
    // Preserve the repository's semantic setup-axis test in one reusable helper:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    return need_energy() && !need_regi() && !need_vstar() &&
        !need_active_vstar() && !need_payload();
  }

  bool active_powerglass_tool_slot_needed() const {
    if (!state_.active || state_.active->tool != Tool::None ||
        !is_regidrago_v_line(state_.active->card)) {
      return false;
    }

    const bool powerglass_held = hand_count(Card::Powerglass) > 0;
    // Arven can search a Pokémon Tool. When the manual attachment is already spent
    // and Energy is the only missing setup axis, a Powerglass still in deck is the
    // immediate live route while Forest Seal Stone would occupy the same Tool slot:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool arven_searches_live_powerglass = supporter_allowed() &&
        hand_count(Card::Arven) > 0 && state_.manual_energy_used &&
        powerglass_energy_is_only_missing_axis() && might_be_unseen(Card::Powerglass);
    if (!powerglass_held && !arven_searches_live_powerglass) return false;

    // Powerglass must be attached to the Pokémon that will be Active when its
    // end-of-turn effect resolves, and that Pokémon may hold only one Tool:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool needed_grass_in_discard =
        state_.active->grass < 2 && count_of(state_.discard, Card::Grass) > 0;
    const bool needed_fire_in_discard =
        state_.active->fire < 1 && count_of(state_.discard, Card::Fire) > 0;
    return needed_grass_in_discard || needed_fire_in_discard;
  }

  bool can_hold_forest_seal_stone(const Pokemon& pokemon) const {
    // Forest Seal Stone may be attached to a Pokémon V with an open Tool slot:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // Pokémon V includes Pokémon VSTAR: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
    return is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None;
  }

  Pokemon* first_open_pokemon_v_on_bench() {
    for (Pokemon& pokemon : state_.bench) {
      if (can_hold_forest_seal_stone(pokemon)) return &pokemon;
    }
    return nullptr;
  }

  Pokemon* preferred_forest_seal_stone_holder() {
    // Preserve the existing Active-first attachment order, then fall back to Bench:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    if (state_.active && can_hold_forest_seal_stone(*state_.active)) {
      return &*state_.active;
    }
    return first_open_pokemon_v_on_bench();
  }

  bool attach_fss() {
    if (hand_count(Card::ForestSealStone) == 0) return false;

    // Forest Seal Stone has no Active-Spot requirement. When Powerglass needs the
    // Active Pokémon's only Tool slot, attach the Stone to another eligible Pokémon V:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (!state_.vstar_power_used && active_powerglass_tool_slot_needed()) {
      Pokemon* target = first_open_pokemon_v_on_bench();
      if (target != nullptr) {
        remove_one(state_.hand, Card::ForestSealStone);
        target->tool = Tool::ForestSealStone;
        trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL; R-POWERGLASS-01",
              "Attached Forest Seal Stone to a Benched Pokémon V and reserved the Active Tool slot for Powerglass.");
        return true;
      }

      // With no alternate Pokémon V holder, keep Forest Seal Stone in hand so the
      // live Powerglass route retains the Active Pokémon's one Tool slot:
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://api.pokemontcg.io/v2/cards/sv6pt5-63
      return false;
    }

    Pokemon* target = preferred_forest_seal_stone_holder();
    if (target == nullptr) return false;

    // Pokémon V includes Pokémon VSTAR, and the trace must identify the actual
    // holder selected by the legal attachment:
    // https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    remove_one(state_.hand, Card::ForestSealStone);
    target->tool = Tool::ForestSealStone;
    trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL",
          "Attached Forest Seal Stone to " + std::string(name(target->card)) + ".");
    return true;
  }
