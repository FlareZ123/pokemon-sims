#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

// Serena exact text: https://api.pokemontcg.io/v2/cards/swsh12-164
// Current deck slot: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_000.inc#L81-L94
// DCI and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
// Hidden-information boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy

struct Counters {
  std::uint64_t trials{0};
  std::uint64_t ready_by_2{0};
  std::uint64_t ready_by_3{0};
  std::uint64_t setup_failures{0};
  std::uint64_t opening_serena{0};
  std::uint64_t prized_serena{0};
  std::uint64_t ever_held_serena{0};
  std::uint64_t played_serena{0};
  std::uint64_t payload_discard_serena{0};
  std::uint64_t nonpayload_discard_serena{0};
  std::uint64_t zero_draw_serena{0};
  std::uint64_t one_draw_serena{0};
  std::uint64_t two_draw_serena{0};
  std::uint64_t three_plus_draw_serena{0};
  std::uint64_t ready_with_unplayed_serena{0};
  std::uint64_t failure_with_unplayed_serena{0};
  std::uint64_t searched_serena{0};
  std::uint64_t total_hand_before{0};
  std::uint64_t total_discards{0};
  std::uint64_t total_draws{0};
};

bool has_text(const std::string& line, const std::string_view text) {
  return line.find(text) != std::string::npos;
}

bool is_payload_name(const std::string& line) {
  return has_text(line, "Dragapult ex") || has_text(line, "Mega Dragonite ex") ||
         has_text(line, "Dialga-GX") || has_text(line, "Hisuian Goodra VSTAR");
}

std::optional<std::uint64_t> parse_unsigned_after(const std::string& line,
                                                  const std::string_view key) {
  const std::size_t start = line.find(key);
  if (start == std::string::npos) return std::nullopt;
  const std::size_t value_start = start + key.size();
  std::size_t value_end = value_start;
  while (value_end < line.size() && line[value_end] >= '0' && line[value_end] <= '9') {
    ++value_end;
  }
  if (value_end == value_start) return std::nullopt;
  std::uint64_t value = 0;
  const char* first = line.data() + value_start;
  const char* last = line.data() + value_end;
  const auto parsed = std::from_chars(first, last, value);
  if (parsed.ec != std::errc{} || parsed.ptr != last) return std::nullopt;
  return value;
}

double percentage(const std::uint64_t value, const std::uint64_t total) {
  if (total == 0) return 0.0;
  return 100.0 * static_cast<double>(value) / static_cast<double>(total);
}

void write_trace_sample(const std::string& path, const std::uint64_t seed,
                        const sim::TraceLog& trace) {
  std::ofstream out(path, std::ios::app);
  out << "\n===== seed " << seed << " =====\n";
  for (const std::string& line : trace.lines) out << line << '\n';
}

}  // namespace

int main(int argc, char** argv) {
  const std::uint64_t trials_per_scenario = argc > 1
      ? static_cast<std::uint64_t>(std::stoull(argv[1]))
      : 25000ULL;
  const std::uint64_t base_seed = argc > 2
      ? static_cast<std::uint64_t>(std::stoull(argv[2]))
      : 20260721ULL;

  const std::vector<sim::Scenario> scenarios = sim::all_scenarios();
  std::ofstream csv("serena-slot-context.csv");
  csv << "scenario,trials,ready_by_t2_pct,ready_by_t3_pct,setup_failure_pct,"
         "opening_serena_pct,prized_serena_pct,ever_held_serena_pct,played_serena_pct,"
         "payload_discard_given_play_pct,nonpayload_discard_given_play_pct,"
         "zero_draw_given_play_pct,one_draw_given_play_pct,two_draw_given_play_pct,"
         "three_plus_draw_given_play_pct,ready_with_unplayed_serena_pct,"
         "failure_with_unplayed_serena_pct,searched_serena_pct,"
         "mean_hand_before_play,mean_discards_per_play,mean_draws_per_play\n";

  bool metrics_observed = false;
  std::size_t played_samples = 0;
  std::size_t held_unused_samples = 0;

  for (std::size_t scenario_index = 0; scenario_index < scenarios.size(); ++scenario_index) {
    const sim::Scenario& scenario = scenarios[scenario_index];
    Counters counters;
    for (std::uint64_t trial = 0; trial < trials_per_scenario; ++trial) {
      const std::uint64_t seed = base_seed + 104729ULL * scenario_index + trial;
      std::mt19937_64 rng(seed);
      sim::TraceLog trace{true, {}};
      sim::Engine engine(scenario, sim::baseline_recipe(), rng, &trace);
      const sim::TrialOutcome outcome = engine.run();

      ++counters.trials;
      counters.ready_by_2 += static_cast<std::uint64_t>(outcome.ready_by_2);
      counters.ready_by_3 += static_cast<std::uint64_t>(outcome.ready_by_3);
      counters.setup_failures += static_cast<std::uint64_t>(outcome.setup_failed);

      bool opening_serena = false;
      bool prized_serena = false;
      bool ever_held_serena = false;
      bool played_serena = false;
      bool payload_discard = false;
      bool nonpayload_discard = false;
      bool searched_serena = false;
      std::optional<std::uint64_t> hand_before;
      std::optional<std::uint64_t> discards;
      std::optional<std::uint64_t> draws;

      for (const std::string& line : trace.lines) {
        if (has_text(line, "Opening player-known state:") && has_text(line, "Hand=") &&
            has_text(line, "Serena")) {
          opening_serena = true;
        }
        if (has_text(line, "DEBUG ONLY: prizes=") && has_text(line, "Serena")) {
          prized_serena = true;
        }
        if (has_text(line, "Hand=") && has_text(line, "Serena")) {
          ever_held_serena = true;
        }
        if ((has_text(line, "WONDER TAG") || has_text(line, "STAR ALCHEMY") ||
             has_text(line, "Steven") || has_text(line, "GLADION")) &&
            has_text(line, "Serena")) {
          searched_serena = true;
        }
        if ((has_text(line, "Serena chosen discard") ||
             has_text(line, "Serena optional discard"))) {
          if (is_payload_name(line)) payload_discard = true;
          else nonpayload_discard = true;
        }
        if (has_text(line, "Used Serena draw mode;") && has_text(line, "hand_before=")) {
          played_serena = true;
          hand_before = parse_unsigned_after(line, "hand_before=");
          discards = parse_unsigned_after(line, "discards=");
          draws = parse_unsigned_after(line, "draws=");
          if (!hand_before || !discards || !draws) {
            std::cerr << "Malformed Serena metrics line: " << line << '\n';
            return 2;
          }
          metrics_observed = true;
        }
      }

      counters.opening_serena += static_cast<std::uint64_t>(opening_serena);
      counters.prized_serena += static_cast<std::uint64_t>(prized_serena);
      counters.ever_held_serena += static_cast<std::uint64_t>(ever_held_serena);
      counters.played_serena += static_cast<std::uint64_t>(played_serena);
      counters.payload_discard_serena += static_cast<std::uint64_t>(payload_discard);
      counters.nonpayload_discard_serena += static_cast<std::uint64_t>(nonpayload_discard);
      counters.searched_serena += static_cast<std::uint64_t>(searched_serena);

      if (played_serena) {
        counters.total_hand_before += *hand_before;
        counters.total_discards += *discards;
        counters.total_draws += *draws;
        if (*draws == 0) ++counters.zero_draw_serena;
        else if (*draws == 1) ++counters.one_draw_serena;
        else if (*draws == 2) ++counters.two_draw_serena;
        else ++counters.three_plus_draw_serena;
        if (played_samples < 8) {
          write_trace_sample("serena-played-samples.txt", seed, trace);
          ++played_samples;
        }
      }

      const bool ready = outcome.first_ready_turn > 0;
      const bool unplayed_serena = ever_held_serena && !played_serena;
      if (ready && unplayed_serena) ++counters.ready_with_unplayed_serena;
      if (outcome.setup_failed && unplayed_serena) ++counters.failure_with_unplayed_serena;
      if (unplayed_serena && held_unused_samples < 8) {
        write_trace_sample("serena-held-unused-samples.txt", seed, trace);
        ++held_unused_samples;
      }
    }

    const double played_denominator = static_cast<double>(counters.played_serena);
    csv << '"' << scenario.label << "\"," << counters.trials << ','
        << percentage(counters.ready_by_2, counters.trials) << ','
        << percentage(counters.ready_by_3, counters.trials) << ','
        << percentage(counters.setup_failures, counters.trials) << ','
        << percentage(counters.opening_serena, counters.trials) << ','
        << percentage(counters.prized_serena, counters.trials) << ','
        << percentage(counters.ever_held_serena, counters.trials) << ','
        << percentage(counters.played_serena, counters.trials) << ','
        << percentage(counters.payload_discard_serena, counters.played_serena) << ','
        << percentage(counters.nonpayload_discard_serena, counters.played_serena) << ','
        << percentage(counters.zero_draw_serena, counters.played_serena) << ','
        << percentage(counters.one_draw_serena, counters.played_serena) << ','
        << percentage(counters.two_draw_serena, counters.played_serena) << ','
        << percentage(counters.three_plus_draw_serena, counters.played_serena) << ','
        << percentage(counters.ready_with_unplayed_serena, counters.trials) << ','
        << percentage(counters.failure_with_unplayed_serena, counters.trials) << ','
        << percentage(counters.searched_serena, counters.trials) << ','
        << (played_denominator == 0.0 ? 0.0 : counters.total_hand_before / played_denominator) << ','
        << (played_denominator == 0.0 ? 0.0 : counters.total_discards / played_denominator) << ','
        << (played_denominator == 0.0 ? 0.0 : counters.total_draws / played_denominator) << '\n';
  }

  if (!metrics_observed) {
    std::cerr << "No enhanced Serena metrics were observed; the audit patch did not apply.\n";
    return 3;
  }
  return 0;
}
