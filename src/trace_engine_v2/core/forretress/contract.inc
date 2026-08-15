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

  // Exploding Energy keeps its card-facing name while reusing the canonical
  // board destination container owned by board_state_policy.inc. Board index 0
  // is Active and each Bench slot is index + 1.
  // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  using AttachmentDestinations = BoardAttachmentDestinations;

  // Keep repeated Forretress/Regidrago board-role questions in one card-facing
  // contract while delegating intrinsic Regidrago-line identity to the canonical
  // card classifier.
  [[nodiscard]] static constexpr bool is_pineco_board_pokemon(
      const Pokemon& pokemon) {
    return pokemon.card == Card::Pineco; // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  }

  [[nodiscard]] static constexpr bool is_forretress_ex_board_pokemon(
      const Pokemon& pokemon) {
    return pokemon.card == Card::ForretressEx; // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  }

  [[nodiscard]] static constexpr bool is_forretress_line_board_pokemon(
      const Pokemon& pokemon) {
    // Centralize the Pineco -> Forretress ex board-line identity used by setup
    // orchestration without moving evolution timing or route policy out of Engine.
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    return is_pineco_board_pokemon(pokemon) ||
           is_forretress_ex_board_pokemon(pokemon);
  }

  [[nodiscard]] static constexpr bool is_regidrago_board_pokemon(
      const Pokemon& pokemon) {
    return is_regidrago_v_line(pokemon.card); // Canonical classifier: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc ; Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  }

  // Combo availability and board setup. Keep this order aligned with runtime.inc
  // so declaration ownership is easy to audit across the textual class boundary.
  [[nodiscard]] bool forretress_combo_enabled() const;
  [[nodiscard]] bool needs_pineco_connector() const;
  [[nodiscard]] bool needs_forretress_connector() const;
  bool bench_pineco_if_useful();
  bool evolve_forretress_ex();

  // Exploding Energy board lookup and printed-effect resolution. BoardIndex,
  // OptionalBoardIndex, pointer-to-index mapping, attachment-destination storage,
  // and mutable board lookup are supplied by board_state_policy.inc.
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
