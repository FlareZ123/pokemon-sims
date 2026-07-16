#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Variant {
  std::string label;
  sim::DeckRecipe recipe;
};

sim::DeckRecipe swap_one(sim::DeckRecipe recipe, sim::Card removed, sim::Card added) {
  auto it = std::find_if(recipe.begin(), recipe.end(), [removed](const auto& entry) {
    return entry.first == removed && entry.second > 0;
  });
  if (it == recipe.end()) throw std::runtime_error("missing removal card");
  --it->second;
  if (it->second == 0) recipe.erase(it);
  auto add = std::find_if(recipe.begin(), recipe.end(), [added](const auto& entry) {
    return entry.first == added;
  });
  if (add == recipe.end()) recipe.emplace_back(added, 1); else ++add->second;
  return recipe;
}

sim::DeckRecipe swaps(sim::DeckRecipe recipe,
                      std::initializer_list<std::pair<sim::Card, sim::Card>> changes) {
  for (const auto [removed, added] : changes) recipe = swap_one(std::move(recipe), removed, added);
  return recipe;
}

std::vector<Variant> variants() {
  const sim::DeckRecipe base = sim::baseline_recipe();
  return {
    {"baseline", base},
    {"current-3-vstar-2-evolution-incense",
     swaps(base, {{sim::Card::DialgaGX, sim::Card::RegidragoVstar},
                  {sim::Card::RoseannesBackup, sim::Card::EvolutionIncense},
                  {sim::Card::FieldBlower, sim::Card::EvolutionIncense}})},
    {"current-4-quick-evolution-vessel-crispin",
     swaps(base, {{sim::Card::MawileGX, sim::Card::QuickBall},
                  {sim::Card::Guzma, sim::Card::EvolutionIncense},
                  {sim::Card::TeamYellsCheer, sim::Card::EarthenVessel},
                  {sim::Card::ChaoticSwell, sim::Card::Crispin}})},
    {"pokestop-3-vstar-evolution",
     swaps(base, {{sim::Card::DialgaGX, sim::Card::RegidragoVstar},
                  {sim::Card::RoseannesBackup, sim::Card::EvolutionIncense},
                  {sim::Card::FieldBlower, sim::Card::PokeStop}})},
    {"pokestop-4-vstar-evolution-quick",
     swaps(base, {{sim::Card::DialgaGX, sim::Card::RegidragoVstar},
                  {sim::Card::RoseannesBackup, sim::Card::EvolutionIncense},
                  {sim::Card::FieldBlower, sim::Card::PokeStop},
                  {sim::Card::ChaoticSwell, sim::Card::QuickBall}})},
    {"vip-3-two-vip-ultra",
     swaps(base, {{sim::Card::FieldBlower, sim::Card::BattleVipPass},
                  {sim::Card::ChaoticSwell, sim::Card::BattleVipPass},
                  {sim::Card::PathToPeak, sim::Card::UltraBall}})},
    {"vip-3-two-vip-steven",
     swaps(base, {{sim::Card::FieldBlower, sim::Card::BattleVipPass},
                  {sim::Card::ChaoticSwell, sim::Card::BattleVipPass},
                  {sim::Card::PathToPeak, sim::Card::StevensResolve}})},
    {"vip-4-two-vip-vstar-evolution",
     swaps(base, {{sim::Card::DialgaGX, sim::Card::BattleVipPass},
                  {sim::Card::RoseannesBackup, sim::Card::BattleVipPass},
                  {sim::Card::FieldBlower, sim::Card::RegidragoVstar},
                  {sim::Card::ChaoticSwell, sim::Card::EvolutionIncense}})},
    {"carmine-4-vstar-two-evolution",
     swaps(base, {{sim::Card::DialgaGX, sim::Card::Carmine},
                  {sim::Card::RoseannesBackup, sim::Card::RegidragoVstar},
                  {sim::Card::FieldBlower, sim::Card::EvolutionIncense},
                  {sim::Card::ChaoticSwell, sim::Card::EvolutionIncense}})},
    {"carmine-vip-vstar-evolution",
     swaps(base, {{sim::Card::DialgaGX, sim::Card::Carmine},
                  {sim::Card::RoseannesBackup, sim::Card::BattleVipPass},
                  {sim::Card::FieldBlower, sim::Card::RegidragoVstar},
                  {sim::Card::ChaoticSwell, sim::Card::EvolutionIncense}})},
    {"pokestop-vip-vstar-evolution",
     swaps(base, {{sim::Card::DialgaGX, sim::Card::PokeStop},
                  {sim::Card::RoseannesBackup, sim::Card::BattleVipPass},
                  {sim::Card::FieldBlower, sim::Card::RegidragoVstar},
                  {sim::Card::ChaoticSwell, sim::Card::EvolutionIncense}})},
  };
}

}  // namespace

int main(int argc, char** argv) {
  const std::uint64_t trials = argc > 1 ? std::stoull(argv[1]) : 100000;
  const std::size_t variant_index = argc > 2 ? std::stoull(argv[2]) : 0;
  const auto choices = variants();
  if (variant_index >= choices.size()) throw std::runtime_error("variant index out of range");
  const Variant& variant = choices[variant_index];
  const auto scenarios = sim::all_scenarios();
  std::cout << "variant,scenario,trials,t2_pct,t3_pct,t4_pct,t2_se_pp,t3_se_pp\n";
  for (std::size_t i = 0; i < scenarios.size(); ++i) {
    const auto aggregate = sim::simulate(scenarios[i], variant.recipe, trials,
                                         20260705ULL + 104729ULL * i);
    std::cout << '"' << variant.label << "\",\"" << scenarios[i].label << "\"," << trials << ','
              << std::fixed << std::setprecision(6)
              << sim::pct(aggregate.by2, aggregate.trials) << ','
              << sim::pct(aggregate.by3, aggregate.trials) << ','
              << sim::pct(aggregate.by4, aggregate.trials) << ','
              << sim::standard_error_pp(aggregate.by2, aggregate.trials) << ','
              << sim::standard_error_pp(aggregate.by3, aggregate.trials) << '\n';
  }
}
