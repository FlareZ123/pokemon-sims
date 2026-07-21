#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool prizes_known = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = prizes_known;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1204_prized_treasure_route_after_legal_reveal();
  }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State exact_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0, 0, 0};
  state.hand = {sim::Card::PathToPeak, sim::Card::ForestSealStone,
                sim::Card::MegaDragonite, sim::Card::ErikasInvitation,
                sim::Card::Dragapult, sim::Card::Gladion, sim::Card::Grass};
  state.prizes = {sim::Card::StevensResolve, sim::Card::Arven,
                  sim::Card::MysteriousTreasure, sim::Card::Grass,
                  sim::Card::Serena, sim::Card::Dipplin};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::QuickBall, sim::Card::Crispin,
                sim::Card::LatiasEx, sim::Card::Fire};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_exact_revealed_treasure_connector() {
  const sim::Scenario scenario{"issue-1204-exact",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{120400};
  sim::Engine engine = make_engine(scenario, rng, exact_state());

  // Gladion's legal reveal proves the prized Treasure, deck-resident Regidrago V,
  // and a remaining redundant Dragon cost. That exact connector must precede Dipplin:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Dipplin: https://api.pokemontcg.io/v2/cards/sv6-127
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1204
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact revealed Treasure connector was not admitted.");
  expect(sim::EngineTestAccess::play_gladion(engine),
         "Gladion did not resolve the exact connector route.");
  expect(engine.state().supporter_used,
         "Gladion did not consume the Supporter play.");
  expect(std::find(engine.state().hand.begin(), engine.state().hand.end(),
                   sim::Card::MysteriousTreasure) != engine.state().hand.end(),
         "Gladion did not take Mysterious Treasure.");
  expect(std::find(engine.state().hand.begin(), engine.state().hand.end(),
                   sim::Card::Dipplin) == engine.state().hand.end(),
         "Gladion still selected inert Dipplin.");
}

void test_route_boundaries() {
  const sim::Scenario live{"issue-1204-controls",
                           sim::DciProfile::NoDiscardControl,
                           sim::LockMode::None, false, 5};
  const auto reject = [&](sim::State state, const sim::Scenario& scenario,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  const sim::Scenario item_lock{"issue-1204-item-lock",
                                sim::DciProfile::NoDiscardControl,
                                sim::LockMode::FullItem, false, 5};
  reject(exact_state(), item_lock, 120401,
         "Item lock must reject the Treasure continuation.");

  sim::State full_bench = exact_state();
  full_bench.bench.assign(5, sim::Pokemon{sim::Card::Oricorio, 0, 0, 0});
  reject(std::move(full_bench), live, 120402,
         "A full Bench must reject the missing-Basic route.");

  sim::State no_target = exact_state();
  no_target.deck.erase(std::remove(no_target.deck.begin(), no_target.deck.end(),
                                   sim::Card::RegidragoV), no_target.deck.end());
  reject(std::move(no_target), live, 120403,
         "A missing deck Regidrago V must reject Treasure.");

  sim::State no_cost = exact_state();
  no_cost.hand = {sim::Card::Gladion};
  reject(std::move(no_cost), live, 120404,
         "No remaining Treasure discard cost must reject the route.");

  sim::State treasure_not_prized = exact_state();
  std::replace(treasure_not_prized.prizes.begin(), treasure_not_prized.prizes.end(),
               sim::Card::MysteriousTreasure, sim::Card::Dipplin);
  reject(std::move(treasure_not_prized), live, 120405,
         "Treasure outside the revealed Prizes must reject the route.");

  sim::State direct_regi_prized = exact_state();
  direct_regi_prized.prizes.front() = sim::Card::RegidragoV;
  std::mt19937_64 rng{120406};
  sim::Engine engine = make_engine(live, rng, std::move(direct_regi_prized));
  expect(sim::EngineTestAccess::play_gladion(engine),
         "The existing direct prized Regidrago V route did not resolve.");
  expect(std::find(engine.state().hand.begin(), engine.state().hand.end(),
                   sim::Card::RegidragoV) != engine.state().hand.end(),
         "The new Treasure route displaced direct prized Regidrago V recovery.");
}

void test_seed_207_reaches_turn_three() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-second");
  if (!scenario) throw std::runtime_error("Missing no-discard-control/go-second scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{207};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The approved seed must choose Treasure after the legal Prize reveal and reach
  // the T3 route demonstrated by the issue without changing the hidden order:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1204
  expect(outcome.first_ready_turn == 3,
         "Seed 207 did not reach readiness on T3.");
  const bool chose_treasure = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T1 | PLAY SUPPORTER") != std::string::npos &&
            line.find("Mysterious Treasure") != std::string::npos;
      });
  expect(chose_treasure, "Seed 207 did not select prized Mysterious Treasure.");
}

}  // namespace

int main() {
  test_exact_revealed_treasure_connector();
  test_route_boundaries();
  test_seed_207_reaches_turn_three();
  return 0;
}
