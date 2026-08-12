#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess3165Export {};
}  // namespace sim

namespace {

std::string read_text(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) throw std::runtime_error("could not open source file for #3165 export");
  return std::string(std::istreambuf_iterator<char>(input),
                     std::istreambuf_iterator<char>());
}

void write_text(const std::filesystem::path& path, const std::string& text) {
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output) throw std::runtime_error("could not write #3165 export artifact");
  output.write(text.data(), static_cast<std::streamsize>(text.size()));
  if (!output) throw std::runtime_error("could not finish #3165 export artifact");
}

void export_exact_patch() {
  const std::filesystem::path build = std::filesystem::current_path();
  const std::filesystem::path source =
      build.parent_path() / "src/trace_engine_v2/part_007.inc";
  std::string text = read_text(source);

  const std::string before =
      "    return scenario_.dci == DciProfile::StrictJit && scenario_.going_first &&\n";
  const std::string after =
      "    return strict_payload_timing() && scenario_.going_first && // Same-ready-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment ; confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3165\n";
  const std::size_t first = text.find(before);
  if (first == std::string::npos || text.find(before, first + before.size()) != std::string::npos) {
    throw std::runtime_error("#3165 source needle was missing or non-unique");
  }
  text.replace(first, before.size(), after);

  const std::filesystem::path output =
      build / "Testing/issue_3165_part_007_fixed.inc";
  std::filesystem::create_directories(output.parent_path());
  write_text(output, text);
}

}  // namespace

int main() {
  export_exact_patch();
  return 0;
}
