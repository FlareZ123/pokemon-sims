#pragma once

  void reset_turn_action_usage() {
    state_.supporter_used = false;
    state_.manual_energy_used = false;
    state_.retreat_used = false;
    state_.stadium_used = false;
    state_.dark_asset_used = false;  // Dark Asset is limited per turn: https://api.pokemontcg.io/v2/cards/swsh3-104
  }

  void reset_turn_transients() {
    state_.turn_ended = false;
    state_.discarded_this_turn.clear();
    // In the Garbodor pressure scenario, path_lock_removed is reused only as the
    // current-turn "opponent Tool removed" flag. The maximum-pressure opponent
    // reattaches a Tool before each following Regidrago turn, so a Field Blower
    // unlock can never persist across turns:
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    if (garbodor_shake_scenario()) state_.path_lock_removed = false;
  }

  void reset_turn_state() {
    reset_turn_action_usage();
    reset_turn_transients();
  }

  void reset_per_turn_state(const int turn) {
    state_.turn = turn;
    reset_turn_state();
  }

  void record_deck_out_loss() {
    state_.turn_ended = true;
    trace("GAME LOST", "R-GAME-DECK-OUT",
          "No cards remained in deck for the required start-of-turn draw.");
  }

  bool draw_start_of_turn_card() {
    // Drawing a card is the first step at the beginning of every turn, including
    // the first player's opening turn:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
    if (!deck_has_cards()) {
      record_deck_out_loss();
      return false;
    }

    draw_card("start-of-turn");
    return true;
  }

  void trace_first_turn_restrictions() {
    // The first player is restricted from playing a Supporter or attacking on that
    // turn; the restriction does not remove the required draw:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#First_Turn
    trace("TURN START", "R-GAME-FIRST-TURN",
          "Going first: drew for the turn; no Supporter or attack is permitted.");
  }

  void begin_turn(const int turn) {
    reset_per_turn_state(turn);
    if (!draw_start_of_turn_card()) return;

    if (first_turn_going_first()) {
      trace_first_turn_restrictions();
    }
  }
