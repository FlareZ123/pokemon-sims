          std::mt19937_64 quick_rng{962};
          Engine quick_first = make_shadow(quick_rng);
          if (!pay_quick_ball_cost(quick_first)) return false;
          quick_first.state_.bench.push_back(
              Pokemon{Card::TapuLeleGX, quick_first.state_.turn});
          quick_first.state_.hand.push_back(Card::Crispin);
          if (!pay_vessel_cost(quick_first)) return false;
          project_vessel_search(quick_first);
          const bool quick_first_route_live =
              quick_first.crispin_can_advance_energy_axis();
          if (!quick_first_route_live) return false;
      
          std::mt19937_64 vessel_rng{963};
          Engine vessel_first = make_shadow(vessel_rng);
          if (!pay_vessel_cost(vessel_first)) return false;
          project_vessel_search(vessel_first);
          const bool vessel_first_route_live = vessel_first.needs_tapu_connector() &&
              pay_quick_ball_cost(vessel_first);
      
          // Quick Ball discards before searching a Basic Pokémon, Tapu Lele-GX must be
          // Benched before Wonder Tag searches Crispin, Earthen Vessel pays a separate
          // discard before its Basic Energy search, and Crispin uses the one Supporter
          // play to attach one searched Energy and put the other into hand. Preempt
          // Vessel only when Quick Ball-first preserves both paid effects while
          // Vessel-first exhausts the legal Quick Ball cost:
          // https://api.pokemontcg.io/v2/cards/swsh1-179
          // https://api.pokemontcg.io/v2/cards/sm2-60
          // https://api.pokemontcg.io/v2/cards/sv7-133
          // https://api.pokemontcg.io/v2/cards/sv4-163
          // https://api.pokemontcg.io/v2/cards/swsh12-136
          // https://www.pokemon.com/us/pokemon-tcg/rules
          // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
          // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
          // https://github.com/FlareZ123/pokemon-sims/issues/962
          return !vessel_first_route_live;
        };
