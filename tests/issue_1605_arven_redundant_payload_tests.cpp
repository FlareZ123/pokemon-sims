#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool route(const Engine& engine) {
    return engine.issue_1605_arven_crobat_route_available();
  }
};
}

namespace {
void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::Engine make_unit_engine(sim::Scenario& scenario,
                             sim::DeckRecipe& recipe,
                             std::mt19937_64& rng) {
  return sim::Engine(scenario, recipe, rng);
}

void install_public_state(sim::Engine& engine) {
  auto& state = sim::EngineTestAccess::state(engine);
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Arven, sim::Card::MegaDragonite,
                sim::Card::MegaDragonite, sim::Card::GoodraVstar,
                sim::Card::TeamYellsCheer, sim::Card::CrobatV,
                sim::Card::ForestSealStone};
  state.deck = {sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                sim::Card::RegidragoV};
}

bool route_for(sim::LockMode locks, std::vector<sim::Card> hand,
               int bench_count = 0, int accounted_regi = 0,
               bool going_first = false, int turn = 1,
               bool supporter_used = false) {
  sim::Scenario scenario{"issue-1605-unit", sim::DciProfile::StrictJit,
                         locks, going_first, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1};
  sim::Engine engine = make_unit_engine(scenario, recipe, rng);
  install_public_state(engine);
  auto& state = sim::EngineTestAccess::state(engine);
  state.turn = turn;
  state.supporter_used = supporter_used;
  state.hand = std::move(hand);
  for (int i = 0; i < bench_count; ++i) {
    state.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 0, 0, 0,
                                        sim::Tool::None});
  }
  for (int i = 0; i < accounted_regi; ++i) {
    state.discard.push_back(sim::Card::RegidragoV);
  }
  return sim::EngineTestAccess::route(engine);
}

void test_public_controls() {
  const std::vector<sim::Card> exact{
      sim::Card::Arven, sim::Card::MegaDragonite,
      sim::Card::MegaDragonite, sim::Card::GoodraVstar,
      sim::Card::TeamYellsCheer, sim::Card::CrobatV,
      sim::Card::ForestSealStone};
  expect(route_for(sim::LockMode::None, exact),
         "The exact public Arven route was rejected.");
  expect(!route_for(sim::LockMode::None,
                    {sim::Card::Arven, sim::Card::MegaDragonite,
                     sim::Card::TeamYellsCheer, sim::Card::CrobatV,
                     sim::Card::ForestSealStone}),
         "A single payload was exposed.");
  expect(!route_for(sim::LockMode::None,
                    {sim::Card::Arven, sim::Card::MegaDragonite,
                     sim::Card::MegaDragonite,
                     sim::Card::MegaDragonite,
                     sim::Card::TeamYellsCheer, sim::Card::CrobatV,
                     sim::Card::ForestSealStone}),
         "The route failed to preserve a distinct payload identity.");
  expect(!route_for(sim::LockMode::FullItem, exact),
         "The route ignored Item lock.");
  expect(!route_for(sim::LockMode::FullSupporter, exact),
         "The route ignored Supporter lock.");
  expect(!route_for(sim::LockMode::FullRuleBoxAbility, exact),
         "The route ignored Rule Box Ability lock.");
  expect(!route_for(sim::LockMode::None, exact, 5),
         "The route ignored a full Bench.");
  expect(!route_for(sim::LockMode::None, exact, 0, 4),
         "The route invented an absent Regidrago V target.");
  auto large_hand = exact;
  large_hand.insert(large_hand.end(),
                    {sim::Card::Lusamine, sim::Card::Channeler,
                     sim::Card::RoseannesBackup});
  expect(!route_for(sim::LockMode::None, large_hand),
         "The route used Crobat when the projected hand already had six cards.");
}

void test_issue_2645_state_driven_turn_order_controls() {
  const std::vector<sim::Card> exact{
      sim::Card::Arven, sim::Card::MegaDragonite,
      sim::Card::MegaDragonite, sim::Card::GoodraVstar,
      sim::Card::TeamYellsCheer, sim::Card::CrobatV,
      sim::Card::ForestSealStone};

  // The first player cannot play a Supporter on T1. Once that restriction has
  // passed, the same observable Arven connector remains legal on T2 and later.
  // Core first-turn and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // State-based route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed overfit bug: https://github.com/FlareZ123/pokemon-sims/issues/2645
  expect(route_for(sim::LockMode::None, exact, 0, 0, false, 1),
         "The original go-second T1 route was rejected.");
  expect(!route_for(sim::LockMode::None, exact, 0, 0, true, 1),
         "The first player illegally used Arven on T1.");
  expect(route_for(sim::LockMode::None, exact, 0, 0, true, 2),
         "The go-first T2 Arven route remained hard-coded away.");
  expect(route_for(sim::LockMode::None, exact, 0, 0, false, 3),
         "A later legal Arven route was rejected by turn identity.");
  expect(!route_for(sim::LockMode::None, exact, 0, 0, true, 2, true),
         "The route ignored an already-used Supporter action.");
}

void test_seed_7_executes_arven_crobat_route() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::crobat_modeling_deck_by_id("crobat1-erika");
  expect(scenario && deck, "Issue-1605 fixture unavailable.");
  std::mt19937_64 rng{7};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();

  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Dynamic DCI and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1605
  expect(has(trace, "T1 | PLAY SUPPORTER") && has(trace, "Arven") &&
             has(trace, "T1 | DISCARD") &&
             has(trace, "Mega Dragonite ex") &&
             has(trace, "T1 | BENCH") && has(trace, "Regidrago V") &&
             has(trace, "Forest Seal Stone") &&
             has(trace, "Crobat V") && has(trace, "Dark Asset"),
         "Seed 7 did not execute the public Arven-Crobat continuation.");
}
}

int main() {
  test_public_controls();
  test_issue_2645_state_driven_turn_order_controls();
  test_seed_7_executes_arven_crobat_route();
}
