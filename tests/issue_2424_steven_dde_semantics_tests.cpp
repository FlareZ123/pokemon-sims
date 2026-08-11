#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, bool deck_seen = true,
                        bool prizes_known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_known;
  }
  static bool late_compression(const Engine& engine) {
    return engine.late_steven_has_known_t3_compression_route();
  }
  static bool zero_energy_candidate(const Engine& engine) {
    return engine.steven_zero_energy_latias_vessel_candidate();
  }
  static bool t4_package(const Engine& engine) {
    return engine.issue_1771_steven_t4_package_available();
  }
  static bool t3_package(const Engine& engine) {
    return engine.issue_1772_steven_t3_package_available();
  }
};
}  // namespace sim

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon dde_regi(sim::Card card, int entered, int grass = 0,
                      int fire = 0) {
  return sim::Pokemon{card, entered, grass, fire, sim::Tool::None, 1};
}

void test_late_steven_accepts_one_basic_dde_completion() {
  sim::Scenario scenario{"issue-2424-late", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 4};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{2424};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(dde_regi(sim::Card::RegidragoV, 1));
  state.hand = {sim::Card::StevensResolve, sim::Card::TateLiza,
                sim::Card::QuickBall, sim::Card::RegidragoVstar,
                sim::Card::Grass};
  state.deck = {sim::Card::LatiasEx, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite, sim::Card::RegidragoV};
  state.prizes = {sim::Card::Gladion, sim::Card::Crispin,
                  sim::Card::ProfessorBurnet, sim::Card::Fire,
                  sim::Card::FieldBlower, sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // DDE-only plus the held Grass is a one-attachment completion. Steven's
  // compression proof must not require a raw G/F staging shape.
  // Steven: https://api.pokemontcg.io/v2/cards/sm7-145
  // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2424
  require(sim::EngineTestAccess::late_compression(engine),
          "Late Steven compression rejected DDE plus one held Basic.");
}

void test_dde_is_not_zero_energy_for_steven() {
  sim::Scenario scenario{"issue-2424-zero", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{24240};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(dde_regi(sim::Card::RegidragoV, 1));
  state.hand = {sim::Card::StevensResolve, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::Crispin, sim::Card::LatiasEx,
                sim::Card::EarthenVessel, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::QuickBall, sim::Card::Gladion,
                  sim::Card::ProfessorBurnet, sim::Card::FieldBlower,
                  sim::Card::ErikasInvitation, sim::Card::MysteriousTreasure};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  require(!sim::EngineTestAccess::zero_energy_candidate(engine),
          "Steven misclassified an attached DDE as zero Energy.");
}

void test_dde_complete_benched_regi_admits_t4_package() {
  sim::Scenario scenario{"issue-2424-t4", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 4};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{24241};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(dde_regi(sim::Card::RegidragoV, 1, 1, 0));
  state.hand = {sim::Card::StevensResolve, sim::Card::TateLiza};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                sim::Card::ProfessorBurnet, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::QuickBall, sim::Card::Gladion,
                  sim::Card::Crispin, sim::Card::MysteriousTreasure,
                  sim::Card::FieldBlower, sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  require(sim::EngineTestAccess::t4_package(engine),
          "Steven T4 package rejected a semantically powered DDE+Basic Regidrago.");
}

void test_dde_only_issue1772_does_not_require_crispin() {
  sim::Scenario scenario{"issue-2424-1772", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 3};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{24242};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(dde_regi(sim::Card::RegidragoV, 1));
  state.hand = {sim::Card::StevensResolve, sim::Card::Gladion,
                sim::Card::EarthenVessel, sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Grass, sim::Card::LatiasEx,
                sim::Card::MegaDragonite, sim::Card::QuickBall};
  state.prizes = {sim::Card::Crispin, sim::Card::ProfessorBurnet,
                  sim::Card::MysteriousTreasure, sim::Card::Fire,
                  sim::Card::FieldBlower, sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  require(sim::EngineTestAccess::t3_package(engine),
          "DDE-only Steven T3 package still incorrectly required Crispin/two Basics.");
}
}  // namespace

int main() {
  test_late_steven_accepts_one_basic_dde_completion();
  test_dde_is_not_zero_energy_for_steven();
  test_dde_complete_benched_regi_admits_t4_package();
  test_dde_only_issue1772_does_not_require_crispin();
}
