#pragma once

// Crobat modeling simulation schedule and CSV reporting.
// This file is composed inside namespace sim after crobat_modeling_decks.inc.
// Modeling/reproduction contract: https://github.com/FlareZ123/pokemon-sims/issues/1394
// Atomic result-write contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#windows-build-and-result-write-behavior

constexpr std::size_t crobat_model_seed_slot(const std::size_t scenario_index) {
  // Historical slots 4 and 11 belonged to retired full-turn-one Item-lock rows.
  // Preserve the established random stream for every surviving scenario:
  // https://github.com/FlareZ123/pokemon-sims/issues/1118
  // https://github.com/FlareZ123/pokemon-sims/issues/2247
  return scenario_index + (scenario_index >= 4 ? 1U : 0U) +
         (scenario_index >= 10 ? 1U : 0U);
}

void write_crobat_modeling_matrix(const std::filesystem::path& output,
                                  const std::uint64_t trials,
                                  const std::uint64_t seed) {
  std::ostringstream csv;
  csv << "variant,cuts,crobat_copies,scenario,requested_trials,valid_trials,invalid_trials,ready_by_t2_pct,"
         "ready_by_t3_pct,ready_by_t4_pct,dark_asset_use_pct,avg_dark_asset_cards_drawn_per_game_using\n";
  const auto scenarios = all_scenarios();
  for (std::size_t scenario_index = 0; scenario_index < scenarios.size(); ++scenario_index) {
    const Scenario& scenario = scenarios[scenario_index];
    const std::size_t seed_slot = crobat_model_seed_slot(scenario_index);
    const std::uint64_t common_seed = seed + 104729ULL * seed_slot;
    for (const CrobatModelingDeck& deck : crobat_modeling_decks()) {
      std::uint64_t valid_trials = 0;
      std::uint64_t invalid_trials = 0;
      std::uint64_t by2 = 0;
      std::uint64_t by3 = 0;
      std::uint64_t by4 = 0;
      std::uint64_t dark_asset = 0;
      std::uint64_t dark_asset_cards_drawn = 0;

      // Temporary validation mode: isolate each natural game so an existing
      // assertion in one generated route cannot terminate the entire connector
      // matrix. Every arm receives the identical scenario-specific seed sequence;
      // assertioning games are reported explicitly and excluded from percentages,
      // never converted into setup failures. Gameplay is unchanged: this calls the
      // ordinary simulator one game at a time and catches only logic assertions.
      // Modeling contract: https://github.com/FlareZ123/pokemon-sims/issues/1394
      for (std::uint64_t trial = 0; trial < trials; ++trial) {
        try {
          const Aggregate one = simulate(
              scenario, deck.recipe, 1,
              common_seed + trial);
          ++valid_trials;
          by2 += one.by2;
          by3 += one.by3;
          by4 += one.by4;
          dark_asset += one.dark_asset;
          dark_asset_cards_drawn += one.dark_asset_cards_drawn;
        } catch (const std::logic_error&) {
          ++invalid_trials;
        }
      }

      const double average_draw = dark_asset == 0 ? 0.0 :
          static_cast<double>(dark_asset_cards_drawn) /
          static_cast<double>(dark_asset);
      csv << deck.id << ',' << '"' << deck.cuts << '"' << ','
          << crobat_copy_count(deck.recipe) << ','
          << scenario.label << ',' << trials << ',' << valid_trials << ','
          << invalid_trials << ',' << std::fixed << std::setprecision(6)
          << pct(by2, valid_trials) << ',' << pct(by3, valid_trials) << ','
          << pct(by4, valid_trials) << ',' << pct(dark_asset, valid_trials) << ','
          << average_draw << '\n';
    }
  }
  write_atomic(output, csv.str());
}

constexpr std::string_view crobat_model_default_output() {
  return "results/crobat_variant_model.csv";  // Reproduction contract: https://github.com/FlareZ123/pokemon-sims/issues/1394
}

constexpr std::uint64_t crobat_model_default_seed() {
  return 20260723ULL;  // Reproduction contract: https://github.com/FlareZ123/pokemon-sims/issues/1394
}
