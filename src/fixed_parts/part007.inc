#if defined(_WIN32)
    handle_ = ::CreateFileW(lock_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle_ == INVALID_HANDLE_VALUE) throw std::runtime_error("cannot lock output");
#else
    descriptor_ = ::open(lock_path.c_str(), O_CREAT | O_RDWR, 0644);
    if (descriptor_ < 0 || ::flock(descriptor_, LOCK_EX) != 0) {
      if (descriptor_ >= 0) ::close(descriptor_);
      throw std::runtime_error("cannot lock output");
    }
#endif
  }
  ~OutputLock() {
#if defined(_WIN32)
    if (handle_ != INVALID_HANDLE_VALUE) ::CloseHandle(handle_);
#else
    if (descriptor_ >= 0) { ::flock(descriptor_, LOCK_UN); ::close(descriptor_); }
#endif
  }
  OutputLock(const OutputLock&) = delete;
  OutputLock& operator=(const OutputLock&) = delete;
 private:
#if defined(_WIN32)
  HANDLE handle_{INVALID_HANDLE_VALUE};
#else
  int descriptor_{-1};
#endif
};

void write_atomic(const std::filesystem::path& output, const std::string& content) {
  if (!output.parent_path().empty()) std::filesystem::create_directories(output.parent_path());
  OutputLock lock(output);
  const std::filesystem::path temporary = output.string() + ".tmp." + std::to_string(
#if defined(_WIN32)
      static_cast<unsigned long long>(::GetCurrentProcessId())
#else
      static_cast<unsigned long long>(::getpid())
#endif
  );
  {
    std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
    if (!stream) throw std::runtime_error("cannot write temporary output");
    stream << content;
    stream.flush();
    if (!stream) throw std::runtime_error("cannot flush temporary output");
  }
#if defined(_WIN32)
  if (!::MoveFileExW(temporary.c_str(), output.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) throw std::runtime_error("cannot replace output");
#else
  std::error_code error;
  std::filesystem::rename(temporary, output, error);
  if (error) { std::filesystem::remove(temporary); throw std::runtime_error("cannot replace output"); }
#endif
}

bool self_test() {
  const std::vector<Card> two_grass = [] { return std::vector<Card>{Card::Grass, Card::Grass}; }();
  if (two_grass.size() != 2 || two_grass[0] != Card::Grass || two_grass[1] != Card::Grass) return false;
  if (is_rule_box_pokemon(Card::Oricorio)) return false;
  if (!is_rule_box_pokemon(Card::TapuLeleGX)) return false;
  if (!is_item(Card::ForestSealStone)) return false;
  if (!is_payload(Card::MegaDragonite) || is_payload(Card::Dipplin)) return false;
  const int total = std::accumulate(kDeckRecipe.begin(), kDeckRecipe.end(), 0, [](const int sum, const auto& entry) { return sum + entry.second; });
  return total == 60;
}

}  // namespace sim

int main(int argc, char** argv) {
  std::uint64_t trials = 100000;
  std::uint64_t variant_trials = 25000;
  std::uint64_t seed = 20260705;
  std::string baseline_output = "results/simulation_results.csv";
  std::string variants_output = "results/variant_results.csv";
  bool run_self_test = false;

  for (int i = 1; i < argc; ++i) {
    const std::string argument = argv[i];
    if (argument == "--trials" && i + 1 < argc) trials = std::stoull(argv[++i]);
    else if (argument == "--variant-trials" && i + 1 < argc) variant_trials = std::stoull(argv[++i]);
    else if (argument == "--seed" && i + 1 < argc) seed = std::stoull(argv[++i]);
    else if (argument == "--out" && i + 1 < argc) baseline_output = argv[++i];
    else if (argument == "--variants-out" && i + 1 < argc) variants_output = argv[++i];
    else if (argument == "--self-test") run_self_test = true;
    else if (argument == "--help") {
      std::cout << "Usage: regidrago_sim [--trials N] [--variant-trials N] [--seed N] [--out PATH] [--variants-out PATH] [--self-test]\n";
      return 0;
    }
  }

  if (run_self_test) return sim::self_test() ? 0 : 1;

  try {
    const sim::DeckRecipe baseline = sim::baseline_recipe();
    std::ostringstream output;
    output << "scenario,trials,ready_by_t2_pct,ready_by_t2_se_pp,ready_by_t3_pct,ready_by_t3_se_pp,ready_by_t4_pct,ready_by_t4_se_pp,opening_regidrago_pct,fss_used_pct,steven_used_pct,blender_used_pct,mean_mulligans\n";
    std::cout << std::fixed << std::setprecision(3);
    const auto all_scenarios = sim::scenarios();
    for (std::size_t index = 0; index < all_scenarios.size(); ++index) {
      const sim::Aggregate a = sim::simulate(all_scenarios[index], baseline, trials, seed + 104729U * static_cast<std::uint64_t>(index));
      const double p2 = sim::pct(a.by2, a.trials);
      const double p3 = sim::pct(a.by3, a.trials);
      const double p4 = sim::pct(a.by4, a.trials);
      output << sim::csv(all_scenarios[index].label) << ',' << a.trials << ',' << p2 << ',' << sim::standard_error_pp(a.by2, a.trials) << ','
             << p3 << ',' << sim::standard_error_pp(a.by3, a.trials) << ',' << p4 << ',' << sim::standard_error_pp(a.by4, a.trials) << ','
             << sim::pct(a.opening_regidrago, a.trials) << ',' << sim::pct(a.fss, a.trials) << ',' << sim::pct(a.steven, a.trials) << ','
             << sim::pct(a.blender, a.trials) << ',' << (a.mulligans / static_cast<double>(a.trials)) << '\n';
      std::cout << std::left << std::setw(48) << all_scenarios[index].label << " T2=" << std::setw(7) << p2 << "% T3=" << std::setw(7) << p3 << "% T4=" << p4 << "%\n";
    }

    std::ostringstream variants;
    variants << "variant,scenario,trials,ready_by_t2_pct,ready_by_t3_pct,ready_by_t4_pct,delta_t2_pp_vs_baseline,delta_t3_pp_vs_baseline,delta_t4_pp_vs_baseline\n";
    const std::array<sim::Scenario, 4> screens{{
        {"strict-jit/go-first", sim::DciProfile::StrictJit, sim::LockMode::None, true},
        {"strict-jit/go-second", sim::DciProfile::StrictJit, sim::LockMode::None, false},
        {"matchup-flex-jit/go-first", sim::DciProfile::MatchupFlexJit, sim::LockMode::None, true},
        {"strict-jit-turn2-item-lock/go-second", sim::DciProfile::StrictJit, sim::LockMode::TurnTwoItem, false},
    }};
    const auto all_variants = sim::variants();
    for (std::size_t screen = 0; screen < screens.size(); ++screen) {
      const sim::Aggregate control = sim::simulate(screens[screen], baseline, variant_trials, seed + 1000003U * static_cast<std::uint64_t>(screen));
      for (const sim::Variant& variant : all_variants) {
        const sim::Aggregate result = sim::simulate(screens[screen], variant.recipe, variant_trials, seed + 1000003U * static_cast<std::uint64_t>(screen));
        variants << sim::csv(variant.label) << ',' << sim::csv(screens[screen].label) << ',' << result.trials << ','
                 << sim::pct(result.by2, result.trials) << ',' << sim::pct(result.by3, result.trials) << ',' << sim::pct(result.by4, result.trials) << ','
                 << (sim::pct(result.by2, result.trials) - sim::pct(control.by2, control.trials)) << ','
                 << (sim::pct(result.by3, result.trials) - sim::pct(control.by3, control.trials)) << ','
                 << (sim::pct(result.by4, result.trials) - sim::pct(control.by4, control.trials)) << '\n';
      }
    }

    sim::write_atomic(baseline_output, output.str());
    sim::write_atomic(variants_output, variants.str());
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
