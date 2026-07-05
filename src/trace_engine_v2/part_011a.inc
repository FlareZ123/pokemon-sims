  bool use_fss() {
    if (state_.vstar_power_used) return false;
    Pokemon* holder = nullptr;
    if (state_.active && state_.active->tool == Tool::ForestSealStone) holder = &*state_.active;
    if (holder == nullptr) {
      for (Pokemon& p : state_.bench) if (p.tool == Tool::ForestSealStone) { holder = &p; break; }
    }
    if (holder == nullptr || !is_pokemon_v(holder->card)) return false;
    const bool useful = should_play_steven() || need_regi() || need_vstar() || need_energy() || need_payload() || need_active_vstar();
    if (!useful) return false;
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    Card target = fss_target_after_search_started();
    bool found = move_deck_to_hand(target);
    if (!found) {
      for (const Card fallback : {Card::RegidragoVstar, Card::Crispin, Card::BrilliantBlender,
                                  Card::ProfessorBurnet, Card::Arven, Card::MysteriousTreasure,
                                  Card::QuickBall, Card::EarthenVessel, Card::LatiasEx}) {
        if (move_deck_to_hand(fallback)) { target = fallback; found = true; break; }
      }
    }
    shuffle(state_.deck);
    trace("STAR ALCHEMY", "R-FSS-02; R-VSTAR-01", found ? "Searched any card: " + std::string(name(target)) + "." : "No card remained in deck.");
    return true;
  }

