#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sim {

// Unified-test discovery requires one access block per standalone regression.
// This contract reads source text only, so no Engine access is necessary.
struct EngineTestAccess {};

}  // namespace sim

namespace {

void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

std::string read_source(const std::filesystem::path& path) {
  std::ifstream stream(path);
  if (!stream) throw std::runtime_error("Unable to open " + path.string());
  std::ostringstream buffer;
  buffer << stream.rdbuf();
  return buffer.str();
}

void test_opening_stage_owns_full_alias_chain() {
  const auto root = std::filesystem::path(__FILE__).parent_path().parent_path();
  const auto composition = root / "src" / "trace_engine_v2" / "composition";
  const auto legacy_stage = composition / "opening_legacy_stage.inc";
  const auto completion_stage = composition / "opening_state_completion_stage.inc";
  const auto source = read_source(legacy_stage);

  require(source.contains("#define begin_turn begin_turn_original"),
          "opening stage must preserve the begin_turn alias");
  require(source.contains("#define might_be_unseen might_be_unseen_empty_deck_original"),
          "opening stage must preserve the hidden-deck alias");
  require(source.contains("#include \"../part_003.inc\""),
          "opening stage must own the part_003 continuation");
  require(source.contains("#define ability_available_for_pokemon ability_available_for_pokemon_garbodor"),
          "opening stage must preserve the Garbotoxin ability alias");
  require(source.contains("#include \"../part_004.inc\""),
          "opening stage must own the part_004 continuation");
  require(source.contains("#include \"../part_005.inc\""),
          "opening stage must own the part_005 continuation");
  require(source.contains("#include \"../core/garbodor_lock_policy.inc\""),
          "opening stage must own the Garbotoxin policy continuation");
  require(source.contains("#undef might_be_unseen"),
          "opening stage must release the temporary hidden-deck alias");
  require(!source.contains("opening_state_completion_stage.inc"),
          "opening stage must not restore the forwarding completion include");
  require(!std::filesystem::exists(completion_stage),
          "forwarding-only opening_state_completion_stage.inc must stay removed");
}

}  // namespace

int main() {
  try {
    test_opening_stage_owns_full_alias_chain();
    std::cout << "Opening composition source contract passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
