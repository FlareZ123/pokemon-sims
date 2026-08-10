#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {

DeckRecipe letter_recipe() {
  DeckRecipe recipe = baseline_recipe();
  const auto vessel = std::find_if(
      recipe.begin(), recipe.end(),
      [](const auto& entry) { return entry.first == Card::EarthenVessel; });
  if (vessel == recipe.end() || vessel->second != 2) {
    throw std::logic_error(
        "regidrago-shell no longer has exactly two Earthen Vessel");
  }

  // Preserve the two physical recipe slots so a paired seed starts from the
  // same shuffle permutation for all 58 unchanged cards. Erasing Vessel and
  // appending Letter changed the pre-shuffle vector and invalidated the old
  // common-random-number claim.
  // Professor's Letter: https://api.pokemontcg.io/v2/cards/xy1-123
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Confirmed comparison bug: https://github.com/FlareZ123/pokemon-sims/issues/2599
  vessel->first = Card::ProfessorsLetter;

  NamedDeck validation{"professors-letter-temporary-swap", recipe};
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

void verify_pair(const DeckRecipe& baseline, const DeckRecipe& letter) {
  const auto baseline_cards = expanded_recipe(baseline);
  const auto letter_cards = expanded_recipe(letter);
  if (baseline_cards.size() != 60U || letter_cards.size() != 60U ||
      baseline_cards.size() != letter_cards.size()) {
    throw std::logic_error("Letter comparison recipes must both contain 60 cards");
  }

  std::size_t differences = 0;
  for (std::size_t index = 0; index < baseline_cards.size(); ++index) {
    if (baseline_cards[index] == letter_cards[index]) continue;
    ++differences;
    if (baseline_cards[index] != Card::EarthenVessel ||
        letter_cards[index] != Card::ProfessorsLetter) {
      throw std::logic_error("Letter pairing changed a non-Vessel physical slot");
    }
  }
  if (differences != 2U) {
    throw std::logic_error("Letter pairing must replace exactly two Vessel slots");
  }
}

TrialOutcome run_game(
    const Scenario& scenario, const DeckRecipe& recipe,
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

std::size_t occurrences(
    const std::string& text, const std::string& needle) {
  std::size_t count = 0;
  std::size_t position = 0;
  while ((position = text.find(needle, position)) != std::string::npos) {
    ++count;
    position += needle.size();
  }
  return count;
}

std::string search_signature(const std::string& line) {
  const std::size_t grass = occurrences(line, "Grass Energy");
  const std::size_t fire = occurrences(line, "Fire Energy");
  if (grass == 2U && fire == 0U) return "GG";
  if (grass == 1U && fire == 1U) return "GF";
  if (grass == 0U && fire == 2U) return "FF";
  if (grass == 1U && fire == 0U) return "G";
  if (grass == 0U && fire == 1U) return "F";
  if (grass == 0U && fire == 0U) return "none";
  return "other";
}

bool item_locked_on_turn(const Scenario& scenario, const int turn) {
  return scenario.locks == LockMode::FullItem ||
      ((scenario.locks == LockMode::TurnTwoItem ||
        scenario.locks == LockMode::FullCombined) &&
       turn >= 2);
}

struct LetterFacts {
  bool played{false};
  std::uint64_t total_plays{0};
  std::array<std::uint64_t, 5> plays_by_turn{};
  std::uint64_t locked_plays{0};
  int first_play_turn{0};
  std::string first_signature{"none"};
  std::map<std::string, std::uint64_t> signatures;
};

LetterFacts inspect_letter_trace(
    const TraceLog& trace, const Scenario& scenario) {
  LetterFacts facts;
  for (const std::string& line : trace.lines) {
    // Production TraceLog uses the card name as the action. The previous audit
    // looked for a synthetic "PLAY ITEM" action and therefore missed real plays.
    // Professor's Letter: https://api.pokemontcg.io/v2/cards/xy1-123
    // Trace/parser bug: https://github.com/FlareZ123/pokemon-sims/issues/2599
    if (line.find("| Professor's Letter | rules:") == std::string::npos) continue;
    const int turn = parsed_turn(line);
    const std::string signature = search_signature(line);
    facts.played = true;
    ++facts.total_plays;
    if (turn >= 1 && turn <= 5) {
      ++facts.plays_by_turn[static_cast<std::size_t>(turn - 1)];
    }
    if (item_locked_on_turn(scenario, turn)) ++facts.locked_plays;
    ++facts.signatures[signature];
    if (facts.first_play_turn == 0) {
      facts.first_play_turn = turn;
      facts.first_signature = signature;
    }
  }
  return facts;
}

struct LetterPairedMetric {
  std::int64_t sum_diff{0};
  std::uint64_t sum_diff_sq{0};

  void add(const bool baseline, const bool letter) {
    const int difference = static_cast<int>(letter) -
        static_cast<int>(baseline);
    sum_diff += difference;
    sum_diff_sq += static_cast<std::uint64_t>(difference * difference);
  }

  double delta_pp(const std::uint64_t n) const {
    return n == 0 ? 0.0 :
        100.0 * static_cast<double>(sum_diff) / static_cast<double>(n);
  }

  double se_pp(const std::uint64_t n) const {
    if (n < 2U) return 0.0;
    const double sample = static_cast<double>(n);
    const double mean = static_cast<double>(sum_diff) / sample;
    const double variance =
        (static_cast<double>(sum_diff_sq) - sample * mean * mean) /
        (sample - 1.0);
    return 100.0 * std::sqrt(std::max(0.0, variance) / sample);
  }
};

int objective_turn(const TrialOutcome& outcome) {
  return outcome.first_ready_turn >= 2 && outcome.first_ready_turn <= 4
      ? outcome.first_ready_turn : 5;
}

struct LetterConditionalAggregate {
  std::uint64_t games{0};
  std::uint64_t baseline_by2{0};
  std::uint64_t baseline_by3{0};
  std::uint64_t baseline_by4{0};
  std::uint64_t letter_by2{0};
  std::uint64_t letter_by3{0};
  std::uint64_t letter_by4{0};
  std::uint64_t letter_earlier{0};
  std::uint64_t baseline_earlier{0};
  std::uint64_t tied{0};
  std::uint64_t letter_only_t4{0};
  std::uint64_t baseline_only_t4{0};

  void add(const TrialOutcome& baseline, const TrialOutcome& letter) {
    ++games;
    baseline_by2 += baseline.ready_by_2 ? 1U : 0U;
    baseline_by3 += baseline.ready_by_3 ? 1U : 0U;
    baseline_by4 += baseline.ready_by_4 ? 1U : 0U;
    letter_by2 += letter.ready_by_2 ? 1U : 0U;
    letter_by3 += letter.ready_by_3 ? 1U : 0U;
    letter_by4 += letter.ready_by_4 ? 1U : 0U;

    const int baseline_turn = objective_turn(baseline);
    const int letter_turn = objective_turn(letter);
    if (letter_turn < baseline_turn) ++letter_earlier;
    else if (baseline_turn < letter_turn) ++baseline_earlier;
    else ++tied;

    if (letter.ready_by_4 && !baseline.ready_by_4) ++letter_only_t4;
    if (baseline.ready_by_4 && !letter.ready_by_4) ++baseline_only_t4;
  }
};

struct LetterAnalysisAggregate {
  std::uint64_t trials{0};
  std::uint64_t baseline_by2{0};
  std::uint64_t baseline_by3{0};
  std::uint64_t baseline_by4{0};
  std::uint64_t letter_by2{0};
  std::uint64_t letter_by3{0};
  std::uint64_t letter_by4{0};
  LetterPairedMetric delta2;
  LetterPairedMetric delta3;
  LetterPairedMetric delta4;
  std::uint64_t letter_played_games{0};
  std::uint64_t letter_total_plays{0};
  std::array<std::uint64_t, 5> letter_plays_by_turn{};
  std::uint64_t locked_plays{0};
  std::map<std::string, std::uint64_t> signatures;
  LetterConditionalAggregate played;
  LetterConditionalAggregate not_played;
  std::map<std::pair<int, std::string>, LetterConditionalAggregate> branches;
};

LetterAnalysisAggregate analyze_scenario(
    const Scenario& scenario, const DeckRecipe& baseline,
    const DeckRecipe& letter, const std::uint64_t trials,
    const std::uint64_t first_seed) {
  LetterAnalysisAggregate aggregate;
  for (std::uint64_t trial = 0; trial < trials; ++trial) {
    const std::uint64_t seed = first_seed + trial;
    const TrialOutcome baseline_outcome = run_game(scenario, baseline, seed);

    TraceLog letter_trace{true, {}};
    const TrialOutcome letter_outcome =
        run_game(scenario, letter, seed, &letter_trace);
    const LetterFacts facts = inspect_letter_trace(letter_trace, scenario);

    ++aggregate.trials;
    aggregate.baseline_by2 += baseline_outcome.ready_by_2 ? 1U : 0U;
    aggregate.baseline_by3 += baseline_outcome.ready_by_3 ? 1U : 0U;
    aggregate.baseline_by4 += baseline_outcome.ready_by_4 ? 1U : 0U;
    aggregate.letter_by2 += letter_outcome.ready_by_2 ? 1U : 0U;
    aggregate.letter_by3 += letter_outcome.ready_by_3 ? 1U : 0U;
    aggregate.letter_by4 += letter_outcome.ready_by_4 ? 1U : 0U;
    aggregate.delta2.add(baseline_outcome.ready_by_2, letter_outcome.ready_by_2);
    aggregate.delta3.add(baseline_outcome.ready_by_3, letter_outcome.ready_by_3);
    aggregate.delta4.add(baseline_outcome.ready_by_4, letter_outcome.ready_by_4);

    aggregate.letter_total_plays += facts.total_plays;
    aggregate.locked_plays += facts.locked_plays;
    for (std::size_t index = 0; index < aggregate.letter_plays_by_turn.size(); ++index) {
      aggregate.letter_plays_by_turn[index] += facts.plays_by_turn[index];
    }
    for (const auto& [signature, count] : facts.signatures) {
      aggregate.signatures[signature] += count;
    }

    if (facts.played) {
      ++aggregate.letter_played_games;
      aggregate.played.add(baseline_outcome, letter_outcome);
      aggregate.branches[{facts.first_play_turn, facts.first_signature}]
          .add(baseline_outcome, letter_outcome);
    } else {
      aggregate.not_played.add(baseline_outcome, letter_outcome);
    }
  }
  return aggregate;
}

std::uint64_t signature_count(
    const LetterAnalysisAggregate& aggregate, const std::string& signature) {
  const auto found = aggregate.signatures.find(signature);
  return found == aggregate.signatures.end() ? 0U : found->second;
}

void write_conditional(
    std::ostream& out, const LetterConditionalAggregate& data) {
  out << data.games << ','
      << pct(data.baseline_by2, data.games) << ','
      << pct(data.baseline_by3, data.games) << ','
      << pct(data.baseline_by4, data.games) << ','
      << pct(data.letter_by2, data.games) << ','
      << pct(data.letter_by3, data.games) << ','
      << pct(data.letter_by4, data.games) << ','
      << pct(data.letter_earlier, data.games) << ','
      << pct(data.baseline_earlier, data.games) << ','
      << pct(data.tied, data.games) << ','
      << pct(data.letter_only_t4, data.games) << ','
      << pct(data.baseline_only_t4, data.games);
}

void write_summary_header(std::ostream& out) {
  out << "scenario,trials,"
         "baseline_t2_pct,letter_t2_pct,delta_t2_pp,delta_t2_se_pp,"
         "baseline_t3_pct,letter_t3_pct,delta_t3_pp,delta_t3_se_pp,"
         "baseline_t4_pct,letter_t4_pct,delta_t4_pp,delta_t4_se_pp,"
         "letter_played_games,letter_played_games_pct,letter_total_plays,"
         "letter_play_t1,letter_play_t2,letter_play_t3,letter_play_t4,"
         "letter_play_t5,letter_locked_plays,search_GG,search_GF,search_FF,"
         "search_G,search_F,search_none,search_other,"
         "played_games,played_baseline_t2_pct,played_baseline_t3_pct,"
         "played_baseline_t4_pct,played_letter_t2_pct,played_letter_t3_pct,"
         "played_letter_t4_pct,played_letter_earlier_pct,"
         "played_baseline_earlier_pct,played_tied_pct,"
         "played_letter_only_t4_pct,played_baseline_only_t4_pct,"
         "not_played_games,not_played_baseline_t2_pct,"
         "not_played_baseline_t3_pct,not_played_baseline_t4_pct,"
         "not_played_letter_t2_pct,not_played_letter_t3_pct,"
         "not_played_letter_t4_pct,not_played_letter_earlier_pct,"
         "not_played_baseline_earlier_pct,not_played_tied_pct,"
         "not_played_letter_only_t4_pct,not_played_baseline_only_t4_pct\n";
}

void write_summary_row(
    std::ostream& out, const Scenario& scenario,
    const LetterAnalysisAggregate& aggregate) {
  out << scenario.label << ',' << aggregate.trials << ','
      << std::fixed << std::setprecision(6)
      << pct(aggregate.baseline_by2, aggregate.trials) << ','
      << pct(aggregate.letter_by2, aggregate.trials) << ','
      << aggregate.delta2.delta_pp(aggregate.trials) << ','
      << aggregate.delta2.se_pp(aggregate.trials) << ','
      << pct(aggregate.baseline_by3, aggregate.trials) << ','
      << pct(aggregate.letter_by3, aggregate.trials) << ','
      << aggregate.delta3.delta_pp(aggregate.trials) << ','
      << aggregate.delta3.se_pp(aggregate.trials) << ','
      << pct(aggregate.baseline_by4, aggregate.trials) << ','
      << pct(aggregate.letter_by4, aggregate.trials) << ','
      << aggregate.delta4.delta_pp(aggregate.trials) << ','
      << aggregate.delta4.se_pp(aggregate.trials) << ','
      << aggregate.letter_played_games << ','
      << pct(aggregate.letter_played_games, aggregate.trials) << ','
      << aggregate.letter_total_plays << ','
      << aggregate.letter_plays_by_turn[0] << ','
      << aggregate.letter_plays_by_turn[1] << ','
      << aggregate.letter_plays_by_turn[2] << ','
      << aggregate.letter_plays_by_turn[3] << ','
      << aggregate.letter_plays_by_turn[4] << ','
      << aggregate.locked_plays << ','
      << signature_count(aggregate, "GG") << ','
      << signature_count(aggregate, "GF") << ','
      << signature_count(aggregate, "FF") << ','
      << signature_count(aggregate, "G") << ','
      << signature_count(aggregate, "F") << ','
      << signature_count(aggregate, "none") << ','
      << signature_count(aggregate, "other") << ',';
  write_conditional(out, aggregate.played);
  out << ',';
  write_conditional(out, aggregate.not_played);
  out << '\n';
}

void write_branch_header(std::ostream& out) {
  out << "scenario,first_play_turn,search_signature,games,"
         "games_pct_of_all,games_pct_of_letter_played,"
         "baseline_t2_pct,baseline_t3_pct,baseline_t4_pct,"
         "letter_t2_pct,letter_t3_pct,letter_t4_pct,"
         "letter_earlier_pct,baseline_earlier_pct,tied_pct,"
         "letter_only_t4_pct,baseline_only_t4_pct\n";
}

void write_branch_rows(
    std::ostream& out, const Scenario& scenario,
    const LetterAnalysisAggregate& aggregate) {
  for (const auto& [key, branch] : aggregate.branches) {
    out << scenario.label << ',' << key.first << ',' << key.second << ','
        << branch.games << ','
        << std::fixed << std::setprecision(6)
        << pct(branch.games, aggregate.trials) << ','
        << pct(branch.games, aggregate.letter_played_games) << ','
        << pct(branch.baseline_by2, branch.games) << ','
        << pct(branch.baseline_by3, branch.games) << ','
        << pct(branch.baseline_by4, branch.games) << ','
        << pct(branch.letter_by2, branch.games) << ','
        << pct(branch.letter_by3, branch.games) << ','
        << pct(branch.letter_by4, branch.games) << ','
        << pct(branch.letter_earlier, branch.games) << ','
        << pct(branch.baseline_earlier, branch.games) << ','
        << pct(branch.tied, branch.games) << ','
        << pct(branch.letter_only_t4, branch.games) << ','
        << pct(branch.baseline_only_t4, branch.games) << '\n';
  }
}

void analyze_all(
    const std::uint64_t trials, const std::uint64_t seed,
    const std::string& summary_path, const std::string& branch_path) {
  const DeckRecipe baseline = baseline_recipe();
  const DeckRecipe letter = letter_recipe();
  verify_pair(baseline, letter);

  std::ofstream summary(summary_path, std::ios::binary | std::ios::trunc);
  std::ofstream branches(branch_path, std::ios::binary | std::ios::trunc);
  if (!summary || !branches) throw std::runtime_error("could not open output");
  write_summary_header(summary);
  write_branch_header(branches);

  const auto scenarios = all_scenarios();
  for (std::size_t index = 0; index < scenarios.size(); ++index) {
    const std::size_t seed_slot =
        index + (index >= 4 ? 1U : 0U) + (index >= 10 ? 1U : 0U);
    const LetterAnalysisAggregate aggregate = analyze_scenario(
        scenarios[index], baseline, letter, trials,
        seed + 104729ULL * seed_slot);
    if (aggregate.locked_plays != 0U) {
      throw std::logic_error(
          "Professor's Letter was played while Item lock was active");
    }
    write_summary_row(summary, scenarios[index], aggregate);
    write_branch_rows(branches, scenarios[index], aggregate);
  }
}

int simulate_this(
    const std::string& scenario_label, const std::uint64_t seed,
    const std::string& variant) {
  const auto scenario = scenario_by_label(scenario_label);
  if (!scenario) throw std::runtime_error("unknown scenario: " + scenario_label);

  const DeckRecipe baseline = baseline_recipe();
  const DeckRecipe letter = letter_recipe();
  verify_pair(baseline, letter);
  const DeckRecipe* recipe = nullptr;
  if (variant == "letter-swap") recipe = &letter;
  else if (variant == "regidrago-shell") recipe = &baseline;
  else throw std::runtime_error("variant must be letter-swap or regidrago-shell");

  TraceLog trace{true, {}};
  const TrialOutcome outcome = run_game(*scenario, *recipe, seed, &trace);
  std::cout << "Deck: " << variant << " | Scenario: " << scenario_label
            << " | Seed: " << seed << " | first-ready turn: "
            << outcome.first_ready_turn << '\n';
  for (const std::string& line : trace.lines) std::cout << line << '\n';
  return 0;
}

void trace_letter_plays(
    const std::uint64_t wanted, const std::uint64_t start_seed,
    const std::string& scenario_label, const std::string& output_path) {
  const auto scenario = scenario_by_label(scenario_label);
  if (!scenario) throw std::runtime_error("unknown scenario: " + scenario_label);
  const DeckRecipe baseline = baseline_recipe();
  const DeckRecipe letter = letter_recipe();
  verify_pair(baseline, letter);

  std::ofstream out(output_path, std::ios::binary | std::ios::trunc);
  if (!out) throw std::runtime_error("could not open trace output");

  constexpr std::uint64_t max_scan = 1000000ULL;
  std::uint64_t found = 0;
  for (std::uint64_t offset = 0; offset < max_scan && found < wanted; ++offset) {
    if (start_seed > ~std::uint64_t{0} - offset) {
      throw std::runtime_error("Letter trace scan overflowed uint64 seed range");
    }
    const std::uint64_t seed = start_seed + offset;
    TraceLog letter_trace{true, {}};
    const TrialOutcome letter_outcome =
        run_game(*scenario, letter, seed, &letter_trace);
    const LetterFacts facts = inspect_letter_trace(letter_trace, *scenario);
    if (!facts.played) continue;

    TraceLog baseline_trace{true, {}};
    const TrialOutcome baseline_outcome =
        run_game(*scenario, baseline, seed, &baseline_trace);
    out << "=== FULL-DECK PAIR " << (found + 1U)
        << " | scenario=" << scenario_label
        << " | seed=" << seed
        << " | first_letter_turn=" << facts.first_play_turn
        << " | first_signature=" << facts.first_signature
        << " | baseline_ready=" << baseline_outcome.first_ready_turn
        << " | letter_ready=" << letter_outcome.first_ready_turn
        << " ===\n";
    out << "--- regidrago-shell ---\n";
    for (const std::string& line : baseline_trace.lines) out << line << '\n';
    out << "--- letter-swap ---\n";
    for (const std::string& line : letter_trace.lines) out << line << '\n';
    out << '\n';
    ++found;
  }
  if (found != wanted) {
    throw std::runtime_error(
        "could not find requested Letter plays within one million full games");
  }
}

}  // namespace sim

int main(int argc, char** argv) {
  try {
    if (argc == 6 && std::string(argv[1]) == "--analyze-real-games") {
      const std::uint64_t trials = std::stoull(argv[2]);
      if (trials == 0) throw std::runtime_error("TRIALS must be positive");
      sim::analyze_all(trials, std::stoull(argv[3]), argv[4], argv[5]);
      return 0;
    }
    if (argc == 5 && std::string(argv[1]) == "--simulate-this") {
      return sim::simulate_this(argv[2], std::stoull(argv[3]), argv[4]);
    }
    if (argc == 6 && std::string(argv[1]) == "--trace-letter-plays") {
      const std::uint64_t wanted = std::stoull(argv[2]);
      if (wanted == 0) throw std::runtime_error("COUNT must be positive");
      sim::trace_letter_plays(
          wanted, std::stoull(argv[3]), argv[4], argv[5]);
      return 0;
    }

    std::cerr
        << "usage:\n"
        << "  professors_letter_real_game_analysis --analyze-real-games "
           "TRIALS SEED SUMMARY.csv BRANCHES.csv\n"
        << "  professors_letter_real_game_analysis --simulate-this "
           "SCENARIO SEED {regidrago-shell|letter-swap}\n"
        << "  professors_letter_real_game_analysis --trace-letter-plays "
           "COUNT START_SEED SCENARIO OUTPUT.txt\n";
    return 2;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
