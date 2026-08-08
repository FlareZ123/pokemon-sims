#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {
  static bool alive() { return true; }
};
}  // namespace sim

namespace {
namespace fs = std::filesystem;

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

fs::path source_root() {
  fs::path directory = fs::current_path();
  for (int depth = 0; depth < 4; ++depth) {
    if (fs::exists(directory / "src" / "regidrago_sim.cpp")) return directory;
    if (!directory.has_parent_path()) break;
    directory = directory.parent_path();
  }
  throw std::runtime_error("Could not locate repository source root");
}

std::string read_text(const fs::path& path) {
  std::ifstream input(path, std::ios::binary);
  expect(input.good(), "Could not read experiment source file");
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

void write_atomic(const fs::path& path, const std::string& text) {
  const fs::path temporary = path.string() + ".issue2368.tmp";
  {
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    expect(output.good(), "Could not write experiment temporary file");
    output << text;
    expect(output.good(), "Could not flush experiment temporary file");
  }
  fs::rename(temporary, path);
}

std::string replace_once(std::string source, const std::string& before,
                         const std::string& after) {
  const std::size_t offset = source.find(before);
  expect(offset != std::string::npos, "Experiment patch anchor disappeared");
  expect(source.find(before, offset + 1) == std::string::npos,
         "Experiment patch anchor became ambiguous");
  source.replace(offset, before.size(), after);
  return source;
}

void print_file(const fs::path& path) {
  std::cout << "\n===== " << path.filename().string() << " =====\n"
            << read_text(path) << '\n';
}

class SourceRestore {
 public:
  SourceRestore(fs::path path, std::string original)
      : path_(std::move(path)), original_(std::move(original)) {}
  ~SourceRestore() {
    try {
      write_atomic(path_, original_);
    } catch (...) {
    }
  }
 private:
  fs::path path_;
  std::string original_;
};

void run_postmerge_paired_experiment() {
  // This is an ephemeral CI-only measurement for the already-merged #2368 fix.
  // DDE supplies two Energy of every type while attached to Dragon Pokemon, so
  // readiness and Energy bottlenecks use semantic Apex payment rather than raw
  // Basic-Energy counters:
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Energy and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1 / strict-JIT / earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
  // Acceptance requirement: https://github.com/FlareZ123/pokemon-sims/issues/2368
  expect(sim::EngineTestAccess::alive(), "Experiment test access failed");
  const fs::path root = source_root();
  const fs::path part2 = root / "src" / "trace_engine_v2" / "part_002.inc";
  const fs::path part14 = root / "src" / "trace_engine_v2" / "part_014c.inc";
  const std::string original2 = read_text(part2);
  const std::string original14 = read_text(part14);
  SourceRestore restore2(part2, original2);
  SourceRestore restore14(part14, original14);

  const std::string outcome_before =
      "  bool t2_eligible_held_vstar_missed_active{false};\n"
      "  bool t2_eligible_missed_active{false};\n"
      "};";
  const std::string outcome_after =
      "  bool t2_eligible_held_vstar_missed_active{false};\n"
      "  bool t2_eligible_missed_active{false};\n"
      "  std::uint8_t issue2368_t2_root_mask{0};\n"
      "  std::uint8_t issue2368_t3_root_mask{0};\n"
      "  bool issue2368_t2_recorded{false};\n"
      "  bool issue2368_t3_recorded{false};\n"
      "};";
  write_atomic(part2, replace_once(original2, outcome_before, outcome_after));

  const std::string ready_before =
      "    const bool payload_is_ready = payload_ready();\n"
      "    const bool ready = state_.turn >= 2 && active_vstar && energy_ready &&\n"
      "        payload_is_ready;\n"
      "    if (!ready) {";
  const std::string ready_after = R"CPP(    const bool payload_is_ready = payload_ready();
    const bool ready = state_.turn >= 2 && active_vstar && energy_ready &&
        payload_is_ready;
    bool vstar_in_play = false;
    bool any_regi_energy_ready = false;
    bool any_vstar_energy_ready = false;
    const auto inspect_issue2368_line = [&](const Pokemon& pokemon) {
      if (pokemon.card != Card::RegidragoV && pokemon.card != Card::RegidragoVstar) return;
      const bool pays = pays_apex_energy_cost(pokemon);
      any_regi_energy_ready = any_regi_energy_ready || pays;
      if (pokemon.card == Card::RegidragoVstar) {
        vstar_in_play = true;
        any_vstar_energy_ready = any_vstar_energy_ready || pays;
      }
    };
    if (state_.active) inspect_issue2368_line(*state_.active);
    for (const Pokemon& pokemon : state_.bench) inspect_issue2368_line(pokemon);
    const bool misplaced_vstar = vstar_in_play && !active_vstar;
    const bool semantic_energy_ready = active_vstar ? energy_ready :
        (vstar_in_play ? any_vstar_energy_ready : any_regi_energy_ready);
    std::uint8_t issue2368_mask = 0;
    if (!vstar_in_play) issue2368_mask |= 1U;
    if (misplaced_vstar) issue2368_mask |= 2U;
    if (!semantic_energy_ready) issue2368_mask |= 4U;
    if (!payload_is_ready) issue2368_mask |= 8U;
    if (state_.turn == 2) {
      outcome_.issue2368_t2_root_mask = issue2368_mask;
      outcome_.issue2368_t2_recorded = true;
    } else if (state_.turn == 3) {
      outcome_.issue2368_t3_root_mask = issue2368_mask;
      outcome_.issue2368_t3_recorded = true;
    }
    if (!ready) {)CPP";
  write_atomic(part14, replace_once(original14, ready_before, ready_after));

  const fs::path old_cwd = fs::current_path();
  fs::current_path(root);
  const int fetch_status = std::system(
      "git fetch origin fix/2368-dde-sequencing:refs/remotes/origin/fix/2368-dde-sequencing");
  expect(fetch_status == 0, "Could not fetch reviewed issue-2368 paired scanner");
  const int show_status = std::system(
      "git show origin/fix/2368-dde-sequencing:scripts/issue2368_paired_hunt.cpp > issue2368_paired_hunt.cpp");
  expect(show_status == 0, "Could not materialize reviewed issue-2368 paired scanner");
  const int compile_status = std::system(
      "g++ -std=c++20 -O2 -I. issue2368_paired_hunt.cpp -o issue2368_paired_hunt");
  expect(compile_status == 0, "Could not compile issue-2368 paired scanner");
  const int scan_status = std::system("./issue2368_paired_hunt");
  expect(scan_status == 0, "Issue-2368 paired scanner failed or saw an exception");

  print_file(root / "paired_bug_hunt.csv");
  print_file(root / "paired_bug_hunt_seeds.csv");
  print_file(root / "paired_bug_hunt_traces.txt");
  fs::current_path(old_cwd);
}

}  // namespace

int main() {
  try {
    run_postmerge_paired_experiment();
    std::cout << "Issue 2368 post-merge paired DDE experiment passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
