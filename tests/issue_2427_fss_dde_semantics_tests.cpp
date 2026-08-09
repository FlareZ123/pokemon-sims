#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true,
                        const bool prizes_known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_known;
  }
  static bool hold_complete_quick_ball(const Engine& engine) {
    return engine.fss_should_hold_for_complete_quick_ball_route();
  }
  static bool latias_vessel(const Engine& engine) {
    return engine.issue_1672_fss_vessel_latias_route_available();
  }
  static bool fss_split(const Engine& engine) {
    return engine.issue_1356_fss_treasure_energy_split_available();
  }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool treasure_split(const Engine& engine) {
    return engine.issue_1356_treasure_vstar_energy_split_available();
  }
  static bool play_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
  static bool hold_quick_ball(const Engine& engine) {
    return engine.issue_1356_complete_next_turn_route_holds_quick_ball();
  }
  static int min_basic(const Engine& engine, const Pokemon& pokemon,
                       int grass, int fire) {
    return engine.minimum_basic_attachments_to_apex(pokemon, grass, fire);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon dde_regi(sim::Card card, int entered_turn = 1,
                      sim::Tool tool = sim::Tool::None) {
  return sim::Pokemon{card, entered_turn, 0, 0, tool, 1};
}

bool has(const std::vector<sim::Card>& zone, sim::Card card) {
  return std::find(zone.begin(), zone.end(), card) != zone.end();
}

void test_physical_basic_distance() {
  sim::Scenario scenario{"issue-2427-distance", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 4};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{2427};
  sim::Engine engine{scenario, recipe, rng};
  const sim::Pokemon dde_only = dde_regi(sim::Card::RegidragoVstar);

  // DDE-only is one *physical* Basic attachment from Apex even though Grass and
  // Fire are both legal typed completions. Do not add those options as deficits.
  // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2427
  require(sim::EngineTestAccess::min_basic(engine, dde_only, 1, 0) == 1,
          "DDE-only plus Grass was not one physical attachment from Apex.");
  require(sim::EngineTestAccess::min_basic(engine, dde_only, 0, 1) == 1,
          "DDE-only plus Fire was not one physical attachment from Apex.");
}

void test_complete_quick_ball_route_holds_fss_with_dde() {
  sim::Scenario scenario{"issue-2427-hold", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 4};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{24271};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 3;
  state.supporter_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                                     sim::Tool::ForestSealStone, 1});
  state.hand = {sim::Card::RegidragoVstar, sim::Card::QuickBall,
                sim::Card::MegaDragonite, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire,
                sim::Card::Crispin};
  state.prizes = {sim::Card::RegidragoV, sim::Card::Gladion,
                  sim::Card::MysteriousTreasure, sim::Card::ProfessorBurnet,
                  sim::Card::FieldBlower, sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  require(sim::EngineTestAccess::hold_complete_quick_ball(engine),
          "DDE+Basic complete Regidrago failed the FSS complete-route hold.");
}

void test_dde_only_latias_vessel_route() {
  sim::Scenario scenario{"issue-2427-latias-vessel",
                         sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{24272};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 3;
  state.supporter_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(dde_regi(sim::Card::RegidragoV, 1,
                                 sim::Tool::ForestSealStone));
  state.hand = {sim::Card::EarthenVessel, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::LatiasEx, sim::Card::Grass,
                sim::Card::Crispin, sim::Card::Fire};
  state.prizes = {sim::Card::QuickBall, sim::Card::Gladion,
                  sim::Card::ProfessorBurnet, sim::Card::MysteriousTreasure,
                  sim::Card::FieldBlower, sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  require(sim::EngineTestAccess::latias_vessel(engine),
          "DDE-only Regidrago was not recognized as one-Basic Latias/Vessel finish.");
}

void test_issue1356_star_target_and_surplus_are_semantic() {
  sim::Scenario scenario{"issue-2427-fss-split", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{24273};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = dde_regi(sim::Card::RegidragoV, 1,
                          sim::Tool::ForestSealStone);
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure,
                sim::Card::Fire, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::Crispin};
  state.prizes = {sim::Card::RegidragoV, sim::Card::Gladion,
                  sim::Card::ProfessorBurnet, sim::Card::QuickBall,
                  sim::Card::FieldBlower, sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  require(sim::EngineTestAccess::fss_split(engine),
          "DDE-only issue-1356 state did not admit semantic Star Alchemy Basic target.");
  require(sim::EngineTestAccess::use_fss(engine),
          "Star Alchemy did not execute the semantic DDE Basic target route.");
  const sim::State& after_fss = sim::EngineTestAccess::state(engine);
  require(has(after_fss.hand, sim::Card::Grass),
          "Star Alchemy failed to search the one-Basic DDE completion.");
  require(after_fss.vstar_power_used,
          "Star Alchemy did not consume the VSTAR Power.");

  require(sim::EngineTestAccess::treasure_split(engine),
          "Post-FSS DDE Treasure split did not reserve the semantic completion.");
  require(sim::EngineTestAccess::hold_quick_ball(engine),
          "Post-FSS DDE complete next-turn route did not hold Quick Ball.");
  require(sim::EngineTestAccess::play_treasure(engine),
          "Post-FSS Treasure/VSTAR split did not execute.");
  const sim::State& after_treasure = sim::EngineTestAccess::state(engine);
  require(has(after_treasure.hand, sim::Card::Grass),
          "Treasure discarded the completing Basic instead of the surplus Basic.");
  require(has(after_treasure.discard, sim::Card::Fire),
          "Treasure did not spend the semantic surplus Basic.");
  require(has(after_treasure.hand, sim::Card::RegidragoVstar),
          "Treasure did not search Regidrago VSTAR.");
}
}  // namespace

int main() {
  test_physical_basic_distance();
  test_complete_quick_ball_route_holds_fss_with_dde();
  test_dde_only_latias_vessel_route();
  test_issue1356_star_target_and_surplus_are_semantic();
}
