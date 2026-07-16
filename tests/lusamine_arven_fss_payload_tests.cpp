#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>
#include <string_view>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static std::vector<Card> lusamine_recovery_targets(const Engine& engine) {
    return engine.lusamine_recovery_targets();
  }
  static bool play_lusamine(Engine& engine) { return engine.play_lusamine(); }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool play_professor_burnet(Engine& engine) {
    return engine.play_professor_burnet();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"lusamine-arven-fss-payload", sim::DciProfile::StrictJit,
                         sim::LockMode::FullItem, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{705};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{scenario, recipe, rng, &trace};
};

sim::State live_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Lusamine};
  state.discard = {sim::Card::Arven, sim::Card::Guzma};
  state.deck = {sim::Card::ForestSealStone, sim::Card::ProfessorBurnet,
                sim::Card::MegaDragonite, sim::Card::Grass,
                sim::Card::Fire, sim::Card::ChaoticSwell};
  return state;
}

bool contains(const std::vector<sim::Card>& zone, const sim::Card card) {
  return std::find(zone.begin(), zone.end(), card) != zone.end();
}

void begin_test_turn(sim::State& state, const int turn) {
  state.turn = turn;
  state.supporter_used = false;
  state.manual_energy_used = false;
  state.retreat_used = false;
  state.turn_ended = false;
  state.discarded_this_turn.clear();
}

void expect_lusamine_rejected(sim::State state, const std::string_view label) {
  Fixture fixture;
  const std::vector<sim::Card> hand_before = state.hand;
  const std::vector<sim::Card> deck_before = state.deck;
  const std::vector<sim::Card> discard_before = state.discard;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (sim::EngineTestAccess::play_lusamine(fixture.engine)) {
    throw std::runtime_error(std::string(label) + ": Lusamine must reject the dead route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand != hand_before || after.deck != deck_before ||
      after.discard != discard_before) {
    throw std::runtime_error(std::string(label) + ": rejection must preserve every zone.");
  }
}

void test_full_item_lock_route_reaches_t4_payload() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, live_route_state());

  // Lusamine recovers exactly two Supporter and/or Stadium cards. Arven remains legal
  // through Item lock and may search Forest Seal Stone as its Pokémon Tool target:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  // Forest Seal Stone may attach to Pokémon VSTAR and Star Alchemy may search any card.
  // Professor Burnet then searches and discards the Dragon payload for Apex Dragon:
  // https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/705
  const std::vector<sim::Card> targets =
      sim::EngineTestAccess::lusamine_recovery_targets(fixture.engine);
  if (targets.size() != 2U || targets[0] != sim::Card::Arven ||
      targets[1] != sim::Card::Guzma) {
    throw std::runtime_error("Lusamine must recover Arven plus the deterministic second target.");
  }
  if (!sim::EngineTestAccess::play_lusamine(fixture.engine)) {
    throw std::runtime_error("Lusamine must start the legal delayed payload route.");
  }

  sim::State state = sim::EngineTestAccess::state(fixture.engine);
  begin_test_turn(state, 3);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (!sim::EngineTestAccess::play_arven(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::ForestSealStone)) {
    throw std::runtime_error("Arven must search Forest Seal Stone through Item lock.");
  }
  if (!sim::EngineTestAccess::attach_fss(fixture.engine) ||
      !sim::EngineTestAccess::use_fss(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::ProfessorBurnet)) {
    throw std::runtime_error("Star Alchemy must bank Professor Burnet for the next turn.");
  }

  state = sim::EngineTestAccess::state(fixture.engine);
  begin_test_turn(state, 4);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (!sim::EngineTestAccess::play_professor_burnet(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).discard,
                sim::Card::MegaDragonite)) {
    throw std::runtime_error("Professor Burnet must discard the modeled Dragon payload on T4.");
  }
}

void test_insufficient_horizon_blocks_item_lock_route() {
  sim::State state = live_route_state();
  state.turn = 3;
  // With max_turn=4, T3 Lusamine would leave only T4 for Arven. Professor Burnet
  // could not be played until T5, so the Item-lock payload chain is outside the
  // configured horizon and must remain unselected:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/issues/705
  expect_lusamine_rejected(std::move(state), "insufficient T3-to-T4 horizon");
}

void test_used_vstar_power_blocks_recovery() {
  sim::State state = live_route_state();
  state.vstar_power_used = true;
  expect_lusamine_rejected(std::move(state), "used VSTAR Power");
}

void test_occupied_tool_slot_blocks_recovery() {
  sim::State state = live_route_state();
  state.active->tool = sim::Tool::Powerglass;
  expect_lusamine_rejected(std::move(state), "occupied Tool slot");
}

void test_absent_forest_seal_stone_blocks_recovery() {
  sim::State state = live_route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::ForestSealStone),
                   state.deck.end());
  expect_lusamine_rejected(std::move(state), "absent Forest Seal Stone");
}

void test_absent_burnet_blocks_item_lock_payload_route() {
  sim::State state = live_route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::ProfessorBurnet),
                   state.deck.end());
  expect_lusamine_rejected(std::move(state), "absent Professor Burnet");
}

void test_absent_payload_blocks_recovery() {
  sim::State state = live_route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::MegaDragonite),
                   state.deck.end());
  expect_lusamine_rejected(std::move(state), "absent Dragon payload");
}

void test_lusamine_still_requires_two_targets() {
  sim::State state = live_route_state();
  state.discard = {sim::Card::Arven};
  expect_lusamine_rejected(std::move(state), "one Lusamine target");
}

}  // namespace

int main() {
  try {
    test_full_item_lock_route_reaches_t4_payload();
    test_insufficient_horizon_blocks_item_lock_route();
    test_used_vstar_power_blocks_recovery();
    test_occupied_tool_slot_blocks_recovery();
    test_absent_forest_seal_stone_blocks_recovery();
    test_absent_burnet_blocks_item_lock_payload_route();
    test_absent_payload_blocks_recovery();
    test_lusamine_still_requires_two_targets();
    std::cout << "Lusamine Arven-FSS payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
