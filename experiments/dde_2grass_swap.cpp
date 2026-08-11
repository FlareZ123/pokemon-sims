#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

using sim::Card;
using sim::DeckRecipe;
using sim::Engine;
using sim::Scenario;
using sim::TrialOutcome;

// Paper-Expanded experiment only. Double Dragon Energy is currently outside the
// Pokémon TCG Live XY card pool, so this treatment must never enter deck_registry():
// https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
// https://support.pokemon.com/hc/en-us/articles/6489934466708-Pok%C3%A9mon-TCG-Live-Migration-FAQ-from-the-Pok%C3%A9mon-TCG-Online
// Existing paper-only model boundary: https://github.com/FlareZ123/pokemon-sims/issues/2332
// DDE mechanics contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
// Apex Dragon costs GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
constexpr std::uint64_t kTrials = 100000;
constexpr std::uint64_t kBaseSeed = 20260811;
constexpr std::uint64_t kScenarioStride = 104729;

struct Stats {
  std::uint64_t attempted{};
  std::uint64_t valid{};
  std::uint64_t logic_errors{};
  std::array<std::uint64_t, 4> ready{};
  std::uint64_t failures{};
  std::uint64_t steven{};
};

struct PairStats {
  std::uint64_t attempted{};
  std::uint64_t valid{};
  std::uint64_t treatment_faster{};
  std::uint64_t control_faster{};
  std::uint64_t same_ready_turn{};
  std::array<std::uint64_t, 4> treatment_only_ready{};
  std::array<std::uint64_t, 4> control_only_ready{};

  std::uint64_t treatment_steven_pairs{};
  std::uint64_t treatment_steven_faster{};
  std::uint64_t treatment_steven_control_faster{};
  std::uint64_t treatment_steven_same{};

  std::uint64_t both_steven_pairs{};
  std::array<std::uint64_t, 4> both_steven_treatment_ready{};
  std::array<std::uint64_t, 4> both_steven_control_ready{};
};

struct RunResult {
  std::optional<TrialOutcome> outcome;
};

struct Witness {
  std::string scenario;
  std::string kind;
  std::uint64_t seed{};
  int control_turn{};
  int treatment_turn{};
  bool control_steven{};
  bool treatment_steven{};
};

void adjust(DeckRecipe& recipe, const Card card, const int delta) {
  const auto found = std::find_if(
      recipe.begin(), recipe.end(), [card](const auto& entry) {
        return entry.first == card;
      });
  if (found == recipe.end()) {
    if (delta <= 0) throw std::logic_error("experiment cut missing");
    recipe.push_back({card, delta});
    return;
  }
  found->second += delta;
  if (found->second < 0) throw std::logic_error("experiment cut exceeds copies");
  if (found->second == 0) recipe.erase(found);
}

int copies(const DeckRecipe& recipe, const Card card) {
  const auto found = std::find_if(
      recipe.begin(), recipe.end(), [card](const auto& entry) {
        return entry.first == card;
      });
  return found == recipe.end() ? 0 : found->second;
}

std::size_t card_count(const DeckRecipe& recipe) {
  return static_cast<std::size_t>(std::accumulate(
      recipe.begin(), recipe.end(), 0,
      [](const int total, const auto& entry) { return total + entry.second; }));
}

DeckRecipe control_recipe() {
  return sim::baseline_recipe();
}

DeckRecipe treatment_recipe() {
  DeckRecipe recipe = sim::baseline_recipe();
  // Exact treatment requested: keep all three Basic Fire Energy and exchange only
  // two of six Basic Grass Energy for two DDE. DDE is Special Energy, so Crispin,
  // Earthen Vessel, Vital Dance, and Powerglass still see only the four remaining
  // Grass plus three Fire Basic Energy:
  // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  adjust(recipe, Card::Grass, -2);
  adjust(recipe, Card::DoubleDragonEnergy, +2);
  return recipe;
}

double pct(const std::uint64_t count, const std::uint64_t total) {
  return total == 0 ? 0.0
                    : 100.0 * static_cast<double>(count) /
                          static_cast<double>(total);
}

int comparable_ready_turn(const TrialOutcome& outcome) {
  return outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 5
      ? outcome.first_ready_turn
      : 99;
}

std::array<bool, 4> readiness(const TrialOutcome& outcome) {
  return {outcome.ready_by_2, outcome.ready_by_3, outcome.ready_by_4,
          outcome.ready_by_5};
}

RunResult run_one(const Scenario& scenario, const DeckRecipe& recipe,
                  const std::uint64_t seed) {
  try {
    std::mt19937_64 rng(seed);
    Engine engine(scenario, recipe, rng, nullptr);
    return RunResult{engine.run()};
  } catch (const std::logic_error&) {
    return RunResult{std::nullopt};
  }
}

void accumulate(Stats& stats, const RunResult& run) {
  ++stats.attempted;
  if (!run.outcome) {
    ++stats.logic_errors;
    return;
  }
  ++stats.valid;
  const auto ready = readiness(*run.outcome);
  for (std::size_t i = 0; i < ready.size(); ++i) {
    stats.ready[i] += ready[i] ? 1U : 0U;
  }
  stats.failures += run.outcome->setup_failed ? 1U : 0U;
  stats.steven += run.outcome->used_steven ? 1U : 0U;
}

void accumulate_pair(PairStats& pair, const RunResult& control,
                     const RunResult& treatment) {
  ++pair.attempted;
  if (!control.outcome || !treatment.outcome) return;
  ++pair.valid;

  const int control_turn = comparable_ready_turn(*control.outcome);
  const int treatment_turn = comparable_ready_turn(*treatment.outcome);
  if (treatment_turn < control_turn) {
    ++pair.treatment_faster;
  } else if (control_turn < treatment_turn) {
    ++pair.control_faster;
  } else {
    ++pair.same_ready_turn;
  }

  const auto control_ready = readiness(*control.outcome);
  const auto treatment_ready = readiness(*treatment.outcome);
  for (std::size_t i = 0; i < control_ready.size(); ++i) {
    if (treatment_ready[i] && !control_ready[i]) ++pair.treatment_only_ready[i];
    if (control_ready[i] && !treatment_ready[i]) ++pair.control_only_ready[i];
  }

  if (treatment.outcome->used_steven) {
    ++pair.treatment_steven_pairs;
    if (treatment_turn < control_turn) {
      ++pair.treatment_steven_faster;
    } else if (control_turn < treatment_turn) {
      ++pair.treatment_steven_control_faster;
    } else {
      ++pair.treatment_steven_same;
    }
  }

  if (control.outcome->used_steven && treatment.outcome->used_steven) {
    ++pair.both_steven_pairs;
    for (std::size_t i = 0; i < control_ready.size(); ++i) {
      pair.both_steven_control_ready[i] += control_ready[i] ? 1U : 0U;
      pair.both_steven_treatment_ready[i] += treatment_ready[i] ? 1U : 0U;
    }
  }
}

std::string slug(std::string_view text) {
  std::string result;
  result.reserve(text.size());
  for (const unsigned char ch : text) {
    result.push_back(std::isalnum(ch) ? static_cast<char>(ch) : '-');
  }
  return result;
}

std::string trace_text(const Scenario& scenario, const DeckRecipe& recipe,
                       const std::uint64_t seed, const std::string_view variant) {
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  Engine engine(scenario, recipe, rng, &trace);
  const TrialOutcome outcome = engine.run();
  std::ostringstream out;
  out << "variant=" << variant << "\nscenario=" << scenario.label
      << "\nseed=" << seed << "\nfirst_ready_turn="
      << outcome.first_ready_turn << "\nused_steven="
      << (outcome.used_steven ? 1 : 0) << "\n";
  for (const std::string& line : trace.lines) out << line << '\n';
  return out.str();
}

void write_witness_traces(const std::vector<Witness>& witnesses,
                          const DeckRecipe& control,
                          const DeckRecipe& treatment,
                          const std::vector<Scenario>& scenarios) {
  for (const Witness& witness : witnesses) {
    const auto found = std::find_if(
        scenarios.begin(), scenarios.end(), [&witness](const Scenario& scenario) {
          return scenario.label == witness.scenario;
        });
    if (found == scenarios.end()) continue;
    const std::string stem = "trace-dde-2grass-" + slug(witness.scenario) + "-" +
        witness.kind + "-seed" + std::to_string(witness.seed);
    sim::write_atomic(stem + "-control.txt",
                      trace_text(*found, control, witness.seed, "control-6G-3F"));
    sim::write_atomic(stem + "-treatment.txt",
                      trace_text(*found, treatment, witness.seed,
                                 "treatment-4G-3F-2DDE"));
  }
}

}  // namespace

int main() {
  const DeckRecipe control = control_recipe();
  const DeckRecipe treatment = treatment_recipe();

  const sim::NamedDeck control_named{"control", control};
  const sim::NamedDeck treatment_named{"treatment", treatment};
  std::string validation_error;
  if (!sim::validate_recipe(control_named, &validation_error) ||
      !sim::validate_recipe(treatment_named, &validation_error) ||
      card_count(control) != 60U || card_count(treatment) != 60U ||
      copies(control, Card::Grass) != 6 || copies(control, Card::Fire) != 3 ||
      copies(control, Card::DoubleDragonEnergy) != 0 ||
      copies(treatment, Card::Grass) != 4 || copies(treatment, Card::Fire) != 3 ||
      copies(treatment, Card::DoubleDragonEnergy) != 2) {
    std::cerr << "DDE experiment recipe contract failed: " << validation_error << '\n';
    return 2;
  }

  std::ostringstream aggregate_csv;
  std::ostringstream paired_csv;
  std::ostringstream witness_csv;
  aggregate_csv
      << "variant,scenario,trials,valid,logic_error_pct,grass,fire,dde,ready_t2_pct,ready_t3_pct,ready_t4_pct,ready_t5_pct,failure_pct,steven_pct\n";
  paired_csv
      << "scenario,valid_pairs,treatment_faster_pct,control_faster_pct,same_ready_turn_pct,treatment_minus_control_t2_pp,treatment_minus_control_t3_pp,treatment_minus_control_t4_pp,treatment_minus_control_t5_pp,treatment_only_t2,control_only_t2,treatment_only_t3,control_only_t3,treatment_only_t4,control_only_t4,treatment_only_t5,control_only_t5,control_steven_pct,treatment_steven_pct,steven_delta_pp,treatment_steven_pairs,treatment_steven_faster_pct,treatment_steven_control_faster_pct,treatment_steven_same_pct,both_steven_pairs,both_steven_t2_delta_pp,both_steven_t3_delta_pp\n";
  witness_csv
      << "scenario,kind,seed,control_turn,treatment_turn,control_used_steven,treatment_used_steven\n";

  const std::vector<Scenario> scenarios = sim::all_scenarios_with_garbodor();
  std::vector<Witness> witnesses;

  for (std::size_t scenario_index = 0; scenario_index < scenarios.size();
       ++scenario_index) {
    const Scenario& scenario = scenarios[scenario_index];
    const std::uint64_t scenario_seed =
        kBaseSeed + kScenarioStride * scenario_index;
    Stats control_stats{};
    Stats treatment_stats{};
    PairStats pair{};
    int treatment_faster_witnesses = 0;
    bool steven_faster_witness = false;
    bool control_faster_witness = false;

    for (std::uint64_t trial = 0; trial < kTrials; ++trial) {
      const std::uint64_t seed = scenario_seed + trial;
      const RunResult control_run = run_one(scenario, control, seed);
      const RunResult treatment_run = run_one(scenario, treatment, seed);
      accumulate(control_stats, control_run);
      accumulate(treatment_stats, treatment_run);
      accumulate_pair(pair, control_run, treatment_run);

      if (!control_run.outcome || !treatment_run.outcome) continue;
      const int control_turn = comparable_ready_turn(*control_run.outcome);
      const int treatment_turn = comparable_ready_turn(*treatment_run.outcome);
      if (treatment_turn < control_turn && treatment_faster_witnesses < 2) {
        witnesses.push_back({scenario.label, "treatment-faster",
                             seed, control_turn, treatment_turn,
                             control_run.outcome->used_steven,
                             treatment_run.outcome->used_steven});
        ++treatment_faster_witnesses;
      }
      if (treatment_turn < control_turn && treatment_run.outcome->used_steven &&
          !steven_faster_witness) {
        witnesses.push_back({scenario.label, "steven-treatment-faster",
                             seed, control_turn, treatment_turn,
                             control_run.outcome->used_steven,
                             treatment_run.outcome->used_steven});
        steven_faster_witness = true;
      }
      if (control_turn < treatment_turn && !control_faster_witness) {
        witnesses.push_back({scenario.label, "control-faster",
                             seed, control_turn, treatment_turn,
                             control_run.outcome->used_steven,
                             treatment_run.outcome->used_steven});
        control_faster_witness = true;
      }
    }

    if (control_stats.valid != kTrials || treatment_stats.valid != kTrials ||
        pair.valid != kTrials) {
      std::cerr << "logic error in paired experiment for " << scenario.label << '\n';
      return 3;
    }

    const auto write_stats = [&](const std::string_view variant,
                                 const DeckRecipe& recipe,
                                 const Stats& stats) {
      aggregate_csv << variant << ',' << scenario.label << ',' << stats.attempted
                    << ',' << stats.valid << ',' << std::fixed
                    << std::setprecision(6)
                    << pct(stats.logic_errors, stats.attempted) << ','
                    << copies(recipe, Card::Grass) << ','
                    << copies(recipe, Card::Fire) << ','
                    << copies(recipe, Card::DoubleDragonEnergy);
      for (const std::uint64_t count : stats.ready) {
        aggregate_csv << ',' << pct(count, stats.valid);
      }
      aggregate_csv << ',' << pct(stats.failures, stats.valid)
                    << ',' << pct(stats.steven, stats.valid) << '\n';
    };
    write_stats("control-6G-3F", control, control_stats);
    write_stats("treatment-4G-3F-2DDE", treatment, treatment_stats);

    paired_csv << scenario.label << ',' << pair.valid << ',' << std::fixed
               << std::setprecision(6)
               << pct(pair.treatment_faster, pair.valid) << ','
               << pct(pair.control_faster, pair.valid) << ','
               << pct(pair.same_ready_turn, pair.valid);
    for (std::size_t i = 0; i < 4; ++i) {
      paired_csv << ','
                 << (pct(treatment_stats.ready[i], treatment_stats.valid) -
                     pct(control_stats.ready[i], control_stats.valid));
    }
    for (std::size_t i = 0; i < 4; ++i) {
      paired_csv << ',' << pair.treatment_only_ready[i]
                 << ',' << pair.control_only_ready[i];
    }
    paired_csv << ',' << pct(control_stats.steven, control_stats.valid)
               << ',' << pct(treatment_stats.steven, treatment_stats.valid)
               << ',' << (pct(treatment_stats.steven, treatment_stats.valid) -
                           pct(control_stats.steven, control_stats.valid))
               << ',' << pair.treatment_steven_pairs
               << ',' << pct(pair.treatment_steven_faster,
                              pair.treatment_steven_pairs)
               << ',' << pct(pair.treatment_steven_control_faster,
                              pair.treatment_steven_pairs)
               << ',' << pct(pair.treatment_steven_same,
                              pair.treatment_steven_pairs)
               << ',' << pair.both_steven_pairs
               << ',' << (pct(pair.both_steven_treatment_ready[0],
                               pair.both_steven_pairs) -
                           pct(pair.both_steven_control_ready[0],
                               pair.both_steven_pairs))
               << ',' << (pct(pair.both_steven_treatment_ready[1],
                               pair.both_steven_pairs) -
                           pct(pair.both_steven_control_ready[1],
                               pair.both_steven_pairs))
               << '\n';
  }

  for (const Witness& witness : witnesses) {
    witness_csv << witness.scenario << ',' << witness.kind << ',' << witness.seed
                << ',' << witness.control_turn << ',' << witness.treatment_turn
                << ',' << (witness.control_steven ? 1 : 0)
                << ',' << (witness.treatment_steven ? 1 : 0) << '\n';
  }

  std::ostringstream summary;
  summary << "DDE_2GRASS_AGGREGATE\n" << aggregate_csv.str()
          << "\nDDE_2GRASS_PAIRED\n" << paired_csv.str()
          << "\nDDE_2GRASS_WITNESSES\n" << witness_csv.str();
  sim::write_atomic("trace-dde-2grass-summary.txt", summary.str());
  write_witness_traces(witnesses, control, treatment, scenarios);
  std::cout << summary.str();
  return 0;
}
