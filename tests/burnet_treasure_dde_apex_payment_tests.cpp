#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>
#include <string_view>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static bool burnet_then_treasure_route_available(const Engine& engine) {
    return engine.issue_1221_burnet_then_treasure_route_available();
  }
};

}  // namespace sim

namespace {

bool route_available(const int grass, const int fire, const int double_dragon) {
  sim::Scenario scenario{"issue-3513-burnet-treasure-dde-apex-payment",
                         sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{3513};
  sim::Engine engine{scenario, recipe, rng};

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, grass, fire,
                              sim::Tool::None};
  state.active->double_dragon = double_dragon;
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::MysteriousTreasure,
                sim::Card::Channeler};
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::burnet_then_treasure_route_available(engine);
}

void expect_route(const int grass, const int fire, const int double_dragon,
                  const bool expected, const std::string_view label) {
  const bool actual = route_available(grass, fire, double_dragon);
  if (actual != expected) {
    throw std::runtime_error(std::string(label) +
                             (expected ? ": expected live route." : ": expected blocked route."));
  }
}

}  // namespace

int main() {
  // Double Dragon Energy supplies two Energy of every type while attached to a
  // Dragon Pokémon, so DDE plus either Basic Energy pays Apex Dragon's [G][G][R]
  // attack cost; DDE alone supplies only two total Energy units:
  // https://api.pokemontcg.io/v2/cards/xy6-97
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet and Mysterious Treasure establish the payload/evolution route:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // Rules procedures used by the modeled Supporter, Item, evolution, and attack path:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#B-01-Supporter-card-procedure
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#B-03-Item-card-procedure
  // https://github.com/FlareZ123/pokemon-sims/issues/3513
  expect_route(1, 0, 1, true, "DDE plus Grass");
  expect_route(0, 1, 1, true, "DDE plus Fire");
  expect_route(0, 0, 1, false, "DDE alone");
  expect_route(2, 1, 0, true, "ordinary GGF");

  std::cout << "Burnet -> Treasure Apex-payment route accepts semantic DDE payment and preserves ordinary GGF.\n";
  return 0;
}
