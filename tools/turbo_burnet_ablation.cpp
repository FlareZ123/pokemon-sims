#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using Delta = std::vector<std::pair<sim::Card, int>>;
struct Variant { std::string name; Delta delta; };

// Professor Burnet directly searches and discards up to two deck cards:
// https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Evolution Incense fetches one Evolution Pokemon without discarding it:
// https://api.pokemontcg.io/v2/cards/swsh1-163
// Ultra Ball discards two hand cards before searching a Pokemon:
// https://api.pokemontcg.io/v2/cards/swsh12pt5-146
// Crispin searches two different Basic Energy and attaches one:
// https://api.pokemontcg.io/v2/cards/sv7-133
const std::vector<Variant> kVariants{
    {"baseline", {}},
    {"burnet_to_blank", {{sim::Card::ProfessorBurnet, -1}, {sim::Card::TurboBlank, 1}}},
    {"burnet_to_ultra", {{sim::Card::ProfessorBurnet, -1}, {sim::Card::UltraBall, 1}}},
    {"burnet_to_incense", {{sim::Card::ProfessorBurnet, -1}, {sim::Card::EvolutionIncense, 1}}},
    {"burnet_to_crispin", {{sim::Card::ProfessorBurnet, -1}, {sim::Card::Crispin, 1}}},
    {"leader", {{sim::Card::TapuLeleGX, -1}, {sim::Card::EarthenVessel, -1},
                 {sim::Card::ProfessorBurnet, -1}, {sim::Card::UltraBall, 1},
                 {sim::Card::EvolutionIncense, 1}, {sim::Card::Crispin, 1}}},
    {"leader_keep_burnet_no_ultra", {{sim::Card::TapuLeleGX, -1}, {sim::Card::EarthenVessel, -1},
                                      {sim::Card::EvolutionIncense, 1}, {sim::Card::Crispin, 1}}},
    {"leader_keep_burnet_no_incense", {{sim::Card::TapuLeleGX, -1}, {sim::Card::EarthenVessel, -1},
                                        {sim::Card::UltraBall, 1}, {sim::Card::Crispin, 1}}},
    {"leader_keep_burnet_no_crispin", {{sim::Card::TapuLeleGX, -1}, {sim::Card::EarthenVessel, -1},
                                        {sim::Card::UltraBall, 1}, {sim::Card::EvolutionIncense, 1}}},
};

int count_of(const sim::DeckRecipe& recipe, const sim::Card target) {
  for (const auto& [card, copies] : recipe) if (card == target) return copies;
  return 0;
}
void add_delta(sim::DeckRecipe& recipe, const sim::Card target, const int delta) {
  for (auto& [card, copies] : recipe) if (card == target) { copies += delta; return; }
  recipe.emplace_back(target, delta);
}
sim::DeckRecipe make_recipe(const Variant& variant) {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  for (const auto& [card, delta] : variant.delta) add_delta(recipe, card, delta);
  recipe.erase(std::remove_if(recipe.begin(), recipe.end(), [](const auto& entry) { return entry.second == 0; }), recipe.end());
  int total = 0;
  for (const auto& [card, copies] : recipe) {
    if (copies < 0) throw std::logic_error("negative count");
    const bool energy = card == sim::Card::Grass || card == sim::Card::Fire;
    if (!energy && copies > 4) throw std::logic_error("copy limit");
    total += copies;
  }
  if (total != 60) throw std::logic_error("not 60 cards");
  const sim::DeckRecipe baseline = sim::baseline_recipe();
  const std::vector<sim::Card> protected_cards{
      sim::Card::RegidragoV, sim::Card::RegidragoVstar, sim::Card::Dragapult,
      sim::Card::MegaDragonite, sim::Card::DialgaGX, sim::Card::GoodraVstar,
      sim::Card::MawileGX, sim::Card::Dipplin, sim::Card::Serena,
      sim::Card::TateLiza, sim::Card::Guzma, sim::Card::Channeler,
      sim::Card::Gladion, sim::Card::Lusamine, sim::Card::TeamYellsCheer,
      sim::Card::RoseannesBackup, sim::Card::ProfessorTuro, sim::Card::FieldBlower,
      sim::Card::ChaoticSwell, sim::Card::PathToPeak, sim::Card::Grass, sim::Card::Fire,
  };
  for (const sim::Card card : protected_cards) {
    if (count_of(recipe, card) != count_of(baseline, card)) {
      throw std::logic_error("protected count changed: " + std::string(sim::name(card)));
    }
  }
  if (count_of(recipe, sim::Card::DialgaGX) != 1) throw std::logic_error("Dialga invariant");
  return recipe;
}
std::string delta_text(const Variant& variant) {
  std::string result;
  for (const auto& [card, delta] : variant.delta) {
    if (!result.empty()) result += "; ";
    if (delta > 0) result += "+";
    result += std::to_string(delta) + " " + std::string(sim::name(card));
  }
  return result.empty() ? "baseline" : result;
}
}

int main(int argc, char** argv) {
  const std::uint64_t trials = argc > 1 ? std::stoull(argv[1]) : 100000ULL;
  const std::size_t variant_index = argc > 2 ? std::stoull(argv[2]) : 0U;
  if (variant_index >= kVariants.size()) throw std::out_of_range("variant index");
  const Variant& variant = kVariants[variant_index];
  const sim::DeckRecipe recipe = make_recipe(variant);
  const auto scenarios = sim::all_scenarios();
  std::cout << "variant_index,variant,delta,scenario,trials,t2_pct,t3_pct,t4_pct\n";
  for (std::size_t index = 0; index < scenarios.size(); ++index) {
    const std::uint64_t seed = 20260716ULL + 10000019ULL * (index + 1U);
    const sim::Aggregate aggregate = sim::simulate(scenarios[index], recipe, trials, seed);
    std::cout << variant_index << ',' << '"' << variant.name << '"' << ',' << '"' << delta_text(variant) << '"' << ','
              << '"' << scenarios[index].label << '"' << ',' << trials << ',' << std::fixed << std::setprecision(6)
              << sim::pct(aggregate.by2, aggregate.trials) << ','
              << sim::pct(aggregate.by3, aggregate.trials) << ','
              << sim::pct(aggregate.by4, aggregate.trials) << '\n';
  }
}
