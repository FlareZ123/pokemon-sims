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
  static Card fss_target(const Engine& engine) {
    return engine.fss_target_after_search_started();
  }
  static bool split_available(const Engine& engine) {
    return engine.issue_1356_treasure_vstar_energy_split_available();
  }
  static bool play_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
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

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto found = std::find(cards.begin(), cards.end(), card);
  if (found == cards.end()) throw std::runtime_error("test fixture card missing");
  cards.erase(found);
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
                sim::Card::TapuLeleGX, sim::Card::Dragapult};
  return state;
}

sim::Card target_for(sim::State state, const std::uint64_t seed) {
  std::mt19937_64 rng(seed);
  sim::Engine engine(test_scenario(), test_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::fss_target(engine);
}

void test_treasure_takes_vstar_and_fss_takes_fire() {
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
  expect(target_for(split_state(), 135601) == sim::Card::Fire,
         "Star Alchemy should search the missing next-turn Fire Energy");
}

void test_split_is_symmetric_for_missing_grass() {
  sim::State state = split_state();
  state.active->grass = 1;
  state.active->fire = 1;
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite, sim::Card::Fire,
                sim::Card::QuickBall, sim::Card::Lusamine};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::TapuLeleGX, sim::Card::Dragapult};

  // Forest Seal Stone searches any card, so the connector split is symmetric when
  // Grass is the sole next-turn attachment and Fire is the surplus cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1356
  expect(target_for(std::move(state), 135602) == sim::Card::Grass,
         "Star Alchemy should search the missing next-turn Grass Energy");
}

void test_resolved_fss_route_spends_surplus_grass() {
  sim::State state = split_state();
  state.vstar_power_used = true;
  erase_one(state.deck, sim::Card::Fire);
  state.hand.push_back(sim::Card::Fire);

  std::mt19937_64 rng(135606);
  sim::Engine engine(test_scenario(), test_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // After Star Alchemy resolves, Fire is protected as the T3 attachment. The one
  // held Grass is surplus for the GG Active and may pay Treasure while the second
  // Treasure, Dragon payload, Quick Ball, and Bench route remain intact:
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
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(std::count(result.hand.begin(), result.hand.end(), sim::Card::MysteriousTreasure) == 1,
         "One Treasure must survive for the T3 payload");
  expect(has(result.hand, sim::Card::RegidragoVstar),
         "Treasure should search Regidrago VSTAR");
  expect(has(result.hand, sim::Card::Fire), "The T3 Fire attachment must survive");
  expect(has(result.hand, sim::Card::MegaDragonite), "The held Dragon payload must survive");
  expect(has(result.hand, sim::Card::QuickBall), "Quick Ball must remain unspent");
  expect(has(result.discard, sim::Card::Grass), "The surplus Grass should pay Treasure");
}

void test_one_treasure_keeps_vstar_priority() {
  sim::State state = split_state();
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::QuickBall, sim::Card::Lusamine};

  // One Treasure cannot search VSTAR now and remain as the next-turn strict-JIT
  // payload outlet, so the established Star Alchemy VSTAR priority remains:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1356
  expect(target_for(std::move(state), 135603) == sim::Card::RegidragoVstar,
         "A lone Treasure must not displace the VSTAR target");
}

void test_no_surplus_energy_keeps_vstar_priority() {
  sim::State state = split_state();
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};

  // Star Alchemy would add the required Fire, leaving no surplus off-type Energy.
  // The generic DCI correctly spends the duplicate Treasure, so the split is rejected:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1356
  expect(target_for(std::move(state), 135604) == sim::Card::RegidragoVstar,
         "The split must require a proved surplus Energy cost");
}

void test_missing_energy_target_keeps_vstar_priority() {
  sim::State state = split_state();
  state.deck = {sim::Card::RegidragoVstar, sim::Card::TapuLeleGX,
                sim::Card::Dragapult};

  // Star Alchemy cannot assign itself to an absent Basic Energy, so Forest Seal
  // Stone keeps the original Regidrago VSTAR target:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1356
  expect(target_for(std::move(state), 135605) == sim::Card::RegidragoVstar,
         "Absent Energy must preserve the original VSTAR target");
}

}  // namespace

int main() {
  test_treasure_takes_vstar_and_fss_takes_fire();
  test_split_is_symmetric_for_missing_grass();
  test_resolved_fss_route_spends_surplus_grass();
  test_one_treasure_keeps_vstar_priority();
  test_no_surplus_energy_keeps_vstar_priority();
  test_missing_energy_target_keeps_vstar_priority();
  std::cout << "issue_1356_fss_treasure_energy_tests: all checks passed\n";
}
