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
  const std::string policy =
      read_text(root / "docs" / "T5_FAILURE_POLICY.md");
  const std::string matrix =
      read_text(root / "results" / "simulation_results.csv");

  // T4 remains the setup deadline while unresolved trials continue through the
  // diagnostic T5 horizon and publish separate late-recovery and failure fields:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/SIM-PLAN.md
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/T5_FAILURE_POLICY.md
  // https://github.com/FlareZ123/pokemon-sims/blob/main/results/simulation_results.csv
  // https://github.com/FlareZ123/pokemon-sims/issues/1491
  // https://github.com/FlareZ123/pokemon-sims/issues/3428
  expect_contains(plan, "through turn 5",
                  "SIM-PLAN.md omits the T5 diagnostic horizon.");
  expect_contains(plan, "T2, T3, and T4 readiness are setup success",
                  "SIM-PLAN.md lost the T4 success deadline.");
  expect_contains(plan,
                  "First readiness on T5 is recorded as diagnostic recovery and remains a setup failure",
                  "SIM-PLAN.md omits diagnostic T5 recovery.");
  expect_contains(plan, "setup failure",
                  "SIM-PLAN.md omits setup failure semantics.");
  expect_contains(policy, "A game that first becomes ready on T5",
                  "The canonical T5 policy changed.");
  expect_contains(matrix, "ready_by_t5_pct",
                  "The matrix omits cumulative T5 readiness.");
  expect_contains(matrix, "ready_on_t5_pct",
                  "The matrix omits diagnostic T5 readiness.");
  expect_contains(matrix, "setup_failure_pct",
                  "The matrix omits setup failure.");
  return 0;
}
