#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool issue_1878_route_available(const Engine& engine) {
    return engine.issue_1878_vessel_quick_ball_tapu_crispin_route_available();
  }
  static std::optional<Card> choose_discard(Engine& engine,
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded = std::nullopt) {
    return engine.choose_discard_issue1740(permit_payload, flex_fodder,
                                           allow_heavy_ball, excluded);
  }
};
}  // namespace sim

namespace {

constexpr const char* kIssue =
    "https://github.com/FlareZ123/pokemon-sims/issues/2368";
constexpr const char* kDde =
    "https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/";
constexpr const char* kApex =
    "https://api.pokemontcg.io/v2/cards/swsh12-136";
constexpr const char* kRules =
    "https://www.pokemon.com/us/pokemon-tcg/rules";

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::DeckRecipe two_grass_to_dde_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  const auto grass = std::find_if(recipe.begin(), recipe.end(), [](const auto& entry) {
    return entry.first == sim::Card::Grass;
  });
  expect(grass != recipe.end() && grass->second == 6,
         "Canonical six-Grass recipe disappeared.");
  const auto index = std::distance(recipe.begin(), grass);
  grass->second = 4;
  // Insert immediately before Grass. Expansion is therefore
  // [D,D,G,G,G,G,F,F,F] versus canonical [G,G,G,G,G,G,F,F,F], so a shared
  // per-game seed is an exact physical-slot relabel of two Grass cards.
  recipe.insert(recipe.begin() + index, {sim::Card::DoubleDragonEnergy, 2});
  return recipe;
}

sim::Pokemon regi(const sim::Card card, const int grass = 0,
                  const int fire = 0, const int dde = 0) {
  sim::Pokemon pokemon{card, 1, grass, fire, sim::Tool::None};
  pokemon.double_dragon = dde;
  return pokemon;
}

sim::TrialOutcome run_seed(const std::string& scenario_label,
                           const std::uint64_t seed,
                           sim::TraceLog* trace = nullptr) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  expect(scenario.has_value(), "Required registered scenario disappeared.");
  std::mt19937_64 rng(seed);
  sim::Engine engine(*scenario, two_grass_to_dde_recipe(), rng, trace);
  return engine.run();
}

void test_issue_1878_rejects_dde_complete_and_direct_dde_finish() {
  const sim::Scenario scenario{
      "issue-2368/1878", sim::DciProfile::StrictJit,
      sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = two_grass_to_dde_recipe();
  std::mt19937_64 rng(2368);

  sim::State complete;
  complete.turn = 3;
  complete.active = regi(sim::Card::RegidragoVstar, 0, 1, 1);
  complete.hand = {sim::Card::EarthenVessel, sim::Card::QuickBall,
                   sim::Card::Dragapult, sim::Card::StevensResolve};
  complete.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                   sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                   sim::Card::Fire};
  sim::Engine ready_engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(ready_engine, std::move(complete));

  // Fire + DDE already pays GGF. A pre-DDE route may not reinterpret raw
  // grass==0/fire==1 as "Fire only" and start a two-Grass Vessel/Crispin chain.
  // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2368
  expect(!sim::EngineTestAccess::issue_1878_route_available(ready_engine),
         "Issue-1878 route admitted an already-payable [FD] state.");

  sim::State direct;
  direct.turn = 3;
  direct.active = regi(sim::Card::RegidragoVstar, 0, 1, 0);
  direct.hand = {sim::Card::DoubleDragonEnergy,
                 sim::Card::EarthenVessel, sim::Card::QuickBall,
                 sim::Card::Dragapult, sim::Card::StevensResolve};
  direct.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                 sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                 sim::Card::Fire};
  sim::Engine direct_engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(direct_engine, std::move(direct));
  expect(!sim::EngineTestAccess::issue_1878_route_available(direct_engine),
         "Issue-1878 route admitted a state where held DDE directly finishes GGF.");
}

void test_redundant_crispin_becomes_dynamic_dci_fodder() {
  const sim::Scenario scenario{
      "issue-2368/dci", sim::DciProfile::StrictJit,
      sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = two_grass_to_dde_recipe();
  std::mt19937_64 rng(236801);
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoV, 1, 0, 1);
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Crispin};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // When DDE has already completed the Energy axis and no current-turn Dragon
  // payload competes for the cost, singleton Crispin has no remaining setup value
  // and must not strand Mysterious Treasure under dynamic DCI.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2368
  const auto cost = sim::EngineTestAccess::choose_discard(
      engine, true, true, true, sim::Card::MysteriousTreasure);
  expect(cost.has_value() && *cost == sim::Card::Crispin,
         "Redundant singleton Crispin remained protected after DDE completed Energy.");
}

void test_exact_paired_witnesses() {
  struct Witness {
    const char* scenario;
    std::uint64_t seed;
    int expected_ready_turn;
  };
  const std::vector<Witness> witnesses{
      {"strict-jit/go-first", 202608170408ULL, 3},
      {"strict-jit/go-second", 202611171091ULL, 3},
      {"strict-jit/go-first", 202608080195ULL, 2},
      {"strict-jit/go-first", 202608081180ULL, 2},
      {"strict-jit/go-first", 202608080032ULL, 2},
  };

  // These are exact paired-seed witnesses from the 4G/3F/2DDE audit. The first
  // two formerly threw inside the issue-1878 Basic-Energy package; the remaining
  // three lost T2 to Tate draw, stale Crispin protection, and redundant Gladion
  // Grass selection respectively. Every route now respects semantic Apex payment.
  // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2368
  for (const Witness& witness : witnesses) {
    sim::TraceLog trace{true, {}, {}};
    const sim::TrialOutcome outcome = run_seed(witness.scenario, witness.seed, &trace);
    if (outcome.first_ready_turn != witness.expected_ready_turn) {
      std::cerr << witness.scenario << " seed " << witness.seed
                << " expected ready T" << witness.expected_ready_turn
                << " but got T" << outcome.first_ready_turn << '\n';
      for (const auto& line : trace.lines) std::cerr << line << '\n';
      throw std::runtime_error("Issue-2368 paired witness regressed.");
    }
  }
}

void test_canonical_issue_1878_route_is_preserved() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Canonical issue-1878 control disappeared.");
  std::mt19937_64 rng(241);
  sim::Engine engine(*scenario, deck->recipe, rng);
  const sim::TrialOutcome outcome = engine.run();
  expect(outcome.first_ready_turn == 3,
         "DDE guards broke the canonical seed-241 issue-1878 route.");
}

}  // namespace

int main() {
  try {
    (void)kIssue; (void)kDde; (void)kApex; (void)kRules;
    test_issue_1878_rejects_dde_complete_and_direct_dde_finish();
    test_redundant_crispin_becomes_dynamic_dci_fodder();
    test_exact_paired_witnesses();
    test_canonical_issue_1878_route_is_preserved();
    std::cout << "Issue 2368 DDE sequencing tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
