      for (Pokemon& p : state_.bench) {
        if (is_pokemon_v(p.card) && p.tool == Tool::None) { target = &p; break; }
      }
    }
    if (target == nullptr) return false;
    remove_one(state_.hand, Card::ForestSealStone);
    target->tool = Tool::ForestSealStone;
    trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL; R-GAME-ITEM", "Attached Forest Seal Stone to Regidrago V.");
    return true;
  }

  bool should_play_steven() const {
    if (state_.turn != 1 || scenario_.going_first || !has_regi_v()) return false;
    const int axes = static_cast<int>(need_vstar() && hand_count(Card::RegidragoVstar) == 0) +
                     static_cast<int>(need_energy() && hand_count(Card::Crispin) == 0) +
                     static_cast<int>(need_payload() && hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0);
    return axes >= 2;
  }

  bool can_fetch_oricorio_for_compression() const {
    return need_energy() && bench_space() > 0 && !in_play(Card::Oricorio) &&
           hand_count(Card::Oricorio) == 0 && deck_count_after_search_started(Card::Oricorio) > 0 &&
           deck_count_after_search_started(Card::Grass) + deck_count_after_search_started(Card::Fire) >= 2;
  }

