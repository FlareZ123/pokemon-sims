#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, bool deck_seen = true,
                        bool prizes_known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_known;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_steven(Engine& engine) { return engine.play_steven(); }
  static bool dde_completes_apex(Engine& engine, Pokemon pokemon) {
    return engine.attach_energy_card(pokemon, Card::DoubleDragonEnergy) &&
           engine.pays_apex_energy_cost(pokemon);
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

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
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

void test_steven_reserves_dde_instead_of_crispin_for_one_card_completion() {
  sim::Scenario scenario{"issue-2424-steven-search-dde", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 3};
  auto recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng{20260811};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None, 0};
  state.hand = {sim::Card::StevensResolve};
  state.deck = {sim::Card::RegidragoVstar,
                sim::Card::DoubleDragonEnergy,
                sim::Card::Crispin,
                sim::Card::ProfessorBurnet,
                sim::Card::MegaDragonite,
                sim::Card::Grass,
                sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state), false, false);

  // Steven can search any three cards. With one Grass already attached, one DDE is
  // the complete next-turn manual Energy step for Apex Dragon's GGF cost, so Steven
  // should reserve VSTAR + DDE + Burnet and leave Crispin in deck. This keeps the
  // next-turn Supporter slot available for the payload Supporter instead of spending
  // it on Energy acceleration.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // DDE modeling contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  require(sim::EngineTestAccess::play_steven(engine),
          "Steven did not select the DDE one-card completion route.");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  require(contains(after.hand, sim::Card::RegidragoVstar),
          "Steven failed to reserve Regidrago VSTAR.");
  require(contains(after.hand, sim::Card::DoubleDragonEnergy),
          "Steven failed to reserve Double Dragon Energy.");
  require(contains(after.hand, sim::Card::ProfessorBurnet),
          "Steven failed to preserve the next-turn payload Supporter.");
  require(!contains(after.hand, sim::Card::Crispin) &&
              contains(after.deck, sim::Card::Crispin),
          "Steven still spent a search target on Crispin despite DDE completion.");
  require(after.turn_ended && contains(after.discard, sim::Card::StevensResolve),
          "Steven did not end the turn after resolving.");

  sim::Pokemon projected = *after.active;
  require(sim::EngineTestAccess::dde_completes_apex(engine, projected),
          "One attached Grass plus DDE did not pay Apex Dragon's GGF cost.");
}
}  // namespace

int main() {
  test_late_steven_accepts_one_basic_dde_completion();
  test_dde_is_not_zero_energy_for_steven();
  test_dde_complete_benched_regi_admits_t4_package();
  test_dde_only_issue1772_does_not_require_crispin();
  test_steven_reserves_dde_instead_of_crispin_for_one_card_completion();
}
