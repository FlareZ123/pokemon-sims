#define main connector_screen_main
#include "regidrago_connector_swap_screen.cpp"
#undef main

Variant make_double_swap(const sim::Card cut_a, const sim::Card add_a,
                         const sim::Card cut_b, const sim::Card add_b,
                         const std::string& id) {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  adjust(recipe, cut_a, -1);
  adjust(recipe, add_a, 1);
  adjust(recipe, cut_b, -1);
  adjust(recipe, add_b, 1);
  Variant variant{id, std::string(sim::name(cut_a)) + " + " + std::string(sim::name(cut_b)),
                  std::string(sim::name(add_a)) + " + " + std::string(sim::name(add_b)),
                  std::move(recipe)};
  std::string error;
  if (!sim::validate_recipe({variant.id, variant.recipe}, &error)) {
    throw std::logic_error(error);
  }
  return variant;
}

int main(int argc, char** argv) {
  const std::uint64_t trials = argc >= 2 ? std::stoull(argv[1]) : 50000ULL;
  const std::filesystem::path output_dir = argc >= 3
      ? std::filesystem::path(argv[2])
      : std::filesystem::path("results/balanced-connector-packages");
  if (trials == 0) return 2;

  // The paired packages preserve all four Mysterious Treasure, Professor Burnet,
  // Oricorio, and Brilliant Blender copies. One Basic-only/Prize connector is
  // converted into a VSTAR-capable node, while one Quick Ball becomes Battle VIP
  // Pass to front-load up to two Basics and K1 on turn one.
  // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
  // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Evolution Incense: https://api.pokemontcg.io/v2/cards/swsh1-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  std::vector<Variant> deck_variants{
      {"baseline", "none", "none", sim::baseline_recipe()},
      make_double_swap(sim::Card::HisuianHeavyBall, sim::Card::UltraBall,
                       sim::Card::QuickBall, sim::Card::BattleVipPass,
                       "Heavy Ball -> Ultra Ball; Quick Ball -> Battle VIP Pass"),
      make_double_swap(sim::Card::HisuianHeavyBall, sim::Card::EvolutionIncense,
                       sim::Card::QuickBall, sim::Card::BattleVipPass,
                       "Heavy Ball -> Evolution Incense; Quick Ball -> Battle VIP Pass"),
      make_double_swap(sim::Card::HisuianHeavyBall, sim::Card::RegidragoVstar,
                       sim::Card::QuickBall, sim::Card::BattleVipPass,
                       "Heavy Ball -> Regidrago VSTAR; Quick Ball -> Battle VIP Pass"),
      make_double_swap(sim::Card::HisuianHeavyBall, sim::Card::UltraBall,
                       sim::Card::QuickBall, sim::Card::EvolutionIncense,
                       "Heavy Ball -> Ultra Ball; Quick Ball -> Evolution Incense"),
  };

  constexpr std::uint64_t base_seed = 202608090300ULL;
  constexpr std::uint64_t trial_stride = 104729ULL;
  constexpr std::uint64_t scenario_stride = 1000000007ULL;
  std::ostringstream csv;
  csv << "variant,cut,add,scenario,turn,attempted_trials,completed_trials,engine_error_pct,"
         "ready_pct,regi_in_play_pct,vstar_in_play_pct,active_vstar_pct,"
         "energy_ready_any_regi_pct,active_energy_ready_pct,payload_ready_pct,k1_pct,"
         "primary_no_regi_pct,primary_no_vstar_pct,primary_inactive_vstar_pct,"
         "primary_energy_pct,primary_payload_pct,primary_other_pct\n";
  std::ostringstream errors;
  errors << "variant,cut,add,scenario,error,count\n";

  const std::vector<sim::Scenario> scenarios = sim::all_scenarios();
  for (std::size_t variant_index = 0; variant_index < deck_variants.size(); ++variant_index) {
    const Variant& variant = deck_variants[variant_index];
    for (std::size_t scenario_index = 0; scenario_index < scenarios.size(); ++scenario_index) {
      sim::Scenario scenario = scenarios[scenario_index];
      scenario.max_turn = 4;
      std::array<Stats, 3> stats{};
      std::map<std::string, std::uint64_t> scenario_errors;
      for (std::uint64_t trial = 0; trial < trials; ++trial) {
        const std::uint64_t trial_seed = base_seed +
            scenario_stride * static_cast<std::uint64_t>(scenario_index) +
            trial_stride * trial;
        try {
          std::mt19937_64 rng(trial_seed);
          sim::Engine engine(scenario, variant.recipe, rng);
          const auto snapshots = sim::EngineTestAccess::run_checkpoints(engine);
          for (std::size_t checkpoint = 0; checkpoint < snapshots.size(); ++checkpoint) {
            record(stats[checkpoint], snapshots[checkpoint]);
          }
        } catch (const std::exception& error) {
          ++scenario_errors[error.what()];
        }
      }
      const std::uint64_t completed = stats[0].trials;
      const std::uint64_t error_count = trials - completed;
      for (const auto& [message, count] : scenario_errors) {
        errors << quoted(variant.id) << ',' << quoted(variant.cut) << ','
               << quoted(variant.add) << ',' << quoted(scenario.label) << ','
               << quoted(message) << ',' << count << '\n';
      }
      for (std::size_t checkpoint = 0; checkpoint < stats.size(); ++checkpoint) {
        const Stats& s = stats[checkpoint];
        csv << quoted(variant.id) << ',' << quoted(variant.cut) << ','
            << quoted(variant.add) << ',' << quoted(scenario.label) << ','
            << (checkpoint + 2U) << ',' << trials << ',' << s.trials << ','
            << pct(error_count, trials) << ',' << pct(s.ready, s.trials) << ','
            << pct(s.regi_in_play, s.trials) << ',' << pct(s.vstar_in_play, s.trials) << ','
            << pct(s.active_vstar, s.trials) << ',' << pct(s.energy_ready_any_regi, s.trials) << ','
            << pct(s.active_energy_ready, s.trials) << ',' << pct(s.payload_ready, s.trials) << ','
            << pct(s.k1, s.trials) << ',' << pct(s.primary_no_regi, s.trials) << ','
            << pct(s.primary_no_vstar, s.trials) << ',' << pct(s.primary_inactive_vstar, s.trials) << ','
            << pct(s.primary_energy, s.trials) << ',' << pct(s.primary_payload, s.trials) << ','
            << pct(s.primary_other, s.trials) << '\n';
      }
    }
    std::cout << "confirmed " << (variant_index + 1U) << '/'
              << deck_variants.size() << " packages" << std::endl;
  }
  std::filesystem::create_directories(output_dir);
  sim::write_atomic(output_dir / "checkpoint_summary.csv", csv.str());
  sim::write_atomic(output_dir / "engine_errors.csv", errors.str());
  return 0;
}
