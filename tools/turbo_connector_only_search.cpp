#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

const std::vector<sim::Card> kCards{
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
    sim::Card::Grass, sim::Card::Fire, sim::Card::BattleVipPass,
    sim::Card::PokeStop, sim::Card::Carmine,
};

// Only setup connectors may be removed. Cards with independent matchup, attack,
// recovery, gust, lock-answer, denial, or win-condition value are absent here and
// therefore remain fixed at their baseline counts.
const std::vector<sim::Card> kRemovalCandidates{
    sim::Card::TapuLeleGX,
    sim::Card::LatiasEx,
    sim::Card::Oricorio,
    sim::Card::BrilliantBlender,
    sim::Card::MysteriousTreasure,
    sim::Card::QuickBall,
    sim::Card::UltraBall,
    sim::Card::EvolutionIncense,
    sim::Card::EarthenVessel,
    sim::Card::Arven,
    sim::Card::Crispin,
    sim::Card::ProfessorBurnet,
    sim::Card::StevensResolve,
    sim::Card::ForestSealStone,
    sim::Card::Powerglass,
    sim::Card::HisuianHeavyBall,
};

const std::vector<sim::Card> kAdditionCandidates{
    sim::Card::TapuLeleGX,
    sim::Card::LatiasEx,
    sim::Card::Oricorio,
    sim::Card::BrilliantBlender,
    sim::Card::MysteriousTreasure,
    sim::Card::QuickBall,
    sim::Card::UltraBall,
    sim::Card::EvolutionIncense,
    sim::Card::EarthenVessel,
    sim::Card::Arven,
    sim::Card::Crispin,
    sim::Card::ProfessorBurnet,
    sim::Card::StevensResolve,
    sim::Card::ForestSealStone,
    sim::Card::Powerglass,
    sim::Card::HisuianHeavyBall,
    sim::Card::BattleVipPass,
    sim::Card::PokeStop,
    sim::Card::Carmine,
};

using Counts = std::vector<int>;

struct Evaluated {
  Counts counts;
  double turbo_score{};
  double robust_score{};
  std::array<double, 4> strict{};
};

int index_of(const sim::Card card) {
  const auto it = std::find(kCards.begin(), kCards.end(), card);
  if (it == kCards.end()) throw std::runtime_error("card missing from search array");
  return static_cast<int>(std::distance(kCards.begin(), it));
}

bool is_removable_connector(const sim::Card card) {
  return std::find(kRemovalCandidates.begin(), kRemovalCandidates.end(), card) !=
         kRemovalCandidates.end();
}

Counts to_counts(const sim::DeckRecipe& recipe) {
  Counts counts(kCards.size(), 0);
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

bool protected_counts_unchanged(const Counts& counts, const Counts& base) {
  for (std::size_t i = 0; i < counts.size(); ++i) {
    if (!is_removable_connector(kCards[i]) &&
        std::find(kAdditionCandidates.begin(), kAdditionCandidates.end(), kCards[i]) ==
            kAdditionCandidates.end() &&
        counts[i] != base[i]) {
      return false;
    }
  }
  const std::vector<sim::Card> protected_cards{
      sim::Card::RegidragoV, sim::Card::RegidragoVstar, sim::Card::Dragapult,
      sim::Card::MegaDragonite, sim::Card::DialgaGX, sim::Card::GoodraVstar,
      sim::Card::MawileGX, sim::Card::Dipplin, sim::Card::Serena,
      sim::Card::TateLiza, sim::Card::Guzma, sim::Card::Channeler,
      sim::Card::Gladion, sim::Card::Lusamine, sim::Card::TeamYellsCheer,
      sim::Card::RoseannesBackup, sim::Card::ProfessorTuro, sim::Card::FieldBlower,
      sim::Card::ChaoticSwell, sim::Card::PathToPeak, sim::Card::Grass,
      sim::Card::Fire,
  };
  for (const sim::Card card : protected_cards) {
    const std::size_t index = static_cast<std::size_t>(index_of(card));
    if (counts[index] != base[index]) return false;
  }
  return true;
}

bool legal_counts(const Counts& counts, const Counts& base) {
  int total = 0;
  for (std::size_t i = 0; i < counts.size(); ++i) {
    const int copies = counts[i];
    if (copies < 0) return false;
    const bool basic_energy = kCards[i] == sim::Card::Grass || kCards[i] == sim::Card::Fire;
    if (!basic_energy && copies > 4) return false;
    total += copies;
  }
  if (counts[static_cast<std::size_t>(index_of(sim::Card::BrilliantBlender))] > 1) return false;
  if (!protected_counts_unchanged(counts, base)) return false;
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

Evaluated evaluate(const Counts& counts, const Counts& base,
                   const std::uint64_t trials, const std::uint64_t seed,
                   const bool all_scenarios) {
  if (!legal_counts(counts, base)) throw std::logic_error("illegal candidate reached evaluator");
  const sim::DeckRecipe recipe = to_recipe(counts);
  const auto first = sim::scenario_by_label("strict-jit/go-first").value();
  const auto second = sim::scenario_by_label("strict-jit/go-second").value();
  const sim::Aggregate gf = sim::simulate(first, recipe, trials, seed);
  const sim::Aggregate gs = sim::simulate(second, recipe, trials, seed + 733003ULL);

  Evaluated result;
  result.counts = counts;
  result.strict = {
      sim::pct(gf.by2, gf.trials), sim::pct(gf.by3, gf.trials),
      sim::pct(gs.by2, gs.trials), sim::pct(gs.by3, gs.trials),
  };
  result.turbo_score = 4.0 * result.strict[0] + result.strict[1] +
                       2.0 * result.strict[2] + 0.5 * result.strict[3];
  if (all_scenarios) {
    double sum = 0.0;
    const auto scenarios = sim::all_scenarios();
    for (std::size_t i = 0; i < scenarios.size(); ++i) {
      const sim::Aggregate agg = sim::simulate(scenarios[i], recipe, trials,
                                                seed + 10000019ULL * (i + 1));
      sum += sim::pct(agg.by2, agg.trials) + sim::pct(agg.by3, agg.trials);
    }
    result.robust_score = sum / static_cast<double>(scenarios.size());
  }
  return result;
}

std::vector<Evaluated> expand(const std::vector<Evaluated>& beam,
                              const Counts& base,
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
        if (legal_counts(child, base)) unique.emplace(key(child), child);
      }
    }
  }

  std::vector<Evaluated> results;
  results.reserve(unique.size());
  for (const auto& [unused, counts] : unique) {
    static_cast<void>(unused);
    results.push_back(evaluate(counts, base, trials, seed, false));
  }
  std::sort(results.begin(), results.end(), [](const Evaluated& lhs, const Evaluated& rhs) {
    return lhs.turbo_score > rhs.turbo_score;
  });
  if (results.size() > keep) results.resize(keep);
  return results;
}

}  // namespace

int main(int argc, char** argv) {
  const std::uint64_t screen_trials = argc > 1 ? std::stoull(argv[1]) : 100;
  const std::uint64_t confirm_trials = argc > 2 ? std::stoull(argv[2]) : 50000;
  const std::size_t beam_width = argc > 3 ? std::stoull(argv[3]) : 40;
  const int max_depth = argc > 4 ? std::stoi(argv[4]) : 4;
  const Counts base = to_counts(sim::baseline_recipe());
  Counts forbidden_discrete_cut = base;
  --forbidden_discrete_cut[static_cast<std::size_t>(index_of(sim::Card::DialgaGX))];
  ++forbidden_discrete_cut[static_cast<std::size_t>(index_of(sim::Card::QuickBall))];
  if (legal_counts(forbidden_discrete_cut, base)) {
    throw std::logic_error("discrete-card protection self-test failed for Dialga-GX");
  }

  std::vector<Evaluated> beam{evaluate(base, base, screen_trials, 20260716ULL, false)};
  for (int depth = 1; depth <= max_depth; ++depth) {
    beam = expand(beam, base, screen_trials,
                  20260716ULL + 1000000ULL * static_cast<std::uint64_t>(depth),
                  beam_width);
    std::cerr << "depth " << depth << " candidates " << beam.size()
              << " best " << beam.front().turbo_score << '\n';
  }

  std::vector<Evaluated> confirmed;
  confirmed.reserve(beam.size() + 1);
  for (const Evaluated& candidate : beam) {
    confirmed.push_back(evaluate(candidate.counts, base, confirm_trials,
                                 20260716ULL + 9000000ULL, true));
  }
  confirmed.push_back(evaluate(base, base, confirm_trials,
                               20260716ULL + 9000000ULL, true));
  std::sort(confirmed.begin(), confirmed.end(), [](const Evaluated& lhs, const Evaluated& rhs) {
    return lhs.turbo_score > rhs.turbo_score;
  });

  std::cout << "rank,variant,turbo_score,robust_score,go_first_t2_pct,go_first_t3_pct,go_second_t2_pct,go_second_t3_pct\n";
  for (std::size_t i = 0; i < confirmed.size(); ++i) {
    const Evaluated& result = confirmed[i];
    std::cout << i + 1 << ',' << '"' << delta_label(result.counts, base) << '"' << ','
              << std::fixed << std::setprecision(6) << result.turbo_score << ','
              << result.robust_score << ',' << result.strict[0] << ','
              << result.strict[1] << ',' << result.strict[2] << ','
              << result.strict[3] << '\n';
  }
}
