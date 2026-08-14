  void choose_supporter_issue3545_projection() {
    // The production turn runs the current Item fixed point before the Supporter
    // action. A BC/VS projection must reproduce that boundary before deciding whether
    // the recovered Supporter is actually consumed. Otherwise an Item search may
    // change the best Supporter after VS has already been spent, as seen when a
    // recovered Gladion lost to an already-held Steven's Resolve after Mysterious
    // Treasure completed the Regidrago axis.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Production turn order: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3545
    play_items_until_stable_issue_3057(true);
    choose_supporter_issue_1152();
  }
