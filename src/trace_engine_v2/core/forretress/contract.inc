#pragma once

  // Engine declaration contract for the cohesive Forretress module. Definitions
  // stay beside the namespace-scope runtime and scenario owners rather than being
  // duplicated across legacy part_*.inc files.
  // Canonical runtime: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/runtime.inc
  // Canonical scenarios: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_forretress_ex_combo.inc
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/972

  // Compose the canonical board-query policy through the existing in-class
  // Forretress declaration boundary. Qualified runtime definitions emitted after
  // Engine closes can then reuse these members without duplicating traversal or
  // board-index vocabulary.
  // C++ textual include semantics: https://eel.is/c++draft/cpp.include
  // Core evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
#include "../board_state_policy.inc"

  // Exploding Energy destinations use the canonical board index type owned by
  // board_state_policy.inc: 0 is Active and each Bench slot is index + 1.
  // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  using AttachmentDestinations = std::vector<BoardIndex>;

  // Combo availability and board setup. Keep this order aligned with runtime.inc
  // so declaration ownership is easy to audit across the textual class boundary.
  [[nodiscard]] bool forretress_combo_enabled() const;
  [[nodiscard]] bool needs_pineco_connector() const;
  [[nodiscard]] bool needs_forretress_connector() const;
  bool bench_pineco_if_useful();
  bool evolve_forretress_ex();

  // Exploding Energy source/target indexing delegates to board_state_policy.inc,
  // while this module retains printed-effect resolution and setup strategy.
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Board policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc
  [[nodiscard]] OptionalBoardIndex exploding_energy_regidrago_target_index() const;
  void discard_forretress_ex_stack(const Pokemon& knocked_out);
  bool resolve_exploding_energy(
      const AttachmentDestinations& attachment_destinations);
  bool use_exploding_energy_for_setup();
  bool advance_forretress_combo();

  // Search connectors used by the combo pipeline. Hidden-information fallbacks
  // remain beside their callers instead of being mixed into board mutation.
  [[nodiscard]] Card fallback_combo_basic_after_search_started() const;
  [[nodiscard]] Card fallback_combo_pokemon_after_search_started() const;
  bool play_quick_ball_for_pineco();
  bool play_ultra_ball_for_forretress_combo();
  bool play_evolution_incense_for_forretress();
  bool run_forretress_combo_search_step(bool permit_payload);