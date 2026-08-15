#pragma once

  struct TurnActionStatePolicy {
    static void reset(State& state) {
      state.supporter_used = false;
      state.manual_energy_used = false;
      state.retreat_used = false;
      state.stadium_used = false;
      state.dark_asset_used = false;  // Dark Asset is limited per turn: https://api.pokemontcg.io/v2/cards/swsh3-104
      state.turn_ended = false;
      state.discarded_this_turn.clear();
    }
  };

  void reset_turn_action_state() {
    TurnActionStatePolicy::reset(state_);
  }

  void reset_per_turn_state(const int turn) {
    state_.turn = turn;
    reset_turn_action_state();

    // In the Garbodor pressure scenario path_lock_removed is reused only as the
    // current-turn "opponent Tool removed" flag. The maximum-pressure opponent
    // reattaches a Tool before each following Regidrago turn, so Field Blower's
    // unlock never persists across turns:
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    if (garbodor_shake_scenario()) state_.path_lock_removed = false;
  }

  bool draw_start_of_turn_card() {
    // Drawing a card is the first step at the beginning of every turn, including
    // the first player's opening turn:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
    if (!deck_has_cards()) {
      state_.turn_ended = true;
      trace("GAME LOST", "R-GAME-DECK-OUT",
            "No cards remained in deck for the required start-of-turn draw.");
      return false;
    }

    draw_card("start-of-turn");
    return true;
  }

  void trace_first_turn_restriction() {
    // The first player is restricted from playing a Supporter or attacking on
    // that turn; the restriction does not remove the required draw:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#First_Turn
    trace("TURN START", "R-GAME-FIRST-TURN",
          "Going first: drew for the turn; no Supporter or attack is permitted.");
  }

  void begin_turn(const int turn) {
    reset_per_turn_state(turn);
    if (!draw_start_of_turn_card()) return;
    if (first_turn_going_first()) trace_first_turn_restriction();
  }
