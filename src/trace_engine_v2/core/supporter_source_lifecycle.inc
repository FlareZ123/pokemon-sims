  bool begin_supporter_resolution(const Card source) {
    // B-03 step 2 plays and reveals the Supporter before its effect resolves.
    // Keep that source outside ordinary hand and discard until step 4.
    // Supporter procedure B-03: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed lifecycle defect: https://github.com/FlareZ123/pokemon-sims/issues/4348
    if (!ResolvingTrainerSourcePolicy::move_from_hand(
            state_.hand, resolving_trainer_sources_, source)) {
      return false;
    }
    state_.supporter_used = true;
    return true;
  }

  bool finish_supporter_resolution(const Card source) {
    // B-03 step 4 discards the Supporter only after its printed effect finishes.
    // Supporter procedure B-03: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed lifecycle defect: https://github.com/FlareZ123/pokemon-sims/issues/4348
    return ResolvingTrainerSourcePolicy::move_to_discard(
        resolving_trainer_sources_, state_.discard, source);
  }
