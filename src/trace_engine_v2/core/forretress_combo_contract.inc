  // Test-variant combo support. Definitions are emitted after Engine closes so
  // these declarations are visible to opening setup, connector policy, and the
  // active turn composition without disturbing the split translation unit.
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/972

  // Combo state and board queries.
  bool forretress_combo_enabled() const;
  bool needs_pineco_connector() const;
  bool needs_forretress_connector() const;
  Pokemon* pokemon_at_board_index(std::size_t index);
  std::optional<std::size_t> exploding_energy_regidrago_target_index() const;
  Card fallback_combo_basic_after_search_started() const;
  Card fallback_combo_pokemon_after_search_started() const;

  // Board mutation and ability resolution.
  bool bench_pineco_if_useful();
  bool evolve_forretress_ex();
  void discard_forretress_ex_stack(const Pokemon& knocked_out);
  bool resolve_exploding_energy(
      const std::vector<std::size_t>& attachment_destinations);
  bool use_exploding_energy_for_setup();
  bool advance_forretress_combo();

  // Search connectors used by the combo pipeline.
  bool play_quick_ball_for_pineco();
  bool play_ultra_ball_for_forretress_combo();
  bool play_evolution_incense_for_forretress();
  bool run_forretress_combo_search_step(bool permit_payload);
