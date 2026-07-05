      if (std::any_of(state_.hand.begin(), state_.hand.end(), is_basic)) break;
      state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
      state_.hand.clear();
      shuffle(state_.deck);
      ++outcome_.mulligans;
    }

    choose_opening_active();
    choose_opening_bench();
    shuffle(state_.deck);
    for (int prize = 0; prize < 6; ++prize) {
      state_.prizes.push_back(state_.deck.back());
      state_.deck.pop_back();
    }

    if (trace_ != nullptr && trace_->enabled) {
      trace_->note("===== SIMULATE THIS =====");
      trace_->note("Scenario: " + scenario_.label + " | DCI: " + dci_name() + " | Locks: " + lock_name());
      trace_->note("Mulligans: " + std::to_string(outcome_.mulligans));
      trace_->note("Opening player-known state: " + state_line());
      trace_->note("DEBUG ONLY: prizes=" + join_cards(state_.prizes) + ". Policy may use them only after a legal deck or Prize inspection.");
      trace_->note("Policy rule: strict and matchup-flex JIT require a payload to enter discard in the same player turn as the ready-state check.");
    }
