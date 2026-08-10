#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {

DeckRecipe full_letter_recipe() {
  DeckRecipe recipe = baseline_recipe();
  const auto vessel = std::find_if(
      recipe.begin(), recipe.end(),
      [](const auto& entry) { return entry.first == Card::EarthenVessel; });
  if (vessel == recipe.end() || vessel->second != 2) {
    throw std::logic_error("regidrago-shell no longer has exactly two Earthen Vessel");
  }

  // Professor's Letter: https://api.pokemontcg.io/v2/cards/xy1-123
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Pairing rationale: https://github.com/FlareZ123/pokemon-sims/issues/2599
  vessel->first = Card::ProfessorsLetter;

  NamedDeck validation{"professors-letter-two-letter-temporary", recipe};
  std::string error;
  if (!validate_recipe(validation, &error)) throw std::logic_error(error);
  return recipe;
}

DeckRecipe one_one_recipe() {
  DeckRecipe recipe = baseline_recipe();
  const auto vessel = std::find_if(
      recipe.begin(), recipe.end(),
      [](const auto& entry) { return entry.first == Card::EarthenVessel; });
  if (vessel == recipe.end() || vessel->second != 2) {
    throw std::logic_error("regidrago-shell no longer has exactly two Earthen Vessel");
  }

  // Keep the first physical Vessel slot and replace only the second physical
  // Vessel slot with Professor's Letter. This preserves the other 59 expanded
  // pre-shuffle card positions under common random numbers.
  // Professor's Letter: https://api.pokemontcg.io/v2/cards/xy1-123
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Core shuffle/search rules: https://www.pokemon.com/us/pokemon-tcg/rules
  vessel->second = 1;
  recipe.insert(vessel + 1, {Card::ProfessorsLetter, 1});

  NamedDeck validation{"professors-letter-one-one-temporary", recipe};
  std::string error;
  if (!validate_recipe(validation, &error)) throw std::logic_error(error);
  return recipe;
}

std::vector<Card> expanded_recipe(const DeckRecipe& recipe) {
  std::vector<Card> cards;
  for (const auto& [card, copies] : recipe) {
    for (int copy = 0; copy < copies; ++copy) cards.push_back(card);
  }
  return cards;
}

void verify_pairing(const DeckRecipe& baseline, const DeckRecipe& split,
                    const DeckRecipe& full_letter) {
  const auto b = expanded_recipe(baseline);
  const auto s = expanded_recipe(split);
  const auto f = expanded_recipe(full_letter);
  if (b.size() != 60U || s.size() != 60U || f.size() != 60U) {
    throw std::logic_error("all Professor's Letter comparison decks must contain 60 cards");
  }

  std::size_t split_differences = 0;
  std::size_t full_differences = 0;
  for (std::size_t index = 0; index < b.size(); ++index) {
    if (b[index] != s[index]) {
      ++split_differences;
      if (b[index] != Card::EarthenVessel || s[index] != Card::ProfessorsLetter) {
        throw std::logic_error("1/1 split changed a non-Vessel physical slot");
      }
    }
    if (b[index] != f[index]) {
      ++full_differences;
      if (b[index] != Card::EarthenVessel || f[index] != Card::ProfessorsLetter) {
        throw std::logic_error("2-Letter swap changed a non-Vessel physical slot");
      }
    }
  }
  if (split_differences != 1U || full_differences != 2U) {
    throw std::logic_error("Professor's Letter physical slot replacement count is wrong");
  }
}

TrialOutcome run_game(const Scenario& scenario, const DeckRecipe& recipe,
                      const std::uint64_t seed, TraceLog* trace = nullptr) {
  std::mt19937_64 rng(seed);
  Engine engine(scenario, recipe, rng, trace);
  return engine.run();
}

int parsed_turn(const std::string& line) {
  if (line.size() < 4U || line[0] != 'T') return 0;
  const std::size_t separator = line.find(" |");
  if (separator == std::string::npos || separator <= 1U) return 0;
  try {
    return std::stoi(line.substr(1, separator - 1U));
  } catch (const std::exception&) {
    return 0;
  }
}

bool item_locked_on_turn(const Scenario& scenario, const int turn) {
  return scenario.locks == LockMode::FullItem ||
      ((scenario.locks == LockMode::TurnTwoItem ||
        scenario.locks == LockMode::FullCombined) && turn >= 2);
}

struct OutcomeCounts {
  std::uint64_t games{0};
  std::uint64_t by2{0};
  std::uint64_t by3{0};
  std::uint64_t by4{0};

  void add(const TrialOutcome& outcome) {
    ++games;
    by2 += outcome.ready_by_2 ? 1U : 0U;
    by3 += outcome.ready_by_3 ? 1U : 0U;
    by4 += outcome.ready_by_4 ? 1U : 0U;
  }
};

struct SplitFacts {
  bool played{false};
  std::uint64_t total_plays{0};
  std::uint64_t locked_plays{0};
  int first_play_turn{0};
};

SplitFacts inspect_split_trace(const TraceLog& trace, const Scenario& scenario) {
  SplitFacts facts;
  for (const std::string& line : trace.lines) {
    // Production action spelling established by issue #2599:
    // https://github.com/FlareZ123/pokemon-sims/issues/2599
    if (line.find("| Professor's Letter | rules:") == std::string::npos) continue;
    const int turn = parsed_turn(line);
    facts.played = true;
    ++facts.total_plays;
    if (facts.first_play_turn == 0) facts.first_play_turn = turn;
    if (item_locked_on_turn(scenario, turn)) ++facts.locked_plays;
  }
  return facts;
}

struct ScenarioAggregate {
  OutcomeCounts baseline;
  OutcomeCounts split;
  OutcomeCounts full_letter;
  OutcomeCounts split_played_baseline;
  OutcomeCounts split_played_split;
  OutcomeCounts split_not_played_baseline;
  OutcomeCounts split_not_played_split;
  std::uint64_t split_letter_played_games{0};
  std::uint64_t split_letter_total_plays{0};
  std::array<std::uint64_t, 5> split_first_play_turn{};
  std::uint64_t split_locked_plays{0};
};

double percent(const std::uint64_t count, const std::uint64_t total) {
  return total == 0U ? 0.0 :
      100.0 * static_cast<double>(count) / static_cast<double>(total);
}

ScenarioAggregate analyze_scenario(const Scenario& scenario,
                                   const DeckRecipe& baseline,
                                   const DeckRecipe& split,
                                   const DeckRecipe& full_letter,
                                   const std::uint64_t trials,
                                   const std::uint64_t first_seed) {
  ScenarioAggregate aggregate;
  for (std::uint64_t trial = 0; trial < trials; ++trial) {
    const std::uint64_t seed = first_seed + trial;
    const TrialOutcome baseline_outcome = run_game(scenario, baseline, seed);

    TraceLog split_trace{true, {}};
    const TrialOutcome split_outcome = run_game(scenario, split, seed, &split_trace);
    const SplitFacts facts = inspect_split_trace(split_trace, scenario);

    const TrialOutcome full_letter_outcome = run_game(scenario, full_letter, seed);

    aggregate.baseline.add(baseline_outcome);
    aggregate.split.add(split_outcome);
    aggregate.full_letter.add(full_letter_outcome);
    aggregate.split_letter_total_plays += facts.total_plays;
    aggregate.split_locked_plays += facts.locked_plays;

    if (facts.played) {
      ++aggregate.split_letter_played_games;
      aggregate.split_played_baseline.add(baseline_outcome);
      aggregate.split_played_split.add(split_outcome);
      if (facts.first_play_turn >= 1 && facts.first_play_turn <= 5) {
        ++aggregate.split_first_play_turn[
            static_cast<std::size_t>(facts.first_play_turn - 1)];
      }
    } else {
      aggregate.split_not_played_baseline.add(baseline_outcome);
      aggregate.split_not_played_split.add(split_outcome);
    }
  }
  return aggregate;
}

void write_counts(std::ostream& out, const OutcomeCounts& counts) {
  out << percent(counts.by2, counts.games) << ','
      << percent(counts.by3, counts.games) << ','
      << percent(counts.by4, counts.games);
}

void write_header(std::ostream& out) {
  out << "scenario,trials,"
         "baseline_t2_pct,baseline_t3_pct,baseline_t4_pct,"
         "split_t2_pct,split_t3_pct,split_t4_pct,"
         "split_delta_t2_pp,split_delta_t3_pp,split_delta_t4_pp,"
         "full_letter_t2_pct,full_letter_t3_pct,full_letter_t4_pct,"
         "full_letter_delta_t2_pp,full_letter_delta_t3_pp,full_letter_delta_t4_pp,"
         "split_minus_full_t2_pp,split_minus_full_t3_pp,split_minus_full_t4_pp,"
         "split_letter_played_games,split_letter_played_games_pct,split_letter_total_plays,"
         "split_first_play_t1,split_first_play_t2,split_first_play_t3,"
         "split_first_play_t4,split_first_play_t5,split_locked_plays,"
         "played_baseline_t2_pct,played_baseline_t3_pct,played_baseline_t4_pct,"
         "played_split_t2_pct,played_split_t3_pct,played_split_t4_pct,"
         "not_played_baseline_t2_pct,not_played_baseline_t3_pct,not_played_baseline_t4_pct,"
         "not_played_split_t2_pct,not_played_split_t3_pct,not_played_split_t4_pct\n";
}

void write_row(std::ostream& out, const Scenario& scenario,
               const ScenarioAggregate& a) {
  out << scenario.label << ',' << a.baseline.games << ','
      << std::fixed << std::setprecision(6);
  write_counts(out, a.baseline);
  out << ',';
  write_counts(out, a.split);
  out << ','
      << percent(a.split.by2, a.split.games) - percent(a.baseline.by2, a.baseline.games) << ','
      << percent(a.split.by3, a.split.games) - percent(a.baseline.by3, a.baseline.games) << ','
      << percent(a.split.by4, a.split.games) - percent(a.baseline.by4, a.baseline.games) << ',';
  write_counts(out, a.full_letter);
  out << ','
      << percent(a.full_letter.by2, a.full_letter.games) - percent(a.baseline.by2, a.baseline.games) << ','
      << percent(a.full_letter.by3, a.full_letter.games) - percent(a.baseline.by3, a.baseline.games) << ','
      << percent(a.full_letter.by4, a.full_letter.games) - percent(a.baseline.by4, a.baseline.games) << ','
      << percent(a.split.by2, a.split.games) - percent(a.full_letter.by2, a.full_letter.games) << ','
      << percent(a.split.by3, a.split.games) - percent(a.full_letter.by3, a.full_letter.games) << ','
      << percent(a.split.by4, a.split.games) - percent(a.full_letter.by4, a.full_letter.games) << ','
      << a.split_letter_played_games << ','
      << percent(a.split_letter_played_games, a.split.games) << ','
      << a.split_letter_total_plays << ','
      << a.split_first_play_turn[0] << ',' << a.split_first_play_turn[1] << ','
      << a.split_first_play_turn[2] << ',' << a.split_first_play_turn[3] << ','
      << a.split_first_play_turn[4] << ',' << a.split_locked_plays << ',';
  write_counts(out, a.split_played_baseline);
  out << ',';
  write_counts(out, a.split_played_split);
  out << ',';
  write_counts(out, a.split_not_played_baseline);
  out << ',';
  write_counts(out, a.split_not_played_split);
  out << '\n';
}

void analyze_all(const std::uint64_t trials, const std::uint64_t seed,
                 const std::string& output_path) {
  const DeckRecipe baseline = baseline_recipe();
  const DeckRecipe split = one_one_recipe();
  const DeckRecipe full_letter = full_letter_recipe();
  verify_pairing(baseline, split, full_letter);

  std::ofstream out(output_path, std::ios::binary | std::ios::trunc);
  if (!out) throw std::runtime_error("could not open output");
  write_header(out);

  const auto scenarios = all_scenarios();
  for (std::size_t index = 0; index < scenarios.size(); ++index) {
    const std::size_t seed_slot =
        index + (index >= 4 ? 1U : 0U) + (index >= 10 ? 1U : 0U);
    const ScenarioAggregate aggregate = analyze_scenario(
        scenarios[index], baseline, split, full_letter, trials,
        seed + 104729ULL * seed_slot);
    if (aggregate.split_locked_plays != 0U) {
      throw std::logic_error("Professor's Letter was played while Item lock was active");
    }
    write_row(out, scenarios[index], aggregate);
  }
}

}  // namespace sim

int main(int argc, char** argv) {
  try {
    if (argc != 4) {
      std::cerr << "usage: professors_letter_one_one_analysis TRIALS SEED OUTPUT.csv\n";
      return 2;
    }
    const std::uint64_t trials = std::stoull(argv[1]);
    if (trials == 0U) throw std::runtime_error("TRIALS must be positive");
    sim::analyze_all(trials, std::stoull(argv[2]), argv[3]);
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
