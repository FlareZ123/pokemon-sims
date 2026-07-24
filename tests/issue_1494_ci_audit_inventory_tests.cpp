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
  if (std::filesystem::exists(current / ".github" / "workflows" / "ci.yml")) {
    return current;
  }
  if (std::filesystem::exists(current.parent_path() / ".github" / "workflows" /
                              "ci.yml")) {
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

std::size_t count_occurrences(const std::string& text,
                              const std::string& needle) {
  std::size_t count = 0;
  std::size_t position = 0;
  while ((position = text.find(needle, position)) != std::string::npos) {
    ++count;
    position += needle.size();
  }
  return count;
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
  const std::string workflow =
      read_text(root / ".github" / "workflows" / "ci.yml");
  const std::string audit_status = read_text(root / "docs" / "AUDIT_STATUS.md");
  const std::string simulator_audit =
      read_text(root / "docs" / "SIMULATOR_AUDIT.md");

  // The permanent Release lane owns eight shell traces, three Pineco traces,
  // and byte comparisons for both source-bound matrices:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml#L20-L166
  // https://github.com/FlareZ123/pokemon-sims/issues/1494
  if (count_occurrences(workflow,
                        "./build/regidrago_sim --simulate-this") != 11) {
    throw std::runtime_error("The permanent CI trace command count drifted.");
  }
  expect_contains(workflow, "Run eight independent simulate-this audits",
                  "The shell trace audit group is missing.");
  expect_contains(workflow, "Run three reviewed Pineco setup audits",
                  "The Pineco trace audit group is missing.");
  expect_contains(workflow, "Verify canonical shell matrix",
                  "The canonical matrix comparison is missing.");
  expect_contains(workflow, "Verify committed paired matrix",
                  "The paired matrix comparison is missing.");
  expect_contains(audit_status, "eight shell `--simulate-this` audits",
                  "AUDIT_STATUS.md understates shell trace coverage.");
  expect_contains(audit_status, "three registered-Pineco audits",
                  "AUDIT_STATUS.md understates Pineco trace coverage.");
  expect_contains(simulator_audit, "eight readable shell `--simulate-this` audits",
                  "SIMULATOR_AUDIT.md understates shell trace coverage.");
  expect_contains(simulator_audit, "three registered-Pineco audits",
                  "SIMULATOR_AUDIT.md understates Pineco trace coverage.");
  return 0;
}
