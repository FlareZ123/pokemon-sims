#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"blender-active-promotion-gate",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{725};
  sim::Engine engine{scenario, recipe, rng};
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State stranded_powered_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass};
  return state;
}

void expect_rejected(sim::State state, const std::string_view label) {
  Fixture fixture;
  const std::vector<sim::Card> hand_before = state.hand;
  const std::vector<sim::Card> deck_before = state.deck;
  const std::vector<sim::Card> discard_before = state.discard;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error(std::string(label) + ": Blender must be held.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand != hand_before || after.deck != deck_before ||
      after.discard != discard_before) {
    throw std::runtime_error(std::string(label) +
                             ": rejection must preserve the ACE SPEC and payload.");
  }
}

void expect_allowed(sim::State state, const std::string_view label) {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (!sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error(std::string(label) + ": Blender route must remain live.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (contains(after.hand, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error(std::string(label) +
                             ": legal Blender route did not resolve normally.");
  }
}

void test_stranded_powered_vstar_holds_blender() {
  // Brilliant Blender is a one-use ACE SPEC. A strict-JIT payload discarded while
  // the powered VSTAR is stranded on the Bench cannot satisfy the repository's
  // current-turn Active-ready predicate and expires at the next turn boundary:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect_rejected(stranded_powered_vstar_state(), "stranded powered VSTAR");
}

void test_star_alchemy_tate_route_keeps_blender_live() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::ForestSealStone};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::BrilliantBlender};
  state.deck = {sim::Card::TateLiza, sim::Card::MegaDragonite};

  // Forest Seal Stone's Star Alchemy can find Tate & Liza, which then promotes the
  // complete Benched VSTAR before Blender supplies the current-turn payload. The Tool
  // Ability remains usable through a Rule Box Ability lock:
  // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect_allowed(std::move(state), "Star Alchemy to Tate route");
}

void test_active_vstar_keeps_blender_live() {
  sim::State state = stranded_powered_vstar_state();
  state.active = state.bench.front();
  state.bench.clear();

  // The Active axis is already complete, so Blender may establish the remaining
  // current-turn Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect_allowed(std::move(state), "already Active VSTAR");
}

void test_live_latias_retreat_keeps_blender_live() {
  sim::State state = stranded_powered_vstar_state();
  state.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0,
                                     sim::Tool::None});

  // Skyliner gives the Basic Active no Retreat Cost, preserving the legal final
  // retreat to the powered Benched VSTAR after Blender resolves:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect_allowed(std::move(state), "live Latias retreat");
}

void test_held_latias_keeps_blender_live() {
  sim::State state = stranded_powered_vstar_state();
  state.hand.push_back(sim::Card::LatiasEx);

  // A held Latias ex can enter the open Bench before the final retreat step and
  // make the Basic Active's Retreat Cost zero:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect_allowed(std::move(state), "held Latias route");
}

void test_tate_switch_keeps_blender_live() {
  sim::State state = stranded_powered_vstar_state();
  state.hand.push_back(sim::Card::TateLiza);

  // Tate & Liza may promote the complete Benched VSTAR before the later Item pass
  // spends Blender for the same-turn payload:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect_allowed(std::move(state), "Tate switch route");
}

void test_turo_promotion_keeps_blender_live() {
  sim::State state = stranded_powered_vstar_state();
  state.hand.push_back(sim::Card::ProfessorTuro);

  // Professor Turo's Scenario may return the Basic Active after Blender establishes
  // the payload, forcing promotion of the complete Benched VSTAR:
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect_allowed(std::move(state), "Professor Turo promotion route");
}

void test_no_discard_control_keeps_payload_banking() {
  sim::Scenario scenario{"blender-no-discard-control",
                         sim::DciProfile::NoDiscardControl,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{726};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state = stranded_powered_vstar_state();
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // No-discard-control explicitly permits earlier payload banking, so the new
  // strict same-turn Active gate must not change that profile:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  if (!sim::EngineTestAccess::play_brilliant_blender(engine)) {
    throw std::runtime_error("No-discard-control Blender banking must remain legal.");
  }
}

}  // namespace

int main() {
  try {
    test_stranded_powered_vstar_holds_blender();
    test_star_alchemy_tate_route_keeps_blender_live();
    test_active_vstar_keeps_blender_live();
    test_live_latias_retreat_keeps_blender_live();
    test_held_latias_keeps_blender_live();
    test_tate_switch_keeps_blender_live();
    test_turo_promotion_keeps_blender_live();
    test_no_discard_control_keeps_payload_banking();
    std::cout << "Blender Active-promotion gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
