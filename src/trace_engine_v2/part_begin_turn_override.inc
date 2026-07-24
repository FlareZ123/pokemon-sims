  void begin_turn(const int turn) {
    state_.turn = turn;
    state_.supporter_used = false;
    state_.manual_energy_used = false;
    state_.retreat_used = false;
    state_.stadium_used = false;
    state_.turn_ended = false;
    state_.dark_asset_used = false;  // Dark Asset is limited per turn: https://api.pokemontcg.io/v2/cards/swsh3-104
    state_.discarded_this_turn.clear();

    // Drawing a card is the first step at the beginning of every turn, including
    // the first player's opening turn:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
    // The first player is restricted from playing a Supporter or attacking on that
    // turn; the restriction does not remove the required draw:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#First_Turn
    if (state_.deck.empty()) {
      state_.turn_ended = true;
      trace("GAME LOST", "R-GAME-DECK-OUT",
            "No cards remained in deck for the required start-of-turn draw.");
      return;
    }

    draw_card("start-of-turn");
    if (first_turn_going_first()) {
      trace("TURN START", "R-GAME-FIRST-TURN",
            "Going first: drew for the turn; no Supporter or attack is permitted.");
    }
  }
