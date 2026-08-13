#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = false;
  }
  static bool next_turn_route(const Engine& engine) {
    return engine.issue_3520_next_turn_treasure_completion_available();
  }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool bank_flag(const Engine& engine) {
    return engine.issue_1598_bank_prized_treasure_;
  }
  static bool should_bank(const Engine& engine) {
    return engine.issue_3520_should_bank_recovered_treasure();
  }
  static bool play_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
};
}  // namespace sim

namespace {
void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(sim::Card held_basic, int double_dragon = 1) {
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0};
  state.active->double_dragon = double_dragon;
  state.hand = {sim::Card::Gladion, held_basic, sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Dipplin,
                  sim::Card::Channeler, sim::Card::ErikasInvitation,
                  sim::Card::Guzma, sim::Card::PathToPeak};
  return state;
}

void test_dde_grass_and_fire() {
  // DDE supplies two Energy of every type to a Dragon, so either Basic Grass or
  // Basic Fire completes Apex Dragon [G][G][R]. Gladion can recover the known
  // prized Treasure, whose discard cost supplies the strict-JIT Dragon payload.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Bug: https://github.com/FlareZ123/pokemon-sims/issues/3520
  for (sim::Card basic : {sim::Card::Grass, sim::Card::Fire}) {
    std::mt19937_64 rng{3520};
    sim::Scenario scenario{"issue-3520", sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 4};
    sim::Engine engine{scenario, sim::baseline_recipe(), rng};
    sim::EngineTestAccess::set_state(engine, route_state(basic));
    expect(sim::EngineTestAccess::next_turn_route(engine),
           "DDE plus completing Basic was not admitted.");
    expect(sim::EngineTestAccess::play_gladion(engine),
           "Gladion did not recover the prized Treasure.");
    expect(sim::EngineTestAccess::bank_flag(engine),
           "Gladion did not set the deferred Treasure flag.");
    expect(sim::EngineTestAccess::should_bank(engine),
           "Recovered Treasure was not protected.");
    expect(!sim::EngineTestAccess::play_treasure(engine),
           "Recovered Treasure was spent before the next-turn attachment.");
  }
}

void test_canonical_route() {
  // Preserve the original GG plus held-Fire issue-1598 witness.
  // https://github.com/FlareZ123/pokemon-sims/issues/1598
  // https://github.com/FlareZ123/pokemon-sims/issues/3520
  std::mt19937_64 rng{1598};
  sim::Scenario scenario{"issue-1598-control", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::State state = route_state(sim::Card::Fire, 0);
  state.active->grass = 2;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(sim::EngineTestAccess::play_gladion(engine),
         "Canonical Gladion route regressed.");
  expect(sim::EngineTestAccess::bank_flag(engine),
         "Canonical Treasure bank flag regressed.");
  expect(!sim::EngineTestAccess::play_treasure(engine),
         "Canonical Treasure bank guard regressed.");
}

void test_boundaries() {
  // DDE attaches only to Dragon Pokemon; Treasure still needs an unlocked Item
  // window and a remaining searchable VSTAR.
  // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Bug: https://github.com/FlareZ123/pokemon-sims/issues/3520
  {
    std::mt19937_64 rng{1};
    sim::Scenario scenario{"no-basic", sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 4};
    sim::Engine engine{scenario, sim::baseline_recipe(), rng};
    sim::State state = route_state(sim::Card::Fire);
    state.hand = {sim::Card::Gladion, sim::Card::MegaDragonite};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::next_turn_route(engine),
           "Route admitted DDE without a completing Basic.");
  }
  {
    std::mt19937_64 rng{2};
    sim::Scenario scenario{"non-dragon", sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 4};
    sim::Engine engine{scenario, sim::baseline_recipe(), rng};
    sim::State state = route_state(sim::Card::Fire);
    state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0};
    state.active->double_dragon = 1;
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::next_turn_route(engine),
           "Route admitted DDE on a non-Dragon Active.");
  }
  {
    std::mt19937_64 rng{3};
    sim::Scenario scenario{"no-vstar", sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 4};
    sim::Engine engine{scenario, sim::baseline_recipe(), rng};
    sim::State state = route_state(sim::Card::Fire);
    state.deck = {sim::Card::Grass, sim::Card::Fire};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::next_turn_route(engine),
           "Route admitted no-searchable-VSTAR state.");
  }
  {
    std::mt19937_64 rng{4};
    sim::Scenario scenario{"item-lock", sim::DciProfile::StrictJit,
                           sim::LockMode::FullItem, false, 4};
    sim::Engine engine{scenario, sim::baseline_recipe(), rng};
    sim::EngineTestAccess::set_state(engine, route_state(sim::Card::Fire));
    expect(!sim::EngineTestAccess::next_turn_route(engine),
           "Route ignored full Item lock.");
  }
}
}  // namespace

int main() {
  try {
    test_dde_grass_and_fire();
    test_canonical_route();
    test_boundaries();
    std::cout << "Issue 3520 DDE prized-Treasure tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
