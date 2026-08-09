#define main connector_screen_previous_main
#include "regidrago_connector_swap_screen.cpp"
#undef main

#include <set>

namespace {

struct Package {
  int treasure{};
  int quick{};
  int vip{};
  int ultra{};
  int incense{};
  int communication{};
  sim::DeckRecipe recipe;
  std::string id;
};

struct Aggregate {
  Package package;
  std::array<double, 7> axis_mean{}; // ready, regi, vstar, active, energy, active-energy, payload
  double score{};
};

Package make_package(int treasure, int quick, int vip, int ultra,
                     int incense, int communication) {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  adjust(recipe, sim::Card::MysteriousTreasure, -4);
  adjust(recipe, sim::Card::QuickBall, -3);
  for (const auto [card, count] : std::array{
           std::pair{sim::Card::MysteriousTreasure, treasure},
           std::pair{sim::Card::QuickBall, quick},
           std::pair{sim::Card::BattleVipPass, vip},
           std::pair{sim::Card::UltraBall, ultra},
           std::pair{sim::Card::EvolutionIncense, incense},
           std::pair{sim::Card::PokemonCommunication, communication}}) {
    adjust(recipe, card, count);
  }
  std::ostringstream id;
  id << "MT" << treasure << " QB" << quick << " VIP" << vip
     << " UB" << ultra << " EI" << incense << " PC" << communication;
  std::string error;
  if (!sim::validate_recipe({id.str(), recipe}, &error)) {
    throw std::logic_error(id.str() + ": " + error);
  }
  return {treasure, quick, vip, ultra, incense, communication, std::move(recipe), id.str()};
}

std::vector<Package> packages() {
  std::vector<Package> out;
  for (int mt = 0; mt <= 4; ++mt)
    for (int qb = 0; qb <= 4; ++qb)
      for (int vip = 0; vip <= 4; ++vip)
        for (int ub = 0; ub <= 4; ++ub)
          for (int ei = 0; ei <= 4; ++ei)
            for (int pc = 0; pc <= 4; ++pc)
              if (mt + qb + vip + ub + ei + pc == 7)
                out.push_back(make_package(mt, qb, vip, ub, ei, pc));
  return out;
}

std::array<double, 7> means_for(const Package& package,
                                const std::vector<sim::Scenario>& scenarios,
                                std::uint64_t trials,
                                std::uint64_t seed_base,
                                std::ostringstream* rows,
                                std::ostringstream* errors) {
  std::array<double, 7> sums{};
  std::uint64_t cells = 0;
  constexpr std::uint64_t trial_stride = 104729ULL;
  constexpr std::uint64_t scenario_stride = 1000000007ULL;
  for (std::size_t si = 0; si < scenarios.size(); ++si) {
    sim::Scenario scenario = scenarios[si];
    scenario.max_turn = 4;
    std::array<Stats, 3> stats{};
    std::map<std::string, std::uint64_t> scenario_errors;
    for (std::uint64_t t = 0; t < trials; ++t) {
      try {
        std::mt19937_64 rng(seed_base + scenario_stride * si + trial_stride * t);
        sim::Engine engine(scenario, package.recipe, rng);
        const auto snapshots = sim::EngineTestAccess::run_checkpoints(engine);
        for (std::size_t cp = 0; cp < 3; ++cp) record(stats[cp], snapshots[cp]);
      } catch (const std::exception& e) {
        ++scenario_errors[e.what()];
      }
    }
    for (const auto& [message, count] : scenario_errors) {
      if (errors) *errors << quoted(package.id) << ',' << quoted(scenario.label)
                          << ',' << quoted(message) << ',' << count << '\n';
    }
    for (std::size_t cp = 0; cp < 3; ++cp) {
      const Stats& s = stats[cp];
      const auto values = std::array{
          pct(s.ready, s.trials), pct(s.regi_in_play, s.trials),
          pct(s.vstar_in_play, s.trials), pct(s.active_vstar, s.trials),
          pct(s.energy_ready_any_regi, s.trials), pct(s.active_energy_ready, s.trials),
          pct(s.payload_ready, s.trials)};
      for (std::size_t a = 0; a < values.size(); ++a) sums[a] += values[a];
      ++cells;
      if (rows) {
        *rows << quoted(package.id) << ',' << quoted(scenario.label) << ',' << (cp + 2)
              << ',' << s.trials;
        for (double value : values) *rows << ',' << value;
        *rows << ',' << pct(s.k1, s.trials) << '\n';
      }
    }
  }
  for (double& value : sums) value /= static_cast<double>(cells);
  return sums;
}

} // namespace

int main(int argc, char** argv) {
  const std::uint64_t screen_trials = argc >= 2 ? std::stoull(argv[1]) : 500ULL;
  const std::uint64_t confirm_trials = argc >= 3 ? std::stoull(argv[2]) : 10000ULL;
  const std::filesystem::path out = argc >= 4 ? argv[3] : "results/generic-connector-mix";

  const std::array core_labels{
      "strict-jit/go-first", "strict-jit/go-second",
      "matchup-flex-jit/go-first", "matchup-flex-jit/go-second"};
  std::vector<sim::Scenario> core;
  for (const char* label : core_labels) {
    const auto found = sim::scenario_by_label(label);
    if (!found) throw std::logic_error(std::string("missing scenario: ") + label);
    core.push_back(*found);
  }

  auto all_packages = packages();
  const auto baseline_it = std::find_if(all_packages.begin(), all_packages.end(), [](const Package& p) {
    return p.treasure == 4 && p.quick == 3 && p.vip == 0 && p.ultra == 0 &&
           p.incense == 0 && p.communication == 0;
  });
  if (baseline_it == all_packages.end()) throw std::logic_error("baseline package missing");

  constexpr std::uint64_t screen_seed = 2026080901ULL;
  const auto baseline_mean = means_for(*baseline_it, core, screen_trials, screen_seed, nullptr, nullptr);

  std::vector<Aggregate> aggs;
  aggs.reserve(all_packages.size());
  std::ostringstream screen_errors;
  screen_errors << "package,scenario,error,count\n";
  for (const Package& package : all_packages) {
    const auto means = means_for(package, core, screen_trials, screen_seed, nullptr, &screen_errors);
    double min_axis_delta = 1e9;
    for (std::size_t a = 1; a < means.size(); ++a)
      min_axis_delta = std::min(min_axis_delta, means[a] - baseline_mean[a]);
    const double ready_delta = means[0] - baseline_mean[0];
    // Maximin penalty: headline readiness cannot compensate for materially weakening another setup axis.
    const double score = ready_delta + 2.0 * std::min(0.0, min_axis_delta);
    aggs.push_back({package, means, score});
  }

  std::ostringstream ranking;
  ranking << "package,mt,qb,vip,ub,ei,pc,ready_delta,regi_delta,vstar_delta,active_delta,energy_delta,active_energy_delta,payload_delta,score\n";
  std::sort(aggs.begin(), aggs.end(), [](const Aggregate& a, const Aggregate& b) { return a.score > b.score; });
  for (const Aggregate& a : aggs) {
    ranking << quoted(a.package.id) << ',' << a.package.treasure << ',' << a.package.quick << ','
            << a.package.vip << ',' << a.package.ultra << ',' << a.package.incense << ','
            << a.package.communication;
    for (std::size_t axis = 0; axis < a.axis_mean.size(); ++axis)
      ranking << ',' << (a.axis_mean[axis] - baseline_mean[axis]);
    ranking << ',' << a.score << '\n';
  }

  // Confirm a small union: top 5 maximin score, top 5 raw readiness, top 5 VSTAR gain.
  std::set<std::string> selected_ids{baseline_it->id};
  for (std::size_t i = 0; i < std::min<std::size_t>(5, aggs.size()); ++i) selected_ids.insert(aggs[i].package.id);
  auto by_ready = aggs;
  std::sort(by_ready.begin(), by_ready.end(), [&](const Aggregate& a, const Aggregate& b) {
    return a.axis_mean[0] > b.axis_mean[0];
  });
  for (std::size_t i = 0; i < std::min<std::size_t>(5, by_ready.size()); ++i) selected_ids.insert(by_ready[i].package.id);
  auto by_vstar = aggs;
  std::sort(by_vstar.begin(), by_vstar.end(), [&](const Aggregate& a, const Aggregate& b) {
    return a.axis_mean[2] > b.axis_mean[2];
  });
  for (const Aggregate& a : by_vstar) {
    if (a.axis_mean[0] <= baseline_mean[0]) continue;
    selected_ids.insert(a.package.id);
    if (selected_ids.size() >= 16U) break;
  }

  std::vector<Package> selected;
  for (const Package& package : all_packages)
    if (selected_ids.contains(package.id)) selected.push_back(package);

  std::ostringstream confirm_rows;
  confirm_rows << "package,scenario,turn,completed_trials,ready_pct,regi_pct,vstar_pct,active_pct,energy_pct,active_energy_pct,payload_pct,k1_pct\n";
  std::ostringstream confirm_errors;
  confirm_errors << "package,scenario,error,count\n";
  const auto all_scenarios = sim::all_scenarios();
  constexpr std::uint64_t confirm_seed = 2026080902ULL;
  for (const Package& package : selected) {
    means_for(package, all_scenarios, confirm_trials, confirm_seed, &confirm_rows, &confirm_errors);
    std::cout << "confirmed " << package.id << '\n';
  }

  std::filesystem::create_directories(out);
  sim::write_atomic(out / "screen_rankings.csv", ranking.str());
  sim::write_atomic(out / "screen_errors.csv", screen_errors.str());
  sim::write_atomic(out / "confirm_checkpoints.csv", confirm_rows.str());
  sim::write_atomic(out / "confirm_errors.csv", confirm_errors.str());
  std::ostringstream manifest;
  manifest << "frozen_non_connector_cards=53\n"
           << "generic_connector_slots=7\n"
           << "families=Mysterious Treasure,Quick Ball,Battle VIP Pass,Ultra Ball,Evolution Incense,Pokemon Communication\n"
           << "packages=" << all_packages.size() << "\n"
           << "screen_trials=" << screen_trials << "\n"
           << "confirmed_packages=" << selected.size() << "\n"
           << "confirm_trials=" << confirm_trials << "\n";
  sim::write_atomic(out / "manifest.txt", manifest.str());
  return 0;
}
