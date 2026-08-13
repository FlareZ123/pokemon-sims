#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sim {

struct EngineTestAccess {};

}  // namespace sim

namespace {

std::filesystem::path repo_root() {
  const std::filesystem::path current = std::filesystem::current_path();
  if (std::filesystem::exists(current / "SIM-PLAN.md")) return current;
  if (std::filesystem::exists(current.parent_path() / "SIM-PLAN.md")) {
    return current.parent_path();
  }
  throw std::runtime_error("Unable to locate the repository root.");
}

std::string read_text(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("Unable to read " + path.string());
  std::ostringstream content;
  content << input.rdbuf();
  return content.str();
}

void expect_contains(const std::string& text, const std::string& expected,
                     const char* message) {
  if (text.find(expected) == std::string::npos) {
    throw std::runtime_error(message);
  }
}

}  // namespace

int main() {
  const std::filesystem::path root = repo_root();
  const std::string plan = read_text(root / "SIM-PLAN.md");
  const std::string readme = read_text(root / "README.md");
  const std::string driver =
      read_text(root / "src" / "trace_engine_v2" / "part_016.inc");
  const std::string matrix =
      read_text(root / "results" / "multi_deck_comparison.csv");
  const std::string results_readme =
      read_text(root / "results" / "README.md");

  // The executable registers two named recipes and --all-decks emits their paired
  // scenario matrix, while the generic variant_results.csv surface stays retired:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#registered-decks
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L250-L330
  // https://github.com/FlareZ123/pokemon-sims/blob/main/results/multi_deck_comparison.csv
  // https://github.com/FlareZ123/pokemon-sims/issues/1493
  // https://github.com/FlareZ123/pokemon-sims/issues/3429
  expect_contains(plan, "- `regidrago-shell`",
                  "SIM-PLAN.md omits the shell recipe inventory entry.");
  expect_contains(plan, "- `regidrago-pineco`",
                  "SIM-PLAN.md omits the Pineco recipe inventory entry.");
  expect_contains(plan, "`--all-decks` evaluates both registered recipes",
                  "SIM-PLAN.md omits the paired aggregate mode.");
  expect_contains(readme, "regidrago-shell",
                  "README.md omits the shell recipe.");
  expect_contains(readme, "regidrago-pineco",
                  "README.md omits the Pineco recipe.");
  expect_contains(driver, "--all-decks",
                  "The aggregate driver no longer exposes --all-decks.");
  expect_contains(matrix, "\"regidrago-shell\"",
                  "The paired matrix omits the shell recipe.");
  expect_contains(matrix, "\"regidrago-pineco\"",
                  "The paired matrix omits the Pineco recipe.");
  expect_contains(results_readme, "variant_results.csv",
                  "The retired generic variant artifact boundary is missing.");
  return 0;
}
