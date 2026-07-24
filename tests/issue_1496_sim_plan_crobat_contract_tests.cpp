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

void expect_omits(const std::string& text, const std::string& stale,
                  const char* message) {
  if (text.find(stale) != std::string::npos) {
    throw std::runtime_error(message);
  }
}

}  // namespace

int main() {
  const std::filesystem::path root = repo_root();
  const std::string plan = read_text(root / "SIM-PLAN.md");
  const std::string readme = read_text(root / "README.md");
  const std::string cli =
      read_text(root / "src" / "trace_engine_v2" / "part_016.inc");
  const std::string report =
      read_text(root / "docs" / "CROBAT_MODEL_REPORT.md");

  // The supported swap surface is the unregistered Crobat modeling registry.
  // It remains distinct from registered decks and the retired generic variant file:
  // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
  // CLI contract: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L250-L330
  // Reproduction contract: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#L72-L86
  // Generated report: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md
  // Completed modeling scope: https://github.com/FlareZ123/pokemon-sims/issues/1394
  // Confirmed documentation bug: https://github.com/FlareZ123/pokemon-sims/issues/1496
  // Validated correction: https://github.com/FlareZ123/pokemon-sims/pull/1510
  expect_contains(plan, "`--model-crobat`",
                  "SIM-PLAN.md omits the Crobat matrix command.");
  expect_contains(plan, "`--model-variant`",
                  "SIM-PLAN.md omits the Crobat trace command.");
  expect_contains(plan, "modeling-only Crobat V",
                  "SIM-PLAN.md does not identify the modeling-only registry.");
  expect_contains(plan, "retired generic `variant_results.csv`",
                  "SIM-PLAN.md lost the retired generic-screen boundary.");
  expect_contains(plan, "`results/crobat_variant_model.csv`",
                  "SIM-PLAN.md omits the source-bound Crobat artifact.");
  expect_contains(plan, "Registered-deck scenario probabilities",
                  "SIM-PLAN.md lost the merged registered-deck metrics contract.");
  expect_omits(plan, "Matched-seed card-swap deltas remain a future extension",
               "SIM-PLAN.md still describes implemented Crobat deltas as future work.");
  expect_contains(readme, "--model-crobat",
                  "README.md lost the Crobat matrix command.");
  expect_contains(cli, "--model-variant",
                  "The CLI lost the one-variant trace option.");
  expect_contains(report, "temporary Crobat V swaps",
                  "The generated Crobat report lost its modeling scope.");
  return 0;
}
