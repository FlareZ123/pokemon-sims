#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr std::uint64_t kTrials = 100000;
constexpr std::uint64_t kSeed = 20260810;
constexpr std::uint64_t kStride = 104729ULL;

struct Candidate {
  const char* label;
  sim::Card cut;
};

constexpr std::array<Candidate, 2> kCandidates{{
    {"Earthen Vessel 2->1", sim::Card::EarthenVessel},
    {"Professor Burnet 1->0", sim::Card::ProfessorBurnet},
}};

// Minior PAR 99 is a Basic Fighting Pokemon, is not a Rule Box Pokemon, is not
// Psychic or Dragon for Mysterious Treasure, and has printed Retreat Cost 2:
// https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sv-series/sv04/99/
//
// The production simulator does not yet have a Minior enum entry. Pineco is an
// exact setup-profile proxy for the properties used by this shell experiment:
// Basic Pokemon, non-Rule-Box, non-Mysterious-Treasure target, Retreat Cost 2.
// The baseline Regidrago shell contains no Forretress ex, so the Pineco-specific
// evolution/Exploding Energy route cannot activate in these temporary recipes:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// Minior's Far-Flying Meteor and Gravitational Tackle remain deliberately outside
// this setup-cost matrix; their matchup value is analyzed separately.
sim::DeckRecipe recipe_with_minior_proxy(const Candidate& candidate) {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::adjust_modeling_recipe(recipe, candidate.cut, -1);
  sim::adjust_modeling_recipe(recipe, sim::Card::Pineco, 1);
  std::string error;
  if (!sim::validate_recipe({std::string("minior-") + candidate.label, recipe}, &error)) {
    throw std::logic_error(error);
  }
  return recipe;
}

double rate(const std::uint64_t value, const std::uint64_t trials) {
  return 100.0 * static_cast<double>(value) / static_cast<double>(trials);
}

double mean_mulligans(const sim::Aggregate& aggregate) {
  return static_cast<double>(aggregate.total_mulligans) /
         static_cast<double>(aggregate.trials);
}

void print_header() {
  std::cout
      << "candidate,scenario,trials,"
         "baseline_t2_pct,candidate_t2_pct,delta_t2_pp,"
         "baseline_t3_pct,candidate_t3_pct,delta_t3_pp,"
         "baseline_t4_pct,candidate_t4_pct,delta_t4_pp,"
         "baseline_t5_pct,candidate_t5_pct,delta_t5_pp,"
         "baseline_failure_pct,candidate_failure_pct,delta_failure_pp,"
         "baseline_mean_mulligans,candidate_mean_mulligans,delta_mean_mulligans,"
         "baseline_started_regi_pct,candidate_started_regi_pct,delta_started_regi_pp,"
         "baseline_started_tapu_pct,candidate_started_tapu_pct,delta_started_tapu_pp,"
         "baseline_t2_missing_vstar_pct,candidate_t2_missing_vstar_pct,delta_t2_missing_vstar_pp,"
         "baseline_t2_missing_grass_pct,candidate_t2_missing_grass_pct,delta_t2_missing_grass_pp,"
         "baseline_t2_missing_fire_pct,candidate_t2_missing_fire_pct,delta_t2_missing_fire_pp,"
         "baseline_t2_missing_payload_pct,candidate_t2_missing_payload_pct,delta_t2_missing_payload_pp,"
         "baseline_blender_used_pct,candidate_blender_used_pct,delta_blender_used_pp,"
         "baseline_steven_used_pct,candidate_steven_used_pct,delta_steven_used_pp,"
         "baseline_fss_used_pct,candidate_fss_used_pct,delta_fss_used_pp\n";
}

void print_triplet(const std::uint64_t baseline,
                   const std::uint64_t candidate,
                   const std::uint64_t trials) {
  const double b = rate(baseline, trials);
  const double c = rate(candidate, trials);
  std::cout << b << ',' << c << ',' << (c - b);
}

void print_row(const Candidate& cut,
               const sim::Scenario& scenario,
               const std::uint64_t trials,
               const sim::Aggregate& baseline,
               const sim::Aggregate& candidate) {
  std::cout << '\"' << cut.label << "\"," << scenario.label << ',' << trials << ','
            << std::fixed << std::setprecision(6);
  print_triplet(baseline.by2, candidate.by2, trials); std::cout << ',';
  print_triplet(baseline.by3, candidate.by3, trials); std::cout << ',';
  print_triplet(baseline.by4, candidate.by4, trials); std::cout << ',';
  print_triplet(baseline.by5, candidate.by5, trials); std::cout << ',';
  print_triplet(baseline.setup_failures, candidate.setup_failures, trials); std::cout << ',';
  const double b_mull = mean_mulligans(baseline);
  const double c_mull = mean_mulligans(candidate);
  std::cout << b_mull << ',' << c_mull << ',' << (c_mull - b_mull) << ',';
  print_triplet(baseline.started_regi, candidate.started_regi, trials); std::cout << ',';
  print_triplet(baseline.started_tapu, candidate.started_tapu, trials); std::cout << ',';
  print_triplet(baseline.t2_missing_active_vstar, candidate.t2_missing_active_vstar, trials); std::cout << ',';
  print_triplet(baseline.t2_missing_grass, candidate.t2_missing_grass, trials); std::cout << ',';
  print_triplet(baseline.t2_missing_fire, candidate.t2_missing_fire, trials); std::cout << ',';
  print_triplet(baseline.t2_missing_payload, candidate.t2_missing_payload, trials); std::cout << ',';
  print_triplet(baseline.blender, candidate.blender, trials); std::cout << ',';
  print_triplet(baseline.steven, candidate.steven, trials); std::cout << ',';
  print_triplet(baseline.fss, candidate.fss, trials);
  std::cout << '\n';
}

}  // namespace

int main() {
  print_header();
  const sim::DeckRecipe baseline_recipe = sim::baseline_recipe();
  const std::vector<sim::Scenario> scenarios = sim::all_scenarios();

  for (std::size_t i = 0; i < scenarios.size(); ++i) {
    const sim::Scenario& scenario = scenarios[i];
    const std::uint64_t common_seed = kSeed + kStride * i;
    const sim::Aggregate baseline = sim::simulate(scenario, baseline_recipe, kTrials, common_seed);
    for (const Candidate& cut : kCandidates) {
      const sim::DeckRecipe recipe = recipe_with_minior_proxy(cut);
      const sim::Aggregate result = sim::simulate(scenario, recipe, kTrials, common_seed);
      print_row(cut, scenario, kTrials, baseline, result);
    }
  }

  return 0;
}
