#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool split_available(const Engine& engine) {
    return engine.issue_1356_treasure_vstar_energy_split_available();
  }
  static bool play_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

bool has(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

const sim::DeckRecipe& test_recipe() {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return recipe;
}

sim::Scenario test_scenario() {
  return sim::Scenario{"issue-1356-fss-treasure-energy", sim::DciProfile::StrictJit,
                       sim::LockMode::None, true, 3};
}

sim::State split_state() {
  sim::State state;
  state.turn = 2;
  state.supporter_used = true;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite, sim::Card::Grass,
                sim::Card::QuickBall, sim::Card::Lusamine};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Fire,
                sim::Card::TapuLeleGX, sim::Card::Dragapult,
                sim::Card::Oricorio};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng) {
  return sim::Engine(scenario, test_recipe(), rng);
}

void resolve_fss(sim::Engine& engine) {
  expect(sim::EngineTestAccess::use_fss(engine),
         "Forest Seal Stone should resolve in the fixture");
}

void test_treasure_takes_vstar_and_fss_takes_fire() {
  const sim::Scenario scenario = test_scenario();
  std::mt19937_64 rng(135601);
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, split_state());

  // The public seed-107 hand has a prior-turn Active Regidrago V with GG, two
  // Treasures after Arven, a held Dragon, and one surplus Grass. Star Alchemy
  // should assign itself to Fire while Treasure supplies Regidrago VSTAR:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356
  resolve_fss(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(has(result.hand, sim::Card::Fire),
         "Star Alchemy should search the missing next-turn Fire Energy");
  expect(has(result.deck, sim::Card::RegidragoVstar),
         "Mysterious Treasure must retain the VSTAR axis");
}

void test_split_is_symmetric_for_missing_grass() {
  sim::State state = split_state();
  state.active->grass = 1;
  state.active->fire = 1;
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite, sim::Card::Fire,
                sim::Card::QuickBall, sim::Card::Lusamine};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::TapuLeleGX, sim::Card::Dragapult,
                sim::Card::Oricorio};

  const sim::Scenario scenario = test_scenario();
  std::mt19937_64 rng(135602);
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Forest Seal Stone searches any card, so the connector split is symmetric when
  // Grass is the sole next-turn attachment and Fire is the surplus cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1356
  resolve_fss(engine);
  expect(has(sim::EngineTestAccess::state(engine).hand, sim::Card::Grass),
         "Star Alchemy should search the missing next-turn Grass Energy");
}

void test_pending_treasure_bridge_holds_quick_ball() {
  const sim::Scenario scenario = test_scenario();
  std::mt19937_64 rng(135606);
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, split_state());
  resolve_fss(engine);

  // Star Alchemy has supplied Fire, and two Treasures plus surplus Grass provide a
  // payable VSTAR bridge that preserves the second Treasure for Mega Dragonite ex.
  // Quick Ball into Oricorio cannot improve T3 and must wait for Treasure:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356
  expect(!sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball should hold for the pending Treasure-to-VSTAR bridge");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(has(result.hand, sim::Card::QuickBall), "Quick Ball must remain in hand");
  expect(has(result.deck, sim::Card::Oricorio), "Oricorio must remain in deck");
}

void test_resolved_fss_route_spends_surplus_grass() {
  const sim::Scenario scenario = test_scenario();
  std::mt19937_64 rng(135607);
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, split_state());
  resolve_fss(engine);

  // Fire is protected as the T3 attachment. The held Grass is surplus for the GG
  // Active and may pay Treasure while the second Treasure, Dragon payload, Quick
  // Ball, Oricorio, and Bench slot remain intact:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356
  expect(sim::EngineTestAccess::split_available(engine),
         "The post-Star Alchemy split route should be available");
  expect(sim::EngineTestAccess::play_treasure(engine),
         "The proved Treasure split route should resolve");
  expect(!sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball should still hold after Treasure supplies VSTAR");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(std::count(result.hand.begin(), result.hand.end(), sim::Card::MysteriousTreasure) == 1,
         "One Treasure must survive for the T3 payload");
  expect(has(result.hand, sim::Card::RegidragoVstar),
         "Treasure should search Regidrago VSTAR");
  expect(has(result.hand, sim::Card::Fire), "The T3 Fire attachment must survive");
  expect(has(result.hand, sim::Card::MegaDragonite), "The held Dragon payload must survive");
  expect(has(result.hand, sim::Card::QuickBall), "Quick Ball must remain unspent");
  expect(has(result.deck, sim::Card::Oricorio), "Oricorio must remain in deck");
  expect(has(result.discard, sim::Card::Grass), "The surplus Grass should pay Treasure");
}

void test_incomplete_split_keeps_vstar_priority() {
  for (const int control : {0, 1, 2}) {
    sim::State state = split_state();
    if (control == 0) {
      state.hand.erase(state.hand.begin());
    } else if (control == 1) {
      state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Grass));
    } else {
      state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Fire));
    }

    const sim::Scenario scenario = test_scenario();
    std::mt19937_64 rng(135610 + control);
    sim::Engine engine = make_engine(scenario, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    resolve_fss(engine);
    const sim::State& result = sim::EngineTestAccess::state(engine);

    // Without two Treasures, a surplus cost, or the missing Energy target, the
    // established Forest Seal Stone VSTAR route remains the legal priority:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1356
    expect(has(result.hand, sim::Card::RegidragoVstar),
           "Incomplete split controls must retain the original VSTAR target");
  }
}

void test_seed_107_complete_route() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(107);
  sim::Engine engine(scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& result = engine.state();

  // Exact public witness: same T3 readiness, Quick Ball and Oricorio preserved,
  // no support Pokémon consumes the Bench, and Treasure pays the surplus Grass:
  // https://github.com/FlareZ123/pokemon-sims/issues/1356
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://www.pokemon.com/us/pokemon-tcg/rules
  expect(outcome.first_ready_turn == 3, "Seed 107 must remain T3 ready");
  expect(result.bench.empty(), "Seed 107 must preserve the Bench slot");
  expect(has(result.hand, sim::Card::QuickBall), "Seed 107 must preserve Quick Ball");
  expect(has(result.deck, sim::Card::Oricorio), "Seed 107 must preserve Oricorio");
  expect(has(result.discard, sim::Card::Grass), "Seed 107 must pay surplus Grass");
}

}  // namespace

int main() {
  test_treasure_takes_vstar_and_fss_takes_fire();
  test_split_is_symmetric_for_missing_grass();
  test_pending_treasure_bridge_holds_quick_ball();
  test_resolved_fss_route_spends_surplus_grass();
  test_incomplete_split_keeps_vstar_priority();
  test_seed_107_complete_route();
  std::cout << "issue_1356_fss_treasure_energy_tests: all checks passed\n";
}
