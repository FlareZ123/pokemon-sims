#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool seen) { engine.deck_seen_ = seen; }
  static bool original_wonder_tag_route_has_live_target(Engine& engine) {
    return engine.original_wonder_tag_route_has_live_target();
  }
  static std::size_t recipe_size(const Engine& engine) { return engine.recipe_.size(); }
};

}  // namespace sim

namespace {

const sim::DeckRecipe& test_recipe() {
  // Engine stores the recipe by reference, so this owner must outlive every test Engine:
  // https://eel.is/c++draft/class.temporary#6.10
  // https://github.com/FlareZ123/pokemon-sims/issues/907
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return recipe;
}

sim::Engine make_engine() {
  static const sim::Scenario scenario{"issue-888", sim::DciProfile::StrictJit,
                                      sim::LockMode::None, false, 4};
  static std::mt19937_64 rng{888};
  return sim::Engine(scenario, test_recipe(), rng);
}

sim::State base_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None}};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Serena,
                sim::Card::MegaDragonite, sim::Card::RegidragoVstar,
                sim::Card::Fire, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::Dragapult,
                sim::Card::GoodraVstar, sim::Card::Grass};
  return state;
}

void test_recipe_reference_survives_factory_return() {
  sim::Engine engine = make_engine();

  // Reading the stored recipe after the factory returns detects the former dangling
  // reference under ASan with stack-use-after-return enabled:
  // https://eel.is/c++draft/class.temporary#6.10
  // https://github.com/FlareZ123/pokemon-sims/issues/907
  if (sim::EngineTestAccess::recipe_size(engine) != sim::baseline_recipe().size()) {
    throw std::runtime_error("The Engine recipe reference did not survive factory return.");
  }
}

void test_held_serena_payload_makes_burnet_target_redundant() {
  sim::Engine engine = make_engine();
  sim::EngineTestAccess::set_state(engine, base_state());
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // Wonder Tag costs the Tapu Lele-GX Bench slot and searches a Supporter.
  // Serena can already discard Mega Dragonite ex, so Professor Burnet does not
  // advance the current-turn Apex Dragon payload axis:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/888
  if (sim::EngineTestAccess::original_wonder_tag_route_has_live_target(engine)) {
    throw std::runtime_error("Wonder Tag treated redundant Professor Burnet as live.");
  }
}

sim::State held_item_state(const sim::Card item) {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::TapuLeleGX, item, sim::Card::MegaDragonite};
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::Grass,
                sim::Card::Dragapult, sim::Card::RegidragoV};
  return state;
}

void test_held_one_discard_item_makes_burnet_target_redundant() {
  for (const sim::Card item : {sim::Card::MysteriousTreasure,
                               sim::Card::QuickBall,
                               sim::Card::EarthenVessel}) {
    sim::Engine engine = make_engine();
    sim::EngineTestAccess::set_state(engine, held_item_state(item));
    sim::EngineTestAccess::set_deck_seen(engine, true);

    // Each Item can discard the held Mega Dragonite ex and has a known legal search
    // target. Wonder Tag into Burnet would spend a Bench slot and Supporter connector
    // without advancing another setup axis:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/942
    if (sim::EngineTestAccess::original_wonder_tag_route_has_live_target(engine)) {
      throw std::runtime_error(
          "Wonder Tag treated redundant Burnet as live beside a direct one-discard Item.");
    }
  }
}

void test_payable_ultra_ball_makes_burnet_target_redundant() {
  sim::Engine engine = make_engine();
  sim::State state = held_item_state(sim::Card::UltraBall);
  state.hand.push_back(sim::Card::Dipplin);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // Ultra Ball can discard Mega Dragonite ex plus Dipplin and search a known Pokémon.
  // Its exact two-other-card cost therefore makes the indirect Burnet target redundant:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://github.com/FlareZ123/pokemon-sims/issues/942
  if (sim::EngineTestAccess::original_wonder_tag_route_has_live_target(engine)) {
    throw std::runtime_error("Wonder Tag ignored a payable Ultra Ball payload route.");
  }
}

void test_dead_or_unpayable_item_keeps_burnet_live() {
  {
    sim::Engine engine = make_engine();
    sim::State state = held_item_state(sim::Card::EarthenVessel);
    state.deck = {sim::Card::ProfessorBurnet, sim::Card::Dragapult};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    sim::EngineTestAccess::set_deck_seen(engine, true);
    if (!sim::EngineTestAccess::original_wonder_tag_route_has_live_target(engine)) {
      throw std::runtime_error("A K1-dead Earthen Vessel should not suppress Burnet.");
    }
  }
  {
    sim::Engine engine = make_engine();
    sim::EngineTestAccess::set_state(engine, held_item_state(sim::Card::UltraBall));
    sim::EngineTestAccess::set_deck_seen(engine, true);
    if (!sim::EngineTestAccess::original_wonder_tag_route_has_live_target(engine)) {
      throw std::runtime_error("An Ultra Ball missing its second cost should not suppress Burnet.");
    }
  }
}

void test_burnet_remains_live_when_no_payload_is_held() {
  sim::Engine engine = make_engine();
  sim::State state = base_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite));
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // When no Dragon payload is held, Professor Burnet remains the direct
  // deck-to-discard bridge available through Wonder Tag:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  if (!sim::EngineTestAccess::original_wonder_tag_route_has_live_target(engine)) {
    throw std::runtime_error("The missing-payload control should keep Burnet live.");
  }
}

}  // namespace

int main() {
  test_recipe_reference_survives_factory_return();
  test_held_serena_payload_makes_burnet_target_redundant();
  test_held_one_discard_item_makes_burnet_target_redundant();
  test_payable_ultra_ball_makes_burnet_target_redundant();
  test_dead_or_unpayable_item_keeps_burnet_live();
  test_burnet_remains_live_when_no_payload_is_held();
  std::cout << "Wonder Tag redundant-Burnet tests passed\n";
}
