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

void test_opening_stage_owns_member_includes_directly() {
  const auto root = std::filesystem::path(__FILE__).parent_path().parent_path();
  const auto composition = root / "src" / "trace_engine_v2" / "composition";
  const auto legacy_stage = composition / "opening_legacy_stage.inc";
  const auto completion_stage = composition / "opening_state_completion_stage.inc";
  const auto source = read_source(legacy_stage);

  constexpr std::string_view member_wrapper =
      "#define REGIDRAGO_ENGINE_MEMBER_WRAPPER\n"
      "#include \"core/opening/opening_member_impl.inc\"\n"
      "#undef REGIDRAGO_ENGINE_MEMBER_WRAPPER";
  constexpr std::string_view member_scope =
      "#define REGIDRAGO_ENGINE_MEMBER_SCOPE\n"
      "#include \"core/opening/opening_state_ability_resolution.inc\"\n"
      "#undef REGIDRAGO_ENGINE_MEMBER_SCOPE";

  require(source.contains(member_wrapper),
          "opening_legacy_stage.inc must own the member-wrapper include boundary");
  require(source.contains(member_scope),
          "opening_legacy_stage.inc must own the member-scope include boundary");
  require(!source.contains("opening_state_completion_stage.inc"),
          "opening_legacy_stage.inc must not restore the forwarding completion include");
  require(!std::filesystem::exists(completion_stage),
          "forwarding-only opening_state_completion_stage.inc must stay removed");
}

}  // namespace

int main() {
  try {
    test_opening_stage_owns_member_includes_directly();
    std::cout << "Opening composition source contract passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
