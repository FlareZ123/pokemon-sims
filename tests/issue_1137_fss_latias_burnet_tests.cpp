#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static Card fss_target(const Engine& engine) {
    return engine.fss_target_after_search_started();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

const sim::DeckRecipe& recipe() {
  static const sim::DeckRecipe value = sim::baseline_recipe();
  return value;
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1137", sim::DciProfile::StrictJit, lock, true, 5};
}

sim::State projected_benched_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                   sim::Tool::ForestSealStone},
      sim::Pokemon{sim::Card::TapuLeleGX, 2}};
  state.hand = {sim::Card::Channeler, sim::Card::MegaDragonite,
                sim::Card::RegidragoVstar, sim::Card::ProfessorBurnet,
                sim::Card::Crispin};
  state.deck = {sim::Card::LatiasEx, sim::Card::BrilliantBlender,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Fire, sim::Card::Fire,
                sim::Card::Fire, sim::Card::Dragapult,
                sim::Card::GoodraVstar, sim::Card::EarthenVessel,
                sim::Card::TateLiza};
  state.prizes = {sim::Card::Powerglass, sim::Card::Lusamine,
                  sim::Card::ChaoticSwell, sim::Card::Arven,
                  sim::Card::QuickBall, sim::Card::DialgaGX};
  return state;
}

sim::Card target_for(sim::State state,
                     const sim::LockMode lock = sim::LockMode::None) {
  std::mt19937_64 rng{1137};
  const sim::Scenario selected_scenario = scenario(lock);
  sim::Engine engine(selected_scenario, recipe(), rng);
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));
  return sim::EngineTestAccess::fss_target(engine);
}

void test_prefers_latias_over_redundant_blender() {
  // Crispin plus the unused manual attachment leaves one Grass for a later turn.
  // The prior-turn Benched Regidrago V can evolve now, Latias ex resolves its
  // Active-position axis, and held Professor Burnet independently preserves the
  // deck-to-discard payload axis. Searching Blender would duplicate that role:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1137
  expect(target_for(projected_benched_vstar_state()) == sim::Card::LatiasEx,
         "Star Alchemy should select Latias over the redundant Blender payload axis");
}

void test_requires_deck_resident_latias() {
  sim::State state = projected_benched_vstar_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::LatiasEx), state.deck.end());
  // Star Alchemy can search only a card remaining in the inspected deck:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/issues/1137
  expect(target_for(std::move(state)) == sim::Card::BrilliantBlender,
         "An unavailable Latias must preserve the Blender target");
}

void test_rejects_rule_box_ability_lock() {
  // Skyliner is an Ability on a Rule Box Pokémon and is disabled by the modeled lock:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1137
  expect(target_for(projected_benched_vstar_state(),
                    sim::LockMode::FullRuleBoxAbility) ==
             sim::Card::BrilliantBlender,
         "Rule Box Ability lock must reject the Latias route");
}

void test_requires_bench_space() {
  sim::State state = projected_benched_vstar_state();
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1});
  state.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  state.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  // A player cannot Bench a sixth Pokémon:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1137
  expect(target_for(std::move(state)) == sim::Card::BrilliantBlender,
         "A full Bench must reject the Latias route");
}

void test_requires_prior_turn_regidrago() {
  sim::State state = projected_benched_vstar_state();
  state.bench.front().entered_turn = state.turn;
  // A Pokémon cannot evolve during the turn it entered play:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1137
  expect(target_for(std::move(state)) == sim::Card::BrilliantBlender,
         "A same-turn Regidrago V must reject the projected evolution route");
}

void test_requires_held_vstar() {
  sim::State state = projected_benched_vstar_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::RegidragoVstar), state.hand.end());
  state.deck.push_back(sim::Card::RegidragoVstar);
  // Latias cannot solve Active position for a VSTAR that cannot exist this turn:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1137
  expect(target_for(std::move(state)) == sim::Card::RegidragoVstar,
         "The missing VSTAR card axis must stay ahead of Latias");
}

void test_requires_next_turn_energy_inventory() {
  sim::State state = projected_benched_vstar_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::Grass), state.deck.end());
  state.deck.push_back(sim::Card::Grass);
  // Crispin consumes the only deck Grass, leaving no observable continuation for
  // Apex Dragon's second Grass requirement:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1137
  expect(target_for(std::move(state)) == sim::Card::BrilliantBlender,
         "Latias must require a real post-Crispin Energy continuation");
}

void test_requires_held_burnet_payload_outlet() {
  sim::State state = projected_benched_vstar_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::ProfessorBurnet), state.hand.end());
  // Without the held Burnet outlet, Blender is the direct deck-to-discard bridge:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/1137
  expect(target_for(std::move(state)) == sim::Card::BrilliantBlender,
         "The Latias override must require the independent held payload outlet");
}

void test_preserves_blender_for_active_regidrago_route() {
  sim::State state = projected_benched_vstar_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::ForestSealStone};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 2}};
  // The Active prior-turn Regidrago can evolve and Crispin plus the manual attachment
  // completes GGF now. Blender can then establish strict JIT without Supporter
  // contention, so Latias adds no missing axis:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1137
  expect(target_for(std::move(state)) == sim::Card::BrilliantBlender,
         "The direct Active-Regidrago Blender route must remain preferred");
}

}  // namespace

int main() {
  test_prefers_latias_over_redundant_blender();
  test_requires_deck_resident_latias();
  test_rejects_rule_box_ability_lock();
  test_requires_bench_space();
  test_requires_prior_turn_regidrago();
  test_requires_held_vstar();
  test_requires_next_turn_energy_inventory();
  test_requires_held_burnet_payload_outlet();
  test_preserves_blender_for_active_regidrago_route();
  return 0;
}
