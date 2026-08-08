#define REGIDRAGO_SIM_NO_MAIN
#include "src/regidrago_sim.cpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {
using namespace sim;

DeckRecipe dde_recipe() {
  DeckRecipe recipe = baseline_recipe();
  const auto it = std::find_if(recipe.begin(), recipe.end(), [](const auto& entry) {
    return entry.first == Card::Grass;
  });
  if (it == recipe.end() || it->second != 6) {
    throw std::logic_error("canonical six-Grass shell missing");
  }
  const auto index = std::distance(recipe.begin(), it);
  it->second = 4;
  recipe.insert(recipe.begin() + index, {Card::DoubleDragonEnergy, 2});
  return recipe;
}

TrialOutcome run_one(const Scenario& scenario, const DeckRecipe& recipe,
                     const std::uint64_t seed, TraceLog* trace = nullptr) {
  std::mt19937_64 rng(seed);
  Engine engine(scenario, recipe, rng, trace);
  return engine.run();
}

struct Event {
  std::string scenario;
  std::string kind;
  std::uint64_t seed{};
  std::string error;
};

void remember(std::vector<Event>& events, const std::string& scenario,
              const std::string& kind, const std::uint64_t seed,
              const std::string& error = {}) {
  const int count = static_cast<int>(std::count_if(
      events.begin(), events.end(), [&](const Event& event) {
        return event.scenario == scenario && event.kind == kind;
      }));
  if (count < 8) events.push_back({scenario, kind, seed, error});
}

double pct_local(const std::uint64_t value, const std::uint64_t total) {
  return total == 0 ? 0.0 : 100.0 * static_cast<double>(value) /
      static_cast<double>(total);
}

void dump_trace(std::ofstream& out, const char* deck_name,
                const Scenario& scenario, const DeckRecipe& recipe,
                const std::uint64_t seed) {
  TraceLog trace{true, {}, {}};
  try {
    const TrialOutcome outcome = run_one(scenario, recipe, seed, &trace);
    out << "===== " << deck_name << " | " << scenario.label << " | seed " << seed
        << " | first-ready=" << outcome.first_ready_turn
        << " | T2=" << outcome.ready_by_2
        << " | T3=" << outcome.ready_by_3 << " =====\n";
  } catch (const std::exception& error) {
    out << "===== " << deck_name << " | " << scenario.label << " | seed " << seed
        << " | EXCEPTION: " << error.what() << " =====\n";
  }
  for (const auto& line : trace.lines) out << line << '\n';
  out << '\n';
}

}  // namespace

int main() {
  std::filesystem::create_directories("Testing/Temporary");
  std::ofstream out("Testing/Temporary/issue2368_paired_readiness_scan.txt");
  const DeckRecipe base = baseline_recipe();
  const DeckRecipe dde = dde_recipe();
  const std::vector<std::string> labels{
      "strict-jit/go-first",
      "matchup-flex-jit/go-first",
      "no-discard-control/go-first",
      "strict-jit/go-second",
      "matchup-flex-jit/go-second",
      "no-discard-control/go-second",
  };
  constexpr std::uint64_t trials = 100000;
  constexpr std::uint64_t seed_base = 202608080000ULL;
  constexpr std::uint64_t stride = 1000003ULL;

  std::vector<Event> events;
  std::uint64_t total_exceptions = 0;
  double base_t2_sum = 0.0, dde_t2_sum = 0.0;
  double base_t3_sum = 0.0, dde_t3_sum = 0.0;
  double improve_t2_sum = 0.0, regress_t2_sum = 0.0;
  double improve_t3_sum = 0.0, regress_t3_sum = 0.0;

  out << "# Issue 2368 paired readiness scan\n"
      << "# exact per-game RNG reset; physical relabel [D,D,G,G,G,G,F,F,F] vs [G,G,G,G,G,G,F,F,F]\n";
  for (std::size_t si = 0; si < labels.size(); ++si) {
    const auto scenario_opt = scenario_by_label(labels.at(si));
    if (!scenario_opt) throw std::logic_error("registered scenario missing");
    const Scenario scenario = *scenario_opt;
    std::uint64_t n = 0;
    std::uint64_t base_t2 = 0, dde_t2 = 0, base_t3 = 0, dde_t3 = 0;
    std::uint64_t improve_t2 = 0, regress_t2 = 0, improve_t3 = 0, regress_t3 = 0;
    std::uint64_t exceptions = 0;
    for (std::uint64_t i = 0; i < trials; ++i) {
      const std::uint64_t seed = seed_base + stride * si + i;
      TrialOutcome b, d;
      try {
        b = run_one(scenario, base, seed);
        d = run_one(scenario, dde, seed);
      } catch (const std::exception& error) {
        ++exceptions;
        ++total_exceptions;
        remember(events, scenario.label, "exception", seed, error.what());
        continue;
      }
      ++n;
      base_t2 += b.ready_by_2;
      dde_t2 += d.ready_by_2;
      base_t3 += b.ready_by_3;
      dde_t3 += d.ready_by_3;
      if (!b.ready_by_2 && d.ready_by_2) ++improve_t2;
      if (b.ready_by_2 && !d.ready_by_2) {
        ++regress_t2;
        remember(events, scenario.label, "T2-regress", seed);
      }
      if (!b.ready_by_3 && d.ready_by_3) ++improve_t3;
      if (b.ready_by_3 && !d.ready_by_3) {
        ++regress_t3;
        remember(events, scenario.label, "T3-regress", seed);
      }
    }
    const double b2 = pct_local(base_t2, n), d2 = pct_local(dde_t2, n);
    const double b3 = pct_local(base_t3, n), d3 = pct_local(dde_t3, n);
    const double i2 = pct_local(improve_t2, n), r2 = pct_local(regress_t2, n);
    const double i3 = pct_local(improve_t3, n), r3 = pct_local(regress_t3, n);
    base_t2_sum += b2; dde_t2_sum += d2; base_t3_sum += b3; dde_t3_sum += d3;
    improve_t2_sum += i2; regress_t2_sum += r2; improve_t3_sum += i3; regress_t3_sum += r3;
    out << scenario.label
        << " T2 " << b2 << " -> " << d2
        << " improve=" << i2 << " regress=" << r2
        << " | T3 " << b3 << " -> " << d3
        << " improve=" << i3 << " regress=" << r3
        << " | exceptions=" << exceptions << '\n';
  }

  constexpr double scenario_count = 6.0;
  out << "MEAN T2 " << base_t2_sum / scenario_count << " -> " << dde_t2_sum / scenario_count
      << " improve=" << improve_t2_sum / scenario_count
      << " regress=" << regress_t2_sum / scenario_count << '\n';
  out << "MEAN T3 " << base_t3_sum / scenario_count << " -> " << dde_t3_sum / scenario_count
      << " improve=" << improve_t3_sum / scenario_count
      << " regress=" << regress_t3_sum / scenario_count << '\n';
  out << "TOTAL EXCEPTIONS=" << total_exceptions << "\n\n";

  for (const Event& event : events) {
    out << "### " << event.kind << " | " << event.scenario << " | seed " << event.seed;
    if (!event.error.empty()) out << " | " << event.error;
    out << '\n';
    const auto scenario = scenario_by_label(event.scenario);
    if (!scenario) continue;
    dump_trace(out, "BASE", *scenario, base, event.seed);
    dump_trace(out, "2DDE", *scenario, dde, event.seed);
  }
  out.close();
  return total_exceptions == 0 ? 0 : 3;
}
