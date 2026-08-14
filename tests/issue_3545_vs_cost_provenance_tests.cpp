#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static bool recovered_supporter_is_used(Engine& engine, const Card candidate) {
    return engine.issue_3545_recovered_supporter_is_used(engine, candidate);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_recovered_supporter_spent_as_treasure_cost_gets_no_supporter_credit() {
  // This is the seed-11 failure shape: VS has put Guzma into hand, then the
  // pre-Supporter Mysterious Treasure chain can spend Guzma as its discard cost and
  // obtain Tapu Lele-GX -> Crispin. The route evaluator must distinguish that Item
  // cost from actually playing Guzma as the turn's Supporter.
  // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Guzma: https://api.pokemontcg.io/v2/cards/sm3-115
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Production sequencing: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
  std::mt19937_64 rng{3545011};
  sim::Scenario scenario{"issue-3545-vs-cost", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0};
  state.hand = {sim::Card::Guzma,
                sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure,
                sim::Card::RegidragoVstar,
                sim::Card::StevensResolve,
                sim::Card::ChaoticSwell};
  state.deck = {sim::Card::RegidragoV,
                sim::Card::TapuLeleGX,
                sim::Card::Crispin,
                sim::Card::Grass,
                sim::Card::Grass,
                sim::Card::Fire,
                sim::Card::Fire,
                sim::Card::MegaDragonite,
                sim::Card::Dragapult,
                sim::Card::GoodraVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::recovered_supporter_is_used(engine, sim::Card::Guzma),
         "VS route credited Guzma after the Item chain consumed it as discard fuel.");
}
}  // namespace

int main() {
  try {
    test_recovered_supporter_spent_as_treasure_cost_gets_no_supporter_credit();
    std::cout << "issue 3545 VS cost-provenance tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
