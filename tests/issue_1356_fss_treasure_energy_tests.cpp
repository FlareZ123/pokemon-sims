#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static Card diagnosed_fss_target(Engine& engine) {
    const Card missing_energy = engine.grass_needed() == 1 ? Card::Grass : Card::Fire;
    const std::size_t payloads_before = static_cast<std::size_t>(std::count_if(
        engine.state_.hand.begin(), engine.state_.hand.end(), is_payload));
    std::mt19937_64 shadow_rng = engine.rng_;
    Engine projected(engine.scenario_, engine.recipe_, shadow_rng);
    projected.state_ = engine.state_;
    projected.deck_seen_ = engine.deck_seen_;
    projected.prizes_revealed_ = engine.prizes_revealed_;
    projected.state_.vstar_power_used = true;
    const bool energy_moved = projected.move_deck_to_hand(missing_energy);
    const bool treasure_played = energy_moved && projected.play_mysterious_treasure(false);
    const std::size_t payloads_after = static_cast<std::size_t>(std::count_if(
        projected.state_.hand.begin(), projected.state_.hand.end(), is_payload));
    std::cerr << "split="
              << engine.fss_should_split_treasure_vstar_and_next_turn_energy()
              << " strict=" << engine.strict_payload_timing()
              << " used=" << engine.state_.vstar_power_used
              << " item_lock=" << engine.item_locked()
              << " turn=" << engine.state_.turn
              << " max_turn=" << engine.scenario_.max_turn
              << " manual_used=" << engine.state_.manual_energy_used
              << " need_vstar=" << engine.need_vstar()
              << " need_energy=" << engine.need_energy()
              << " grass_needed=" << engine.grass_needed()
              << " fire_needed=" << engine.fire_needed()
              << " active=" << (engine.state_.active ? name(engine.state_.active->card) : "none")
              << " entered=" << (engine.state_.active ? engine.state_.active->entered_turn : -1)
              << " mt_hand=" << engine.hand_count(Card::MysteriousTreasure)
              << " vstar_deck=" << engine.deck_count_after_search_started(Card::RegidragoVstar)
              << " fire_deck=" << engine.deck_count_after_search_started(Card::Fire)
              << " grass_deck=" << engine.deck_count_after_search_started(Card::Grass)
              << " payloads_before=" << payloads_before
              << " energy_moved=" << energy_moved
              << " shadow_played=" << treasure_played
              << " shadow_vstar_hand=" << projected.hand_count(Card::RegidragoVstar)
              << " shadow_mt_hand=" << projected.hand_count(Card::MysteriousTreasure)
              << " shadow_energy_hand=" << projected.hand_count(missing_energy)
              << " payloads_after=" << payloads_after << '\n';
    return engine.fss_target_after_search_started();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

const sim::DeckRecipe& test_recipe() {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return recipe;
}

sim::State split_state() {
  sim::State state;
  state.turn = 2;
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
  const sim::Scenario scenario{"issue-1356-fss-treasure-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 3};
  std::mt19937_64 rng(seed);
  sim::Engine engine(scenario, test_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::diagnosed_fss_target(engine);
}

void test_treasure_takes_vstar_and_fss_takes_fire() {
  // The exact public seed-107 hand retains Quick Ball and Lusamine as legal DCI costs.
  // One Treasure can search Regidrago VSTAR while the second survives to discard the
  // held Dragon on T3. Star Alchemy should therefore search Fire instead of duplicating VSTAR:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
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

  // Forest Seal Stone searches any card, so the same connector allocation applies
  // when Grass is the sole next-turn attachment instead of Fire:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1356
  expect(target_for(std::move(state), 135602) == sim::Card::Grass,
         "Star Alchemy should search the missing next-turn Grass Energy");
}

void test_one_treasure_keeps_vstar_priority() {
  sim::State state = split_state();
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::QuickBall, sim::Card::Lusamine};

  // One Treasure cannot both search VSTAR now and remain as the next-turn strict-JIT
  // payload outlet, so the established Star Alchemy VSTAR priority must remain:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1356
  expect(target_for(std::move(state), 135603) == sim::Card::RegidragoVstar,
         "A lone Treasure must not displace the VSTAR target");
}

void test_unpayable_preservation_route_keeps_vstar_priority() {
  sim::State state = split_state();
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};

  // Projecting the real DCI selector proves that spending the second Treasure as the
  // first Treasure's cost leaves no held outlet. The split route is therefore dead:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1356
  expect(target_for(std::move(state), 135604) == sim::Card::RegidragoVstar,
         "The split must require a payable cost that preserves the second Treasure");
}

void test_missing_energy_target_keeps_vstar_priority() {
  sim::State state = split_state();
  state.deck = {sim::Card::RegidragoVstar, sim::Card::TapuLeleGX,
                sim::Card::Dragapult};

  // Star Alchemy cannot assign itself to an absent Basic Energy. Treasure therefore
  // remains a possible later connector while Forest Seal Stone takes Regidrago VSTAR:
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
  test_one_treasure_keeps_vstar_priority();
  test_unpayable_preservation_route_keeps_vstar_priority();
  test_missing_energy_target_keeps_vstar_priority();
  std::cout << "issue_1356_fss_treasure_energy_tests: all checks passed\n";
}
