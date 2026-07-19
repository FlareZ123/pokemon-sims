#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr std::uint64_t kScenarioStride = 104729ULL;
constexpr std::uint64_t kPairedTrialStride = 1000003ULL;
constexpr int kTraceExamplesPerDirection = 2;

std::uint64_t parse_positive(const char* text, const char* label) {
  const std::uint64_t value = std::stoull(text);
  if (value == 0) throw std::runtime_error(std::string(label) + " must be positive");
  return value;
}

sim::DeckRecipe communication_variant(const sim::DeckRecipe& baseline) {
  sim::DeckRecipe variant = baseline;
  const auto quick_ball = std::find_if(
      variant.begin(), variant.end(), [](const auto& entry) {
        return entry.first == sim::Card::QuickBall;
      });
  if (quick_ball == variant.end() || quick_ball->second < 1) {
    throw std::runtime_error("baseline Quick Ball count is unavailable");
  }

  // Replace the final physical Quick Ball slot with Pokémon Communication. Inserting
  // immediately after the reduced Quick Ball entry keeps the expanded 60-card vector
  // identical at every other position before the shared RNG shuffles it.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  --quick_ball->second;
  variant.insert(std::next(quick_ball), {sim::Card::PokemonCommunication, 1});
  return variant;
}

std::vector<sim::Card> expand(const sim::DeckRecipe& recipe) {
  std::vector<sim::Card> cards;
  for (const auto& [card, copies] : recipe) {
    cards.insert(cards.end(), static_cast<std::size_t>(copies), card);
  }
  return cards;
}

void verify_exact_swap(const sim::DeckRecipe& baseline,
                       const sim::DeckRecipe& variant) {
  const std::vector<sim::Card> before = expand(baseline);
  const std::vector<sim::Card> after = expand(variant);
  if (before.size() != 60 || after.size() != 60) {
    throw std::runtime_error("both recipes must contain exactly 60 cards");
  }

  std::size_t differences = 0;
  for (std::size_t index = 0; index < before.size(); ++index) {
    if (before[index] == after[index]) continue;
    ++differences;
    if (before[index] != sim::Card::QuickBall ||
        after[index] != sim::Card::PokemonCommunication) {
      throw std::runtime_error("variant changed a card other than Quick Ball");
    }
  }
  if (differences != 1) {
    throw std::runtime_error("variant must replace exactly one physical card slot");
  }
}

std::string aggregate_header() {
  return "scenario,trials,ready_by_t2_pct,ready_by_t2_se_pp,ready_by_t3_pct,ready_by_t3_se_pp,ready_by_t4_pct,ready_by_t4_se_pp,ready_by_t5_pct,ready_by_t5_se_pp,ready_on_t5_pct,ready_on_t5_se_pp,setup_failure_pct,setup_failure_se_pp,started_regi_pct,fss_used_pct,legacy_star_used_pct,steven_used_pct,blender_used_pct,mean_mulligans\n";
}

void append_aggregate_row(std::ostringstream& csv,
                          const sim::Scenario& scenario,
                          const sim::Aggregate& aggregate) {
  csv << '"' << scenario.label << "\"," << aggregate.trials << ','
      << sim::pct(aggregate.by2, aggregate.trials) << ','
      << sim::standard_error_pp(aggregate.by2, aggregate.trials) << ','
      << sim::pct(aggregate.by3, aggregate.trials) << ','
      << sim::standard_error_pp(aggregate.by3, aggregate.trials) << ','
      << sim::pct(aggregate.by4, aggregate.trials) << ','
      << sim::standard_error_pp(aggregate.by4, aggregate.trials) << ','
      << sim::pct(aggregate.by5, aggregate.trials) << ','
      << sim::standard_error_pp(aggregate.by5, aggregate.trials) << ','
      << sim::pct(aggregate.ready_on5, aggregate.trials) << ','
      << sim::standard_error_pp(aggregate.ready_on5, aggregate.trials) << ','
      << sim::pct(aggregate.setup_failures, aggregate.trials) << ','
      << sim::standard_error_pp(aggregate.setup_failures, aggregate.trials) << ','
      << sim::pct(aggregate.started_regi, aggregate.trials) << ','
      << sim::pct(aggregate.fss, aggregate.trials) << ','
      << sim::pct(aggregate.legacy, aggregate.trials) << ','
      << sim::pct(aggregate.steven, aggregate.trials) << ','
      << sim::pct(aggregate.blender, aggregate.trials) << ','
      << aggregate.total_mulligans / static_cast<double>(aggregate.trials) << '\n';
}

bool earlier(const sim::TrialOutcome& left, const sim::TrialOutcome& right) {
  if (left.first_ready_turn == 0) return false;
  return right.first_ready_turn == 0 ||
         left.first_ready_turn < right.first_ready_turn;
}

struct PairedCounts {
  std::uint64_t trials{};
  std::uint64_t variant_faster{};
  std::uint64_t baseline_faster{};
  std::uint64_t same_turn{};
  std::uint64_t setup_win{};
  std::uint64_t setup_loss{};
  std::array<std::uint64_t, 3> threshold_gain{};
  std::array<std::uint64_t, 3> threshold_loss{};
  std::array<std::array<std::uint64_t, 6>, 6> transitions{};
};

bool ready_at(const sim::TrialOutcome& outcome, const int threshold) {
  if (threshold == 2) return outcome.ready_by_2;
  if (threshold == 3) return outcome.ready_by_3;
  return outcome.ready_by_4;
}

void update_paired(PairedCounts& counts,
                   const sim::TrialOutcome& baseline,
                   const sim::TrialOutcome& variant) {
  ++counts.trials;
  if (earlier(variant, baseline)) {
    ++counts.variant_faster;
  } else if (earlier(baseline, variant)) {
    ++counts.baseline_faster;
  } else {
    ++counts.same_turn;
  }

  if (baseline.setup_failed && !variant.setup_failed) ++counts.setup_win;
  if (!baseline.setup_failed && variant.setup_failed) ++counts.setup_loss;

  constexpr std::array<int, 3> thresholds{2, 3, 4};
  for (std::size_t index = 0; index < thresholds.size(); ++index) {
    const bool baseline_ready = ready_at(baseline, thresholds[index]);
    const bool variant_ready = ready_at(variant, thresholds[index]);
    if (!baseline_ready && variant_ready) ++counts.threshold_gain[index];
    if (baseline_ready && !variant_ready) ++counts.threshold_loss[index];
  }

  const int baseline_turn = std::clamp(baseline.first_ready_turn, 0, 5);
  const int variant_turn = std::clamp(variant.first_ready_turn, 0, 5);
  ++counts.transitions[static_cast<std::size_t>(baseline_turn)]
                       [static_cast<std::size_t>(variant_turn)];
}

double paired_delta_pp(const std::uint64_t gain,
                       const std::uint64_t loss,
                       const std::uint64_t trials) {
  return 100.0 * (static_cast<double>(gain) - static_cast<double>(loss)) /
         static_cast<double>(trials);
}

double paired_delta_se_pp(const std::uint64_t gain,
                          const std::uint64_t loss,
                          const std::uint64_t trials) {
  const double n = static_cast<double>(trials);
  const double mean = (static_cast<double>(gain) - static_cast<double>(loss)) / n;
  const double second_moment =
      (static_cast<double>(gain) + static_cast<double>(loss)) / n;
  const double variance = std::max(0.0, second_moment - mean * mean);
  return 100.0 * std::sqrt(variance / n);
}

struct TraceResult {
  sim::TrialOutcome outcome;
  std::vector<std::string> lines;
};

TraceResult run_trace(const sim::Scenario& scenario,
                      const sim::DeckRecipe& recipe,
                      const std::uint64_t seed) {
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);
  return {engine.run(), std::move(trace.lines)};
}

void append_trace_pair(std::ostringstream& output,
                       const sim::Scenario& scenario,
                       const std::uint64_t seed,
                       const sim::DeckRecipe& baseline,
                       const sim::DeckRecipe& variant,
                       const std::string_view direction) {
  const TraceResult baseline_trace = run_trace(scenario, baseline, seed);
  const TraceResult variant_trace = run_trace(scenario, variant, seed);
  output << "\n============================================================\n"
         << scenario.label << " | seed " << seed << " | " << direction << '\n'
         << "Baseline first ready: " << baseline_trace.outcome.first_ready_turn
         << " | Variant first ready: " << variant_trace.outcome.first_ready_turn << '\n'
         << "---------------- BASELINE: 3 Quick Ball ----------------\n";
  for (const std::string& line : baseline_trace.lines) output << line << '\n';
  output << "---------------- VARIANT: 2 Quick Ball + 1 Pokémon Communication ----------------\n";
  for (const std::string& line : variant_trace.lines) output << line << '\n';
}

std::string turn_label(const int turn) {
  return turn == 0 ? "not-ready" : "T" + std::to_string(turn);
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 5) {
    std::cerr << "usage: quick_ball_to_communication AGGREGATE_TRIALS PAIRED_TRIALS SEED OUT_DIR\n";
    return 2;
  }

  try {
    const std::uint64_t aggregate_trials = parse_positive(argv[1], "aggregate trials");
    const std::uint64_t paired_trials = parse_positive(argv[2], "paired trials");
    const std::uint64_t base_seed = std::stoull(argv[3]);
    const std::filesystem::path output_dir = argv[4];
    std::filesystem::create_directories(output_dir);

    const sim::DeckRecipe baseline = sim::baseline_recipe();
    const sim::DeckRecipe variant = communication_variant(baseline);
    verify_exact_swap(baseline, variant);

    const std::vector<sim::Scenario> scenarios = sim::all_scenarios();
    std::ostringstream baseline_csv;
    std::ostringstream variant_csv;
    std::ostringstream aggregate_delta_csv;
    std::ostringstream paired_csv;
    std::ostringstream transition_csv;
    std::ostringstream examples;
    std::ostringstream summary;

    baseline_csv << aggregate_header();
    variant_csv << aggregate_header();
    aggregate_delta_csv
        << "scenario,trials,t2_delta_pp,t3_delta_pp,t4_delta_pp,t5_delta_pp,setup_failure_delta_pp,started_regi_delta_pp,fss_delta_pp,legacy_star_delta_pp,steven_delta_pp,blender_delta_pp,mulligan_delta\n";
    paired_csv
        << "scenario,trials,variant_faster_pct,baseline_faster_pct,same_first_ready_pct,setup_win_pct,setup_loss_pct,t2_gain_pct,t2_loss_pct,t2_delta_pp,t2_delta_se_pp,t3_gain_pct,t3_loss_pct,t3_delta_pp,t3_delta_se_pp,t4_gain_pct,t4_loss_pct,t4_delta_pp,t4_delta_se_pp\n";
    transition_csv << "scenario,baseline_first_ready,variant_first_ready,count,pct\n";

    summary << "# One Quick Ball to one Pokémon Communication\n\n"
            << "Aggregate trials per scenario: " << aggregate_trials << "  \n"
            << "Paired trials per scenario: " << paired_trials << "  \n"
            << "Seed: " << base_seed << "\n\n"
            << "The expanded recipes differ at exactly one physical slot: Quick Ball becomes Pokémon Communication. "
            << "Both engines receive the same RNG seed in every paired trial, preserving the opening permutation until a policy path consumes randomness differently.\n\n"
            << "| Scenario | Aggregate ΔT2 | Aggregate ΔT3 | Aggregate ΔT4 | Paired faster | Paired slower | Setup wins | Setup losses |\n"
            << "|---|---:|---:|---:|---:|---:|---:|---:|\n";

    for (std::size_t scenario_index = 0; scenario_index < scenarios.size(); ++scenario_index) {
      const sim::Scenario& scenario = scenarios[scenario_index];
      const std::uint64_t scenario_seed =
          base_seed + kScenarioStride * static_cast<std::uint64_t>(scenario_index);
      const sim::Aggregate baseline_aggregate =
          sim::simulate(scenario, baseline, aggregate_trials, scenario_seed);
      const sim::Aggregate variant_aggregate =
          sim::simulate(scenario, variant, aggregate_trials, scenario_seed);
      append_aggregate_row(baseline_csv, scenario, baseline_aggregate);
      append_aggregate_row(variant_csv, scenario, variant_aggregate);

      const auto delta = [aggregate_trials](const std::uint64_t after,
                                            const std::uint64_t before) {
        return 100.0 * (static_cast<double>(after) - static_cast<double>(before)) /
               static_cast<double>(aggregate_trials);
      };
      const double aggregate_t2_delta = delta(variant_aggregate.by2, baseline_aggregate.by2);
      const double aggregate_t3_delta = delta(variant_aggregate.by3, baseline_aggregate.by3);
      const double aggregate_t4_delta = delta(variant_aggregate.by4, baseline_aggregate.by4);
      aggregate_delta_csv << '"' << scenario.label << "\"," << aggregate_trials << ','
          << aggregate_t2_delta << ',' << aggregate_t3_delta << ','
          << aggregate_t4_delta << ','
          << delta(variant_aggregate.by5, baseline_aggregate.by5) << ','
          << delta(variant_aggregate.setup_failures, baseline_aggregate.setup_failures) << ','
          << delta(variant_aggregate.started_regi, baseline_aggregate.started_regi) << ','
          << delta(variant_aggregate.fss, baseline_aggregate.fss) << ','
          << delta(variant_aggregate.legacy, baseline_aggregate.legacy) << ','
          << delta(variant_aggregate.steven, baseline_aggregate.steven) << ','
          << delta(variant_aggregate.blender, baseline_aggregate.blender) << ','
          << variant_aggregate.total_mulligans / static_cast<double>(aggregate_trials) -
                 baseline_aggregate.total_mulligans / static_cast<double>(aggregate_trials)
          << '\n';

      PairedCounts counts;
      int faster_examples = 0;
      int slower_examples = 0;
      for (std::uint64_t trial = 0; trial < paired_trials; ++trial) {
        const std::uint64_t trial_seed = scenario_seed + kPairedTrialStride * trial;
        std::mt19937_64 baseline_rng(trial_seed);
        std::mt19937_64 variant_rng(trial_seed);
        sim::Engine baseline_engine(scenario, baseline, baseline_rng);
        sim::Engine variant_engine(scenario, variant, variant_rng);
        const sim::TrialOutcome baseline_outcome = baseline_engine.run();
        const sim::TrialOutcome variant_outcome = variant_engine.run();
        update_paired(counts, baseline_outcome, variant_outcome);

        if (earlier(variant_outcome, baseline_outcome) &&
            faster_examples < kTraceExamplesPerDirection) {
          append_trace_pair(examples, scenario, trial_seed, baseline, variant,
                            "variant faster");
          ++faster_examples;
        } else if (earlier(baseline_outcome, variant_outcome) &&
                   slower_examples < kTraceExamplesPerDirection) {
          append_trace_pair(examples, scenario, trial_seed, baseline, variant,
                            "baseline faster");
          ++slower_examples;
        }
      }

      const double variant_faster_pct = sim::pct(counts.variant_faster, counts.trials);
      const double baseline_faster_pct = sim::pct(counts.baseline_faster, counts.trials);
      const double setup_win_pct = sim::pct(counts.setup_win, counts.trials);
      const double setup_loss_pct = sim::pct(counts.setup_loss, counts.trials);
      paired_csv << '"' << scenario.label << "\"," << counts.trials << ','
          << variant_faster_pct << ',' << baseline_faster_pct << ','
          << sim::pct(counts.same_turn, counts.trials) << ','
          << setup_win_pct << ',' << setup_loss_pct;
      for (std::size_t threshold = 0; threshold < 3; ++threshold) {
        paired_csv << ',' << sim::pct(counts.threshold_gain[threshold], counts.trials)
                   << ',' << sim::pct(counts.threshold_loss[threshold], counts.trials)
                   << ',' << paired_delta_pp(counts.threshold_gain[threshold],
                                              counts.threshold_loss[threshold],
                                              counts.trials)
                   << ',' << paired_delta_se_pp(counts.threshold_gain[threshold],
                                                 counts.threshold_loss[threshold],
                                                 counts.trials);
      }
      paired_csv << '\n';

      for (int before = 0; before <= 5; ++before) {
        for (int after = 0; after <= 5; ++after) {
          const std::uint64_t count = counts.transitions[static_cast<std::size_t>(before)]
                                                        [static_cast<std::size_t>(after)];
          if (count == 0) continue;
          transition_csv << '"' << scenario.label << "\"," << turn_label(before)
                         << ',' << turn_label(after) << ',' << count << ','
                         << sim::pct(count, counts.trials) << '\n';
        }
      }

      summary << "| " << scenario.label << " | " << aggregate_t2_delta
              << " pp | " << aggregate_t3_delta << " pp | " << aggregate_t4_delta
              << " pp | " << variant_faster_pct << "% | " << baseline_faster_pct
              << "% | " << setup_win_pct << "% | " << setup_loss_pct << "% |\n";
    }

    sim::write_atomic(output_dir / "baseline.csv", baseline_csv.str());
    sim::write_atomic(output_dir / "variant.csv", variant_csv.str());
    sim::write_atomic(output_dir / "aggregate-deltas.csv", aggregate_delta_csv.str());
    sim::write_atomic(output_dir / "paired-results.csv", paired_csv.str());
    sim::write_atomic(output_dir / "turn-transitions.csv", transition_csv.str());
    sim::write_atomic(output_dir / "changed-trace-examples.txt", examples.str());
    sim::write_atomic(output_dir / "summary.md", summary.str());
    std::cout << summary.str();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
