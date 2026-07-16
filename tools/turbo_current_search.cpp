#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr std::array<sim::Card, 38> kCards{{
    sim::Card::RegidragoV, sim::Card::RegidragoVstar, sim::Card::Dragapult,
    sim::Card::MegaDragonite, sim::Card::DialgaGX, sim::Card::GoodraVstar,
    sim::Card::TapuLeleGX, sim::Card::LatiasEx, sim::Card::MawileGX,
    sim::Card::Oricorio, sim::Card::Dipplin, sim::Card::BrilliantBlender,
    sim::Card::MysteriousTreasure, sim::Card::QuickBall, sim::Card::UltraBall,
    sim::Card::EvolutionIncense, sim::Card::EarthenVessel, sim::Card::Arven,
    sim::Card::Crispin, sim::Card::ProfessorBurnet, sim::Card::Serena,
    sim::Card::TateLiza, sim::Card::StevensResolve, sim::Card::Guzma,
    sim::Card::Channeler, sim::Card::Gladion, sim::Card::Lusamine,
    sim::Card::TeamYellsCheer, sim::Card::RoseannesBackup, sim::Card::ProfessorTuro,
    sim::Card::ForestSealStone, sim::Card::Powerglass, sim::Card::FieldBlower,
    sim::Card::ChaoticSwell, sim::Card::PathToPeak, sim::Card::HisuianHeavyBall,
    sim::Card::Grass, sim::Card::Fire,
}};

// Restrict cuts to the low-sensitivity flex surface. Structural Regidrago, Energy,
// acceleration, and primary search cards remain protected during beam expansion.
constexpr std::array<sim::Card, 15> kRemovalCandidates{{
    sim::Card::DialgaGX, sim::Card::GoodraVstar, sim::Card::MawileGX,
    sim::Card::Oricorio, sim::Card::Dipplin, sim::Card::Serena,
    sim::Card::Guzma, sim::Card::Channeler, sim::Card::Gladion,
    sim::Card::Lusamine, sim::Card::TeamYellsCheer, sim::Card::RoseannesBackup,
    sim::Card::ProfessorTuro, sim::Card::FieldBlower, sim::Card::ChaoticSwell,
}};

constexpr std::array<sim::Card, 14> kAdditionCandidates{{
    sim::Card::RegidragoVstar, sim::Card::Dragapult, sim::Card::MegaDragonite,
    sim::Card::TapuLeleGX, sim::Card::MysteriousTreasure, sim::Card::QuickBall,
    sim::Card::UltraBall, sim::Card::EvolutionIncense, sim::Card::EarthenVessel,
    sim::Card::Arven, sim::Card::Crispin, sim::Card::ProfessorBurnet,
    sim::Card::TateLiza, sim::Card::StevensResolve,
}};

using Counts = std::array<int, kCards.size()>;

struct Evaluated {
  Counts counts{};
  double score{};
  std::array<double, 4> values{};
};

int index_of(const sim::Card card) {
  const auto it = std::find(kCards.begin(), kCards.end(), card);
  if (it == kCards.end()) throw std::runtime_error("card missing from search array");
  return static_cast<int>(std::distance(kCards.begin(), it));
}

Counts to_counts(const sim::DeckRecipe& recipe) {
  Counts counts{};
  for (const auto& [card, copies] : recipe) {
    counts[static_cast<std::size_t>(index_of(card))] = copies;
  }
  return counts;
}

sim::DeckRecipe to_recipe(const Counts& counts) {
  sim::DeckRecipe recipe;
  for (std::size_t i = 0; i < counts.size(); ++i) {
    if (counts[i] > 0) recipe.emplace_back(kCards[i], counts[i]);
  }
  return recipe;
}

bool legal_counts(const Counts& counts) {
  int total = 0;
  for (std::size_t i = 0; i < counts.size(); ++i) {
    const int copies = counts[i];
    if (copies < 0) return false;
    const bool basic_energy = kCards[i] == sim::Card::Grass || kCards[i] == sim::Card::Fire;
    if (!basic_energy && copies > 4) return false;
    total += copies;
  }
  if (counts[static_cast<std::size_t>(index_of(sim::Card::BrilliantBlender))] > 1) {
    return false;
  }
  return total == 60;
}

std::string key(const Counts& counts) {
  std::string result;
  for (const int copies : counts) {
    result += std::to_string(copies);
    result.push_back(',');
  }
  return result;
}

std::string delta_label(const Counts& counts, const Counts& base) {
  std::ostringstream out;
  bool first = true;
  for (std::size_t i = 0; i < counts.size(); ++i) {
    const int delta = counts[i] - base[i];
    if (delta == 0) continue;
    if (!first) out << ' ';
    out << (delta > 0 ? "+" : "") << delta << ' ' << sim::name(kCards[i]);
    first = false;
  }
  return first ? "baseline" : out.str();
}

Evaluated evaluate(const Counts& counts, const std::uint64_t trials,
                   const std::uint64_t seed) {
  static const auto go_first = sim::scenario_by_label("strict-jit/go-first").value();
  static const auto go_second = sim::scenario_by_label("strict-jit/go-second").value();
  const sim::DeckRecipe recipe = to_recipe(counts);
  const sim::Aggregate first = sim::simulate(go_first, recipe, trials, seed);
  const sim::Aggregate second = sim::simulate(go_second, recipe, trials, seed + 733003ULL);

  Evaluated result;
  result.counts = counts;
  result.values = {
      sim::pct(first.by2, first.trials), sim::pct(first.by3, first.trials),
      sim::pct(second.by2, second.trials), sim::pct(second.by3, second.trials),
  };
  // Turbo objective prioritizes the harder going-first T2 gate, then going-second
  // T2, while retaining T3 as a secondary Pareto dimension:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scope
  result.score = 4.0 * result.values[0] + result.values[1] +
                 2.0 * result.values[2] + 0.5 * result.values[3];
  return result;
}

std::vector<Evaluated> expand(const std::vector<Evaluated>& beam,
                              const std::uint64_t trials,
                              const std::uint64_t seed,
                              const std::size_t keep) {
  std::map<std::string, Counts> unique;
  for (const Evaluated& parent : beam) {
    for (const sim::Card removed_card : kRemovalCandidates) {
      const std::size_t removed = static_cast<std::size_t>(index_of(removed_card));
      if (parent.counts[removed] == 0) continue;
      for (const sim::Card added_card : kAdditionCandidates) {
        const std::size_t added = static_cast<std::size_t>(index_of(added_card));
        if (removed == added) continue;
        Counts child = parent.counts;
        --child[removed];
        ++child[added];
        if (legal_counts(child)) unique.emplace(key(child), child);
      }
    }
  }

  std::vector<Evaluated> results;
  results.reserve(unique.size());
  for (const auto& [unused, counts] : unique) {
    static_cast<void>(unused);
    results.push_back(evaluate(counts, trials, seed));
  }
  std::sort(results.begin(), results.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.score > rhs.score;
  });
  if (results.size() > keep) results.resize(keep);
  return results;
}

}  // namespace

int main(int argc, char** argv) {
  const std::uint64_t screen_trials = argc > 1 ? std::stoull(argv[1]) : 50;
  const std::uint64_t confirm_trials = argc > 2 ? std::stoull(argv[2]) : 50000;
  const std::size_t beam_width = argc > 3 ? std::stoull(argv[3]) : 12;
  const Counts base = to_counts(sim::baseline_recipe());

  std::vector<Evaluated> beam{evaluate(base, screen_trials, 20260716)};
  for (int depth = 1; depth <= 3; ++depth) {
    beam = expand(beam, screen_trials, 20260716 + 1000000ULL * depth, beam_width);
    std::cerr << "depth " << depth << " best screen score " << beam.front().score << '\n';
  }

  std::vector<Evaluated> confirmed;
  for (const Evaluated& candidate : beam) {
    confirmed.push_back(evaluate(candidate.counts, confirm_trials, 20260716 + 9000000ULL));
  }
  confirmed.push_back(evaluate(base, confirm_trials, 20260716 + 9000000ULL));
  std::sort(confirmed.begin(), confirmed.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.score > rhs.score;
  });

  std::cout << "rank,variant,score,go_first_t2_pct,go_first_t3_pct,go_second_t2_pct,go_second_t3_pct\n";
  for (std::size_t i = 0; i < confirmed.size(); ++i) {
    const Evaluated& result = confirmed[i];
    std::cout << i + 1 << ',' << '"' << delta_label(result.counts, base) << '"' << ','
              << std::fixed << std::setprecision(4) << result.score << ','
              << result.values[0] << ',' << result.values[1] << ','
              << result.values[2] << ',' << result.values[3] << '\n';
  }
}
