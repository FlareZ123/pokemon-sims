#pragma once

  // Engine declaration contract for the cohesive Forretress module. Definitions
  // stay beside the namespace-scope runtime and scenario owners rather than being
  // duplicated across legacy part_*.inc files.
  // Canonical runtime: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/runtime.inc
  // Canonical scenarios: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_forretress_ex_combo.inc
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/972

  // Compose board traversal, board-index vocabulary, and shared evolution timing
  // through one policy include. Qualified runtime definitions emitted after Engine
  // closes can then reuse those members without redeclaring container conventions.
  // C++ textual include semantics: https://eel.is/c++draft/cpp.include
  // Core evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
#include "../board_state_policy.inc"

  // Board and route-state queries.
  [[nodiscard]] bool forretress_combo_enabled() const;
  [[nodiscard]] bool needs_pineco_connector() const;
  [[nodiscard]] bool needs_forretress_connector() const;
  [[nodiscard]] Pokemon* pokemon_at_board_index(BoardIndex index);
  [[nodiscard]] OptionalBoardIndex exploding_energy_regidrago_target_index() const;
  [[nodiscard]] Card fallback_combo_basic_after_search_started() const;
  [[nodiscard]] Card fallback_combo_pokemon_after_search_started() const;

  // Board mutation and Exploding Energy resolution.
  bool bench_pineco_if_useful();
  bool evolve_forretress_ex();
  void discard_forretress_ex_stack(const Pokemon& knocked_out);
  bool resolve_exploding_energy(
      const AttachmentDestinations& attachment_destinations);
  bool use_exploding_energy_for_setup();
  bool advance_forretress_combo();

  // Search connectors used by the combo pipeline.
  bool play_quick_ball_for_pineco();
  bool play_ultra_ball_for_forretress_combo();
  bool play_evolution_incense_for_forretress();
  bool run_forretress_combo_search_step(bool permit_payload);
