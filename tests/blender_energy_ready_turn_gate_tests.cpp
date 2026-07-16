#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  explicit Fixture(const sim::DciProfile dci = sim::DciProfile::StrictJit)
      : scenario{"blender-energy-ready-turn-gate", dci,
                 sim::LockMode::None, false, 4},
        recipe{sim::baseline_recipe()},
        rng{737},
        engine{scenario, recipe, rng} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State active_gf_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Grass,
                sim::Card::Fire};
  return state;
}

void expect_held(sim::State state, const std::string_view label) {
  Fixture fixture;
  const auto hand_before = state.hand;
  const auto deck_before = state.deck;
  const auto discard_before = state.discard;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error(std::string(label) + ": Blender must be held.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand != hand_before || after.deck != deck_before ||
      after.discard != discard_before) {
    throw std::runtime_error(std::string(label) +
                             ": rejection must preserve every resource.");
  }
}

void expect_played(sim::State state, const std::string_view label,
                   const sim::DciProfile dci = sim::DciProfile::StrictJit) {
  Fixture fixture{dci};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (!sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error(std::string(label) + ": Blender route must remain live.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error(std::string(label) +
                             ": Blender and its Dragon payload must reach discard.");
  }
}

void test_used_manual_attachment_holds_blender() {
  sim::State state = active_gf_vstar_state();
  state.manual_energy_used = true;
  state.hand.push_back(sim::Card::Grass);

  // Brilliant Blender is a one-shot ACE SPEC payload outlet. Apex Dragon costs GGF,
  // and a player may attach only one Energy from hand during the turn. Once that
  // attachment is used, an Active GF Regidrago VSTAR cannot become ready this turn:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_held(std::move(state), "used manual attachment");
}

void test_two_missing_energy_after_manual_attachment_holds_blender() {
  sim::State state = active_gf_vstar_state();
  state.active->fire = 0;
  state.manual_energy_used = true;
  state.hand.push_back(sim::Card::Crispin);

  // Crispin can find different Basic Energy types and attach one. With both Grass
  // and Fire still missing and the manual attachment spent, one Crispin attachment
  // cannot complete Apex Dragon's GGF cost during this turn:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_held(std::move(state), "two missing Energy after manual attachment");
}

void test_unused_manual_attachment_keeps_blender_live() {
  sim::State state = active_gf_vstar_state();
  state.hand.push_back(sim::Card::Grass);

  // One unused manual attachment of the held Grass completes GGF this turn:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_played(std::move(state), "unused manual attachment");
}

void test_crispin_attachment_keeps_blender_live() {
  sim::State state = active_gf_vstar_state();
  state.manual_energy_used = true;
  state.hand.push_back(sim::Card::Crispin);

  // With Grass and Fire available, Crispin may attach the missing Grass directly,
  // so the spent manual attachment does not close this same-turn GGF route:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_played(std::move(state), "Crispin direct attachment");
}

void test_vital_dance_keeps_blender_live() {
  sim::State state = active_gf_vstar_state();
  state.hand.push_back(sim::Card::Oricorio);

  // Playing Oricorio from hand can use Vital Dance to find the missing Basic Grass,
  // which the unused manual attachment then places on Regidrago VSTAR:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_played(std::move(state), "Vital Dance plus manual attachment");
}

void test_earthen_vessel_keeps_blender_live() {
  sim::State state = active_gf_vstar_state();
  state.hand.push_back(sim::Card::EarthenVessel);
  state.hand.push_back(sim::Card::Dipplin);

  // Earthen Vessel has a payable other-card discard cost and can search the missing
  // Basic Grass before the unused manual attachment completes GGF:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_played(std::move(state), "payable Earthen Vessel");
}

void test_live_star_alchemy_keeps_blender_live() {
  sim::State state = active_gf_vstar_state();
  state.active->tool = sim::Tool::ForestSealStone;

  // Star Alchemy on an attached Forest Seal Stone can search the missing Grass,
  // leaving the unused manual attachment available before Blender resolves:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_played(std::move(state), "live Star Alchemy Energy search");
}

void test_no_discard_control_banking_remains_unchanged() {
  sim::State state = active_gf_vstar_state();
  state.manual_energy_used = true;

  // The Energy-ready-turn gate belongs only to strict current-turn payload timing.
  // The no-discard-control profile intentionally permits earlier payload banking:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_played(std::move(state), "no-discard-control banking",
                sim::DciProfile::NoDiscardControl);
}

}  // namespace

int main() {
  test_used_manual_attachment_holds_blender();
  test_two_missing_energy_after_manual_attachment_holds_blender();
  test_unused_manual_attachment_keeps_blender_live();
  test_crispin_attachment_keeps_blender_live();
  test_vital_dance_keeps_blender_live();
  test_earthen_vessel_keeps_blender_live();
  test_live_star_alchemy_keeps_blender_live();
  test_no_discard_control_banking_remains_unchanged();
  return 0;
}
