#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
constexpr std::uint64_t kMaxSeed = 1000000;

bool contains(const sim::TraceLog& trace, const std::string_view needle) {
  for (const std::string& line : trace.lines) {
    if (line.find(needle) != std::string::npos) return true;
  }
  return false;
}

struct Search {
  std::string_view label;
  std::string_view scenario;
  int exact_turn;
  int deadline;
  enum class Contract { Ready, Issue809, Issue802 } contract;
};

bool matches(const Search& search, const sim::TrialOutcome& outcome,
             const sim::TraceLog& trace) {
  if (search.exact_turn > 0 && outcome.first_ready_turn != search.exact_turn) return false;
  if (search.deadline > 0 &&
      (outcome.first_ready_turn == 0 || outcome.first_ready_turn > search.deadline)) return false;

  switch (search.contract) {
    case Search::Contract::Ready:
      return contains(trace, "READY | rules: R-RVS-01; P-JIT-01");
    case Search::Contract::Issue809:
      return contains(trace, "Searched a Basic Pokémon: Tapu Lele-GX.") &&
             contains(trace, "T1 | WONDER TAG | rules: R-TAPU-01 | Searched and revealed Crispin.") &&
             contains(trace, "T2 | READY | rules: R-RVS-01; P-JIT-01") &&
             !contains(trace, "Searched a Basic Pokémon: Oricorio GRI 55.") &&
             !contains(trace, "T2 | STAR ALCHEMY");
    case Search::Contract::Issue802:
      return contains(trace, "T4 | ATTACH | rules: R-GAME-ENERGY | Fire Energy manually to Tapu Lele-GX for its Retreat Cost.") &&
             contains(trace, "T4 | RETREAT | rules: R-GAME-RETREAT | Paid Tapu Lele-GX's one-Energy Retreat Cost and promoted the ready Benched Regidrago VSTAR.") &&
             contains(trace, "T4 | READY | rules: R-RVS-01; P-JIT-01") &&
             !contains(trace, "Latias ex gives the Basic Active no Retreat Cost") &&
             !contains(trace, "Tate & Liza switch mode");
  }
  return false;
}

std::uint64_t find_seed(const Search& search) {
  const auto scenario = sim::scenario_by_label(std::string(search.scenario));
  if (!scenario) throw std::runtime_error("Unknown scenario.");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  for (std::uint64_t seed = 0; seed <= kMaxSeed; ++seed) {
    std::mt19937_64 rng(seed);
    sim::TraceLog trace{true, {}};
    sim::Engine engine(*scenario, recipe, rng, &trace);
    const sim::TrialOutcome outcome = engine.run();
    if (matches(search, outcome, trace)) return seed;
  }
  throw std::runtime_error("No matching seed found for " + std::string(search.label));
}
}  // namespace

int main() {
  constexpr std::array<Search, 6> searches{{
      {"issue-809", "no-discard-control/go-first", 2, 2, Search::Contract::Issue809},
      {"issue-802", "no-discard-control/go-first", 4, 4, Search::Contract::Issue802},
      {"strict-second-t3", "strict-jit/go-second", 0, 3, Search::Contract::Ready},
      {"strict-first-t4", "strict-jit/go-first", 0, 4, Search::Contract::Ready},
      {"turn2-item-lock-t3", "strict-jit-turn2-item-lock/go-second", 0, 3, Search::Contract::Ready},
      {"rulebox-lock-t3", "strict-jit-rulebox-ability-lock/go-second", 0, 3, Search::Contract::Ready},
  }};

  std::cout << "contract,scenario,seed\n";
  for (const Search& search : searches) {
    std::cout << search.label << ',' << search.scenario << ',' << find_seed(search) << '\n';
  }
  return 0;
}
