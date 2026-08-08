#define REGIDRAGO_SIM_NO_MAIN
#include "src/regidrago_sim.cpp"

#include <array>
#include <fstream>
#include <map>
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

bool bit(const std::uint8_t mask, const int value) {
  return (mask & static_cast<std::uint8_t>(value)) != 0;
}

double pct_local(const std::uint64_t value, const std::uint64_t total) {
  return total == 0 ? 0.0 : 100.0 * static_cast<double>(value) /
      static_cast<double>(total);
}

struct Counts {
  std::uint64_t n{0};
  std::uint64_t base_t2_ready{0}, dde_t2_ready{0};
  std::uint64_t base_t3_ready{0}, dde_t3_ready{0};
  std::uint64_t t2_ready_improve{0}, t2_ready_regress{0};
  std::uint64_t t3_ready_improve{0}, t3_ready_regress{0};
  std::array<std::uint64_t, 4> base_t2_missing{};
  std::array<std::uint64_t, 4> dde_t2_missing{};
  std::array<std::uint64_t, 4> t2_improve{};
  std::array<std::uint64_t, 4> t2_regress{};
  std::uint64_t common_t2_unresolved{0};
  std::array<std::uint64_t, 4> base_t3_missing_common{};
  std::array<std::uint64_t, 4> dde_t3_missing_common{};
  std::array<std::uint64_t, 4> t3_improve_common{};
  std::array<std::uint64_t, 4> t3_regress_common{};
  std::uint64_t exceptions{0};
};

struct Event {
  std::string scenario;
  std::string kind;
  std::uint64_t seed{};
};

std::string axis_name(const int index) {
  static const std::array<const char*, 4> names{
      "vstar", "position", "energy", "payload"};
  return names.at(static_cast<std::size_t>(index));
}

void remember(std::vector<Event>& events, const std::string& scenario,
              const std::string& kind, const std::uint64_t seed) {
  const int same = static_cast<int>(std::count_if(
      events.begin(), events.end(), [&](const Event& event) {
        return event.scenario == scenario && event.kind == kind;
      }));
  if (same < 4) events.push_back({scenario, kind, seed});
}

void dump_trace(std::ofstream& out, const char* deck_name,
                const Scenario& scenario, const DeckRecipe& recipe,
                const std::uint64_t seed) {
  TraceLog trace{true, {}, {}};
  try {
    const TrialOutcome outcome = run_one(scenario, recipe, seed, &trace);
    out << "===== " << deck_name << " | " << scenario.label << " | " << seed
        << " | ready=" << outcome.first_ready_turn
        << " | t2mask=" << static_cast<int>(outcome.issue2368_t2_root_mask)
        << " | t3mask=" << static_cast<int>(outcome.issue2368_t3_root_mask)
        << " =====\n";
  } catch (const std::exception& error) {
    out << "===== " << deck_name << " | " << scenario.label << " | " << seed
        << " | EXCEPTION=" << error.what() << " =====\n";
  }
  for (const auto& line : trace.lines) out << line << '\n';
  out << '\n';
}

}  // namespace

int main() {
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
  constexpr std::array<int, 4> bits{1, 2, 4, 8};

  std::ofstream csv("paired_bug_hunt.csv");
  csv << "scenario,trials,base_t2_ready,dde_t2_ready,t2_ready_improve,t2_ready_regress,"
         "base_t3_ready,dde_t3_ready,t3_ready_improve,t3_ready_regress,common_t2_unresolved";
  for (const auto& axis : {"vstar", "position", "energy", "payload"}) {
    csv << ",base_t2_" << axis << ",dde_t2_" << axis
        << ",t2_" << axis << "_improve,t2_" << axis << "_regress"
        << ",base_t3_common_" << axis << ",dde_t3_common_" << axis
        << ",t3_common_" << axis << "_improve,t3_common_" << axis << "_regress";
  }
  csv << ",exceptions\n";

  std::vector<Event> events;
  std::uint64_t total_exceptions = 0;
  for (std::size_t si = 0; si < labels.size(); ++si) {
    const auto scenario_opt = scenario_by_label(labels.at(si));
    if (!scenario_opt) throw std::logic_error("registered scenario missing");
    const Scenario scenario = *scenario_opt;
    Counts c;
    for (std::uint64_t i = 0; i < trials; ++i) {
      const std::uint64_t seed = seed_base + stride * si + i;
      TrialOutcome b, d;
      try {
        b = run_one(scenario, base, seed);
        d = run_one(scenario, dde, seed);
      } catch (const std::exception& error) {
        ++c.exceptions;
        ++total_exceptions;
        remember(events, scenario.label,
                 std::string("exception:") + error.what(), seed);
        continue;
      }
      ++c.n;
      c.base_t2_ready += b.ready_by_2;
      c.dde_t2_ready += d.ready_by_2;
      c.base_t3_ready += b.ready_by_3;
      c.dde_t3_ready += d.ready_by_3;
      if (!b.ready_by_2 && d.ready_by_2) {
        ++c.t2_ready_improve;
      } else if (b.ready_by_2 && !d.ready_by_2) {
        ++c.t2_ready_regress;
        remember(events, scenario.label, "t2-ready-regress", seed);
      }
      if (!b.ready_by_3 && d.ready_by_3) {
        ++c.t3_ready_improve;
      } else if (b.ready_by_3 && !d.ready_by_3) {
        ++c.t3_ready_regress;
        remember(events, scenario.label, "t3-ready-regress", seed);
      }

      if (!b.issue2368_t2_recorded || !d.issue2368_t2_recorded) {
        throw std::logic_error("T2 semantic snapshot missing");
      }
      for (int ai = 0; ai < 4; ++ai) {
        const bool bm = bit(b.issue2368_t2_root_mask, bits.at(ai));
        const bool dm = bit(d.issue2368_t2_root_mask, bits.at(ai));
        c.base_t2_missing.at(ai) += bm;
        c.dde_t2_missing.at(ai) += dm;
        if (bm && !dm) {
          ++c.t2_improve.at(ai);
        } else if (!bm && dm) {
          ++c.t2_regress.at(ai);
          remember(events, scenario.label,
                   "t2-" + axis_name(ai) + "-regress", seed);
        }
      }

      if (!b.ready_by_2 && !d.ready_by_2) {
        ++c.common_t2_unresolved;
        if (!b.issue2368_t3_recorded || !d.issue2368_t3_recorded) {
          remember(events, scenario.label, "missing-t3-common-snapshot", seed);
          continue;
        }
        for (int ai = 0; ai < 4; ++ai) {
          const bool bm = bit(b.issue2368_t3_root_mask, bits.at(ai));
          const bool dm = bit(d.issue2368_t3_root_mask, bits.at(ai));
          c.base_t3_missing_common.at(ai) += bm;
          c.dde_t3_missing_common.at(ai) += dm;
          if (bm && !dm) {
            ++c.t3_improve_common.at(ai);
          } else if (!bm && dm) {
            ++c.t3_regress_common.at(ai);
            remember(events, scenario.label,
                     "t3-common-" + axis_name(ai) + "-regress", seed);
          }
        }
      }
    }

    csv << scenario.label << ',' << c.n
        << ',' << pct_local(c.base_t2_ready, c.n)
        << ',' << pct_local(c.dde_t2_ready, c.n)
        << ',' << pct_local(c.t2_ready_improve, c.n)
        << ',' << pct_local(c.t2_ready_regress, c.n)
        << ',' << pct_local(c.base_t3_ready, c.n)
        << ',' << pct_local(c.dde_t3_ready, c.n)
        << ',' << pct_local(c.t3_ready_improve, c.n)
        << ',' << pct_local(c.t3_ready_regress, c.n)
        << ',' << pct_local(c.common_t2_unresolved, c.n);
    for (int ai = 0; ai < 4; ++ai) {
      csv << ',' << pct_local(c.base_t2_missing.at(ai), c.n)
          << ',' << pct_local(c.dde_t2_missing.at(ai), c.n)
          << ',' << pct_local(c.t2_improve.at(ai), c.n)
          << ',' << pct_local(c.t2_regress.at(ai), c.n)
          << ',' << pct_local(c.base_t3_missing_common.at(ai), c.common_t2_unresolved)
          << ',' << pct_local(c.dde_t3_missing_common.at(ai), c.common_t2_unresolved)
          << ',' << pct_local(c.t3_improve_common.at(ai), c.common_t2_unresolved)
          << ',' << pct_local(c.t3_regress_common.at(ai), c.common_t2_unresolved);
    }
    csv << ',' << c.exceptions << '\n';
  }
  csv.close();

  std::ofstream seed_csv("paired_bug_hunt_seeds.csv");
  seed_csv << "scenario,kind,seed\n";
  for (const Event& event : events) {
    seed_csv << event.scenario << ',' << event.kind << ',' << event.seed << '\n';
  }
  seed_csv.close();

  std::ofstream traces("paired_bug_hunt_traces.txt");
  for (const Event& event : events) {
    if (event.kind.find("regress") == std::string::npos &&
        event.kind.find("exception") == std::string::npos) {
      continue;
    }
    const auto scenario = scenario_by_label(event.scenario);
    if (!scenario) continue;
    traces << "### EVENT " << event.kind << "\n";
    dump_trace(traces, "BASE", *scenario, base, event.seed);
    dump_trace(traces, "2DDE", *scenario, dde, event.seed);
  }
  return total_exceptions == 0 ? 0 : 3;
}
