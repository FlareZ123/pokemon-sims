#define main bottleneck_screen_main
#include "regidrago_bottleneck_analysis_v2.cpp"
#undef main

int main(int argc, char** argv) {
  const std::uint64_t trials = argc >= 2 ? std::stoull(argv[1]) : 50000ULL;
  const std::filesystem::path output_dir = argc >= 3
      ? std::filesystem::path(argv[2])
      : std::filesystem::path("results/bottleneck-confirm");
  if (trials == 0) {
    std::cerr << "trials must be positive\n";
    return 2;
  }

  // These are the Pareto-screen survivors from the broad paired sweep: each
  // improved T3/T4 readiness across strict and matchup-flex while keeping the
  // measured Regidrago, VSTAR, Active, Energy, payload, and K1 checkpoint axes
  // within 0.3 percentage points of baseline or better. Confirm them under every
  // registered aggregate scenario before recommending a node swap.
  // Repository decision priority requires preserving discrete setup resources:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  std::vector<Variant> deck_variants{
      {"baseline", "none", "none", sim::baseline_recipe()},
      make_swap(sim::Card::ErikasInvitation, sim::Card::Arven),
      make_swap(sim::Card::Powerglass, sim::Card::Arven),
      make_swap(sim::Card::ErikasInvitation, sim::Card::QuickBall),
      make_swap(sim::Card::TeamYellsCheer, sim::Card::UltraBall),
      make_swap(sim::Card::Powerglass, sim::Card::UltraBall),
      make_swap(sim::Card::Powerglass, sim::Card::QuickBall),
      make_swap(sim::Card::ProfessorTuro, sim::Card::Arven),
      make_swap(sim::Card::Channeler, sim::Card::Arven),
  };

  constexpr std::uint64_t base_seed = 20260809ULL;
  constexpr std::uint64_t trial_stride = 104729ULL;
  constexpr std::uint64_t scenario_stride = 1000000007ULL;

  std::ostringstream checkpoints;
  checkpoints << "variant,cut,add,scenario,turn,attempted_trials,completed_trials,"
                 "engine_error_pct,ready_pct,regi_in_play_pct,vstar_in_play_pct,"
                 "active_vstar_pct,energy_ready_any_regi_pct,active_energy_ready_pct,"
                 "payload_ready_pct,k1_pct,primary_no_regi_pct,primary_no_vstar_pct,"
                 "primary_inactive_vstar_pct,primary_energy_pct,primary_payload_pct,"
                 "primary_other_pct\n";
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

      const std::uint64_t completed_trials = stats[0].trials;
      const std::uint64_t engine_error_count = trials - completed_trials;
      for (const auto& [message, count] : scenario_errors) {
        errors << quoted(variant.id) << ',' << quoted(variant.cut) << ','
               << quoted(variant.add) << ',' << quoted(scenario.label) << ','
               << quoted(message) << ',' << count << '\n';
      }

      for (std::size_t checkpoint = 0; checkpoint < stats.size(); ++checkpoint) {
        const Stats& s = stats[checkpoint];
        const int turn = static_cast<int>(checkpoint) + 2;
        checkpoints << quoted(variant.id) << ',' << quoted(variant.cut) << ','
                    << quoted(variant.add) << ',' << quoted(scenario.label) << ','
                    << turn << ',' << trials << ',' << s.trials << ','
                    << pct(engine_error_count, trials) << ','
                    << pct(s.ready, s.trials) << ','
                    << pct(s.regi_in_play, s.trials) << ','
                    << pct(s.vstar_in_play, s.trials) << ','
                    << pct(s.active_vstar, s.trials) << ','
                    << pct(s.energy_ready_any_regi, s.trials) << ','
                    << pct(s.active_energy_ready, s.trials) << ','
                    << pct(s.payload_ready, s.trials) << ','
                    << pct(s.k1, s.trials) << ','
                    << pct(s.primary_no_regi, s.trials) << ','
                    << pct(s.primary_no_vstar, s.trials) << ','
                    << pct(s.primary_inactive_vstar, s.trials) << ','
                    << pct(s.primary_energy, s.trials) << ','
                    << pct(s.primary_payload, s.trials) << ','
                    << pct(s.primary_other, s.trials) << '\n';
      }
    }
    std::cout << "confirmed " << (variant_index + 1U) << '/'
              << deck_variants.size() << " variants" << std::endl;
  }

  std::filesystem::create_directories(output_dir);
  sim::write_atomic(output_dir / "checkpoint_summary.csv", checkpoints.str());
  sim::write_atomic(output_dir / "engine_errors.csv", errors.str());
  std::cout << "wrote confirmation sweep: " << deck_variants.size()
            << " variants x " << scenarios.size() << " scenarios x "
            << trials << " paired trials" << std::endl;
  return 0;
}
