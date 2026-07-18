#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
  static bool play_mysterious_treasure(Engine& engine, bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool play_quick_ball(Engine& engine, bool permit_payload) {
    return engine.play_quick_ball(permit_payload);
  }
  static bool play_ultra_ball(Engine& engine, bool permit_payload) {
    return engine.play_ultra_ball(permit_payload);
  }
  static bool run_search_items_one_step(Engine& engine, bool permit_payload) {
    return engine.run_search_items_one_step(permit_payload);
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool might_be_unseen(const Engine& engine, const Card card) {
    return engine.might_be_unseen(card);
  }
  static bool vstar_can_exist_this_turn(const Engine& engine) {
    return engine.vstar_can_exist_this_turn();
  }
  static bool can_play_payload_this_turn(const Engine& engine) {
    return engine.can_play_payload_this_turn();
  }
  static bool has_live_blender_payload_line(const Engine& engine) {
    return engine.has_live_blender_payload_line();
  }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"known-dead-payload", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const std::uint64_t seed) : rng(seed), engine(scenario, recipe, rng) {}
};

sim::State known_no_payload_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.deck = {sim::Card::TapuLeleGX};
  state.prizes = {sim::Card::MegaDragonite, sim::Card::Dragapult, sim::Card::GoodraVstar,
                  sim::Card::DialgaGX, sim::Card::Dipplin, sim::Card::ForestSealStone};
  return state;
}

void test_blender_yields_to_live_hand_discard_after_k1() {
  Fixture fixture{199};
  sim::State state = known_no_payload_state();
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::BrilliantBlender,
                sim::Card::MysteriousTreasure, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Hisuian Heavy Ball reveals the Prize cards, establishing K1 knowledge:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  if (!sim::EngineTestAccess::play_heavy_ball(fixture.engine)) {
    throw std::runtime_error("Heavy Ball should establish K1 before the payload-outlet choice.");
  }
  if (!sim::EngineTestAccess::run_search_items_one_step(fixture.engine, true)) {
    throw std::runtime_error("Mysterious Treasure should remain a live hand-discard payload route.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  // Brilliant Blender discards selected cards from deck, so a K1-proven empty
  // payload deck makes Blender a dead route: https://api.pokemontcg.io/v2/cards/sv8-164
  if (!contains(after.hand, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error("K1 should hold Blender and use the live Mysterious Treasure hand-discard route.");
  }
}

void test_dead_outlet_does_not_make_payload_search_live_after_k1() {
  Fixture fixture{201};
  sim::State state = known_no_payload_state();
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::MysteriousTreasure,
                sim::Card::QuickBall, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (!sim::EngineTestAccess::play_heavy_ball(fixture.engine)) {
    throw std::runtime_error("Heavy Ball should establish K1 before evaluating a payload connector.");
  }
  // Mysterious Treasure can only turn into a payload setup route when a payload
  // can still be found in deck: https://api.pokemontcg.io/v2/cards/sm6-113
  if (sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, false)) {
    throw std::runtime_error("A dead Quick Ball outlet must not justify spending Mysterious Treasure after K1.");
  }
  if (!contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("The known-dead payload connector should remain in hand.");
  }
}

void test_burnet_yields_supporter_slot_after_k1() {
  Fixture fixture{200};
  sim::State state = known_no_payload_state();
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::ProfessorBurnet,
                sim::Card::Serena, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (!sim::EngineTestAccess::play_heavy_ball(fixture.engine)) {
    throw std::runtime_error("Heavy Ball should establish K1 before Supporter selection.");
  }
  sim::EngineTestAccess::choose_supporter(fixture.engine);

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  // Professor Burnet can only discard cards it searches from deck:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena can discard the in-hand payload for its draw mode:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  if (!contains(after.hand, sim::Card::ProfessorBurnet) ||
      !contains(after.discard, sim::Card::Serena) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error("K1 should hold dead Burnet and select Serena's live hand-discard route.");
  }
}

void test_mysterious_treasure_holds_when_vstar_is_known_absent() {
  Fixture fixture{202};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dipplin};
  state.deck = {sim::Card::LatiasEx};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::MawileGX, sim::Card::Guzma, sim::Card::Channeler};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Mysterious Treasure searches the deck only after discarding a card:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  if (sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, false)) {
    throw std::runtime_error("K1 must not spend Treasure and its cost when VSTAR is absent from deck.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::MysteriousTreasure) || !contains(after.hand, sim::Card::Dipplin)) {
    throw std::runtime_error("Known-dead Treasure route must preserve both hand cards.");
  }
}

void test_quick_ball_holds_when_regidrago_is_known_absent() {
  Fixture fixture{203};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::QuickBall, sim::Card::Dipplin};
  state.deck = {sim::Card::MawileGX};
  state.prizes = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::RegidragoVstar, sim::Card::Guzma, sim::Card::Channeler};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Quick Ball searches the deck for a Basic only after discarding another card:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  if (sim::EngineTestAccess::play_quick_ball(fixture.engine, false)) {
    throw std::runtime_error("K1 must not spend Quick Ball and its cost when Regidrago V is absent from deck.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::QuickBall) || !contains(after.hand, sim::Card::Dipplin)) {
    throw std::runtime_error("Known-dead Quick Ball route must preserve both hand cards.");
  }
}

void test_ultra_ball_holds_when_vstar_is_known_absent() {
  Fixture fixture{204};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::MawileGX};
  state.deck = {sim::Card::LatiasEx};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::QuickBall, sim::Card::Guzma, sim::Card::Channeler};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Ultra Ball searches the deck only after discarding 2 other cards:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  if (sim::EngineTestAccess::play_ultra_ball(fixture.engine, false)) {
    throw std::runtime_error("K1 must not spend Ultra Ball and two costs when VSTAR is absent from deck.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand.size() != 3U || !contains(after.hand, sim::Card::UltraBall)) {
    throw std::runtime_error("Known-dead Ultra Ball route must preserve all three hand cards.");
  }
}

void test_k0_mysterious_treasure_keeps_plausible_vstar_route() {
  Fixture fixture{205};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dipplin};
  state.deck = {sim::Card::LatiasEx};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Before legal inspection, fixed-list K0 policy may still treat VSTAR as unseen.
  // Playing Treasure then establishes deck knowledge under its printed search:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  if (!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, false)) {
    throw std::runtime_error("K0 must retain the plausible fixed-list Treasure route.");
  }
}

void test_k0_counts_the_regidrago_v_under_an_evolved_vstar() {
  Fixture fixture{226};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::RegidragoV};
  state.discard = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The Regidrago V beneath an evolved Regidrago VSTAR remains in the public
  // Evolution stack, so all four fixed-list Basic copies are accounted for:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (sim::EngineTestAccess::might_be_unseen(fixture.engine, sim::Card::RegidragoV)) {
    throw std::runtime_error("K0 must not invent a fifth Regidrago V after one copy evolved into VSTAR.");
  }
}

void test_strict_jit_holds_payload_when_no_vstar_can_exist() {
  Fixture fixture{721};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 2}};
  state.hand = {sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // A Pokémon cannot evolve during the turn it entered play. Strict JIT counts a
  // payload only on the same turn that readiness is created, so Blender must remain
  // held while no Regidrago VSTAR can legally exist:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/721
  if (sim::EngineTestAccess::vstar_can_exist_this_turn(fixture.engine) ||
      sim::EngineTestAccess::can_play_payload_this_turn(fixture.engine) ||
      sim::EngineTestAccess::has_live_blender_payload_line(fixture.engine) ||
      sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error("Strict JIT must hold every payload route when no VSTAR can exist this turn.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::BrilliantBlender) ||
      !contains(after.deck, sim::Card::MegaDragonite) || !after.discard.empty()) {
    throw std::runtime_error("The held Blender route must leave hand, deck, and discard unchanged.");
  }
}

void test_strict_jit_keeps_payload_live_for_prior_turn_regidrago() {
  Fixture fixture{722};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::BrilliantBlender, sim::Card::RegidragoVstar};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // A prior-turn Regidrago V plus the held Regidrago VSTAR card completes the
  // current-turn evolution axis, so the #721 existence control remains live after
  // #718 adds the separate card-access boundary:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/721
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  if (!sim::EngineTestAccess::vstar_can_exist_this_turn(fixture.engine) ||
      !sim::EngineTestAccess::can_play_payload_this_turn(fixture.engine) ||
      !sim::EngineTestAccess::has_live_blender_payload_line(fixture.engine) ||
      !sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error(
        "A prior-turn Regidrago V with its held VSTAR card must keep the strict-JIT payload route live.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The positive control should consume Blender and discard its payload.");
  }
}

}  // namespace

int main() {
  test_blender_yields_to_live_hand_discard_after_k1();
  test_dead_outlet_does_not_make_payload_search_live_after_k1();
  test_burnet_yields_supporter_slot_after_k1();
  test_mysterious_treasure_holds_when_vstar_is_known_absent();
  test_quick_ball_holds_when_regidrago_is_known_absent();
  test_ultra_ball_holds_when_vstar_is_known_absent();
  test_k0_mysterious_treasure_keeps_plausible_vstar_route();
  test_k0_counts_the_regidrago_v_under_an_evolved_vstar();
  test_strict_jit_holds_payload_when_no_vstar_can_exist();
  test_strict_jit_keeps_payload_live_for_prior_turn_regidrago();
  return 0;
}
