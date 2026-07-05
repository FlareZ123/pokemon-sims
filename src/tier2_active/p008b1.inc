    for (const Card desired : priorities) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), desired);
      if (it == state_.prizes.end()) continue;
      const Card revealed = *it;
      *it = Card::HisuianHeavyBall;
      state_.hand.push_back(revealed);
      shuffle(state_.prizes);
      trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01", "Looked at Prizes and exchanged Heavy Ball for " + std::string(name(revealed)) + ".");
      return true;
    }
    state_.discard.push_back(Card::HisuianHeavyBall);
    shuffle(state_.prizes);
    trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01", "Looked at Prizes; did not reveal a useful Basic, so Heavy Ball was discarded.");
    return true;
  }

  bool need_regi() const { return !has_any_regi(); }
  bool need_vstar() const { return has_regi_v() && !has_vstar(); }
