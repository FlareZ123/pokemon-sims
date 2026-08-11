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
  static bool gladion_next_manual(const Engine& engine) {
    return engine.issue_1697_held_next_turn_manual_energy_route();
  }
  static bool vessel_latias_hold(const Engine& engine) {
    return engine.hold_earthen_vessel_for_next_turn_latias_jit();
  }
  static bool issue1700(Engine& engine) {
    return engine.play_issue_1700_known_prize_gladion_before_blender();
  }
  static bool post_attach_latias(Engine& engine) {
    return engine.post_attach_treasure_latias_route_available();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon regi(sim::Card card, int entered, int grass, int fire, int dde,
                  sim::Tool tool = sim::Tool::None) {
  return sim::Pokemon{card, entered, grass, fire, tool, dde};
}

void test_gladion_future_manual_route_accepts_dde_only() {
  sim::Scenario scenario{"issue-2422-dde-manual", sim::DciProfile::NoDiscardControl,
                         sim::LockMode::None, false, 4};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{2422};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = regi(sim::Card::RegidragoV, 1, 0, 0, 1);
  state.hand = {sim::Card::Grass, sim::Card::Crispin};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite};
  state.prizes = {sim::Card::QuickBall, sim::Card::Gladion,
                  sim::Card::ProfessorBurnet, sim::Card::MysteriousTreasure,
                  sim::Card::FieldBlower, sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // One DDE is exactly one physical Basic attachment from Apex. The next-turn
  // Gladion/FSS preservation classifier must not add Grass and Fire options as
  // separate card deficits. DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed family bug: https://github.com/FlareZ123/pokemon-sims/issues/2422
  require(sim::EngineTestAccess::gladion_next_manual(engine),
          "Gladion future-manual route rejected DDE-only plus one held Basic.");
}

void test_vessel_latias_hold_accepts_dde_only() {
  sim::Scenario scenario{"issue-2425-dde-hold", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{2425};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(regi(sim::Card::RegidragoVstar, 1, 0, 0, 1));
  state.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0,
                                     sim::Tool::None, 0});
  state.hand = {sim::Card::EarthenVessel, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Grass,
                sim::Card::RegidragoV};
  state.prizes = {sim::Card::QuickBall, sim::Card::Gladion,
                  sim::Card::ProfessorBurnet, sim::Card::MysteriousTreasure,
                  sim::Card::FieldBlower, sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Either searched Basic completes DDE-only Apex next turn, while Vessel can
  // discard the held Dragon on the strict-JIT ready turn.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Bug: https://github.com/FlareZ123/pokemon-sims/issues/2425
  require(sim::EngineTestAccess::vessel_latias_hold(engine),
          "Vessel/Latias hold rejected a DDE-only one-Basic next-turn finish.");
}

sim::State issue1700_state(bool with_dde) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(regi(sim::Card::RegidragoV, 1, 1, 0,
                             with_dde ? 1 : 0));
  state.hand = {sim::Card::Gladion, sim::Card::BrilliantBlender,
                sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::LatiasEx};
  state.discard = {sim::Card::EarthenVessel, sim::Card::StevensResolve};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Fire,
                sim::Card::RegidragoV};
  state.prizes = {sim::Card::Fire, sim::Card::QuickBall,
                  sim::Card::ProfessorBurnet, sim::Card::MysteriousTreasure,
                  sim::Card::FieldBlower, sim::Card::ErikasInvitation};
  return state;
}

void test_prized_fire_shortcut_respects_dde_readiness() {
  sim::Scenario scenario{"issue-2428", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  auto recipe = sim::double_dragon_modeling_recipe();

  std::mt19937_64 dde_rng{2428};
  sim::Engine dde_engine{scenario, recipe, dde_rng};
  sim::EngineTestAccess::set_state(dde_engine, issue1700_state(true));
  require(!sim::EngineTestAccess::issue1700(dde_engine),
          "Gladion spent the Supporter on prized Fire despite DDE+Grass already paying Apex.");
  require(!sim::EngineTestAccess::state(dde_engine).supporter_used,
          "DDE-complete issue-1700 negative control consumed the Supporter.");

  std::mt19937_64 basic_rng{24280};
  sim::Engine basic_engine{scenario, recipe, basic_rng};
  sim::EngineTestAccess::set_state(basic_engine, issue1700_state(false));
  require(sim::EngineTestAccess::issue1700(basic_engine),
          "Original one-Grass/no-DDE prized-Fire route regressed.");
  require(sim::EngineTestAccess::state(basic_engine).supporter_used,
          "Original issue-1700 route did not spend Gladion.");
}

void test_post_attach_latias_accepts_dde_complete_vstar() {
  sim::Scenario scenario{"issue-2442", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{2442};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(regi(sim::Card::RegidragoVstar, 1, 1, 0, 1));
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MegaDragonite};
  state.deck = {sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::QuickBall, sim::Card::Gladion,
                  sim::Card::ProfessorBurnet, sim::Card::RegidragoV,
                  sim::Card::FieldBlower, sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The selected Benched VSTAR is Energy-complete through DDE+Grass. Treasure can
  // discard the current-turn Dragon payload and search Latias to solve only the
  // Active-position axis. Bug: https://github.com/FlareZ123/pokemon-sims/issues/2442
  require(sim::EngineTestAccess::post_attach_latias(engine),
          "Post-attachment Treasure/Latias route rejected DDE+Basic ready VSTAR.");
}
}  // namespace

int main() {
  test_gladion_future_manual_route_accepts_dde_only();
  test_vessel_latias_hold_accepts_dde_only();
  test_prized_fire_shortcut_respects_dde_readiness();
  test_post_attach_latias_accepts_dde_complete_vstar();
}
