      if (bench_oricorio_if_useful()) return true;
      if (play_earthen_vessel(permit_payload)) return true;
      if (play_quick_ball(permit_payload)) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
    }
    if (permit_payload && need_payload()) {
      if (play_brilliant_blender()) return true;
      // Evolution Incense can fetch a modeled Evolution Dragon for a separately
      // held discard Item in this same ready turn: https://api.pokemontcg.io/v2/cards/swsh1-163
      if (play_evolution_incense(permit_payload)) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
      if (play_quick_ball(permit_payload)) return true;
      // Ultra Ball's discard cost can put a current-turn Dragon payload in discard: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      if (play_ultra_ball(permit_payload)) return true;
      if (play_earthen_vessel(permit_payload)) return true;
    }
    return false;
  }

  void play_items_until_stable(const bool permit_payload) {
    if (item_locked() || state_.turn_ended) return;
    for (int guard = 0; guard < 28; ++guard) {
      if (play_heavy_ball()) continue;
      if (!run_search_items_one_step(permit_payload)) break;
    }
  }