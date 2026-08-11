#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::filesystem::path source_root() {
  std::filesystem::path directory = std::filesystem::current_path();
  for (int depth = 0; depth < 4; ++depth) {
    if (std::filesystem::exists(directory / "src" / "regidrago_sim.cpp")) {
      return directory;
    }
    if (!directory.has_parent_path()) break;
    directory = directory.parent_path();
  }
  throw std::runtime_error("Could not locate repository source root");
}

std::string read_source(const std::filesystem::path& path) {
  const std::filesystem::path resolved =
      path.is_absolute() ? path : source_root() / path;
  std::ifstream input(resolved);
  expect(input.good(), "Could not open source file for issue-2368 static audit");
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

void require_text(const std::string& source, const std::string& needle,
                  const char* message) {
  expect(source.find(needle) != std::string::npos, message);
}

void test_dde_sensitive_sequencing_uses_semantic_apex_payment() {
  // Double Dragon Energy supplies every Energy type and two Energy while attached
  // to a Dragon Pokémon, so raw Grass/Fire counters cannot decide whether Apex
  // Dragon's GGF cost is complete:
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Energy and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Semantic Energy policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed umbrella bug: https://github.com/FlareZ123/pokemon-sims/issues/2368
  const std::string issue1878 = read_source(
      "src/trace_engine_v2/part_issue_1878_vessel_quick_ball_tapu_crispin_route.inc");
  require_text(issue1878, "pays_apex_energy_cost(projected)",
               "Issue-1878 route lost semantic projected Apex payment");
  require_text(issue1878, "!need_energy() || held_manual_energy_finishes",
               "Issue-1878 route lost DDE-aware completion admission guard");

  const std::string tate = read_source(
      "src/trace_engine_v2/part_tate_blender_tate_override.inc");
  require_text(tate, "pokemon.double_dragon > 0 && pays_apex_energy_cost(pokemon)",
               "Dynamic DCI lost attached-DDE semantic completion guard");
  require_text(tate, "dde_completed_energy_line && !need_energy()",
               "Dynamic DCI lost semantic Energy-axis completion requirement");
  require_text(tate, "issue_2368_preserve_direct_treasure_vstar_payload_completion()",
               "Tate draw route lost direct DDE Treasure completion preservation");

  const std::string gladion = read_source(
      "src/trace_engine_v2/part_issue_1595_gladion_grass_turo_blender_override.inc");
  require_text(gladion, "state_.active->double_dragon > 0 &&",
               "Gladion route lost physical attached-DDE requirement");
  require_text(gladion, "pays_apex_energy_cost(*state_.active)",
               "Gladion route lost semantic Apex payment check");

  const std::string steven_blender = read_source(
      "src/trace_engine_v2/part_issue_1798_steven_blender_route.inc");
  require_text(steven_blender,
               "return pays_apex_energy_cost(*state_.active) && payload_ready();",
               "Issue-1798 route restored a raw Basic-Energy Apex readiness proxy");
}

void test_celestial_roar_has_no_retired_raw_missing_energy_counters() {
  // The Celestial Roar next-window helper projects candidate attachments through
  // semantic Energy handling. Retired raw missing-Grass/Fire counters must stay out:
  // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Confirmed cleanup: https://github.com/FlareZ123/pokemon-sims/issues/2370
  // Parent semantic audit: https://github.com/FlareZ123/pokemon-sims/issues/2368
  const std::string celestial = read_source(
      "src/trace_engine_v2/part_celestial_roar_override.inc");
  expect(celestial.find("missing_grass") == std::string::npos,
         "Celestial Roar restored retired raw missing_grass readiness state");
  expect(celestial.find("missing_fire") == std::string::npos,
         "Celestial Roar restored retired raw missing_fire readiness state");
  require_text(celestial, "pays_apex_energy_cost(projected)",
               "Celestial Roar lost semantic projected Apex payment");
}

void test_no_generic_raw_apex_readiness_proxies_remain() {
  // Apex readiness is semantic once DDE exists. Printed Basic-Energy-only effects
  // may inspect Basic card identities, while setup-route admission must not rebuild
  // physical distance by adding typed completion options or demanding raw Basic GGF:
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Umbrella migration contract: https://github.com/FlareZ123/pokemon-sims/issues/2368
  const auto directory = source_root() / "src" / "trace_engine_v2";
  const std::regex additive_typed_deficit(
      R"(grass_needed\(\)\s*\+\s*fire_needed\(\)|fire_needed\(\)\s*\+\s*grass_needed\(\))");
  const std::regex raw_ggf(
      R"((grass\s*>=\s*2[^;\n]*fire\s*>=\s*1)|(fire\s*>=\s*1[^;\n]*grass\s*>=\s*2))");

  for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".inc") continue;
    const std::string filename = entry.path().filename().string();
    const std::string source = read_source(entry.path());
    expect(!std::regex_search(source, additive_typed_deficit),
           ("DDE migration restored additive typed-deficit math in " + filename).c_str());
    if (filename == "part_004.inc") continue;  // Canonical no-DDE payment branch: https://github.com/FlareZ123/pokemon-sims/issues/2368
    expect(!std::regex_search(source, raw_ggf),
           ("DDE migration restored raw GGF readiness proxy in " + filename).c_str());
  }
}

void test_remaining_route_families_are_semantic() {
  // These route families historically contained raw Basic-energy distance checks.
  // DDE and Apex semantics: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ https://api.pokemontcg.io/v2/cards/swsh12-136
  // Umbrella migration: https://github.com/FlareZ123/pokemon-sims/issues/2368
  const std::string fss = read_source("src/trace_engine_v2/part_010_fss_override.inc");
  require_text(fss, "minimum_basic_attachments_to_apex",
               "FSS family lost physical DDE attachment-distance helper");
  require_text(fss, "pays_apex_energy_cost",
               "FSS family lost semantic Apex payment checks");

  const std::string steven = read_source(
      "src/trace_engine_v2/part_010_steven_crispin_override.inc");
  require_text(steven, "completing_basic_energy_for",
               "Steven family lost DDE one-Basic completion projection");
  require_text(steven, "double_dragon == 0",
               "Steven zero-Energy staging no longer excludes DDE");

  const std::string vessel = read_source(
      "src/trace_engine_v2/part_earthen_vessel_vstar_window_override.inc");
  require_text(vessel, "completing_basic_energy_for",
               "Vessel/Latias hold lost semantic one-Basic completion");

  const std::string gladion = read_source(
      "src/trace_engine_v2/part_issue_1608_burnet_before_dead_crispin_override.inc");
  require_text(gladion, "completing_basic_energy_for",
               "Gladion family lost DDE-aware future-manual completion");

  const std::string latias = read_source(
      "src/trace_engine_v2/part_014c_latias_bench_override.inc");
  require_text(latias, "pays_apex_energy_cost(*post_attach_target)",
               "Latias search-completion family lost semantic post-attach readiness");
}

}  // namespace

int main() {
  test_dde_sensitive_sequencing_uses_semantic_apex_payment();
  test_celestial_roar_has_no_retired_raw_missing_energy_counters();
  test_no_generic_raw_apex_readiness_proxies_remain();
  test_remaining_route_families_are_semantic();
}
