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

std::string section_text(const std::string& text, const std::string& heading) {
  const std::size_t begin = text.find(heading);
  if (begin == std::string::npos) {
    throw std::runtime_error("SIM-PLAN.md is missing section: " + heading);
  }
  const std::size_t next = text.find("\n## ", begin + heading.size());
  return text.substr(begin, next == std::string::npos ? std::string::npos
                                                     : next - begin);
}

}  // namespace

int main() {
  const std::filesystem::path root = repo_root();
  const std::string plan = read_text(root / "SIM-PLAN.md");
  const std::string policy =
      read_text(root / "docs" / "T5_FAILURE_POLICY.md");
  const std::string matrix =
      read_text(root / "results" / "simulation_results.csv");

  // Validate the semantic T4-success/T5-diagnostic contract inside its current
  // sections instead of pinning the test to historical prose:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/SIM-PLAN.md
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/T5_FAILURE_POLICY.md
  // https://github.com/FlareZ123/pokemon-sims/issues/3428
  const std::string lifecycle = section_text(plan, "## Trial lifecycle");
  expect_contains(lifecycle, "turn 5",
                  "SIM-PLAN.md omits the T5 diagnostic horizon.");
  expect_contains(lifecycle, "T2, T3, and T4",
                  "SIM-PLAN.md omits the T2-T4 success window.");
  expect_contains(lifecycle, "setup success",
                  "SIM-PLAN.md lost the setup-success deadline semantics.");
  expect_contains(lifecycle, "T5",
                  "SIM-PLAN.md omits diagnostic T5 recovery.");
  expect_contains(lifecycle, "diagnostic recovery",
                  "SIM-PLAN.md lost the T5 diagnostic-recovery semantics.");
  expect_contains(lifecycle, "setup failure",
                  "SIM-PLAN.md no longer states that T5-only readiness fails setup.");

  const std::string statistical = section_text(plan, "## Statistical output");
  expect_contains(statistical, "setup failure",
                  "SIM-PLAN.md omits setup-failure reporting.");

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
