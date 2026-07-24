from __future__ import annotations

import os
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def update_policy() -> None:
    path = ROOT / "src" / "trace_engine_v2" / "part_turo_oricorio_override.inc"
    source = path.read_text(encoding="utf-8")
    old = """    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    if (deck_seen_ || prizes_revealed_) {
"""
    new = """    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;

    // The held-route classifier may prove completion from public held cards, while
    // unresolved Dark Asset cards remain probabilistic until Crobat V is played.
    // Disable a new shadow Dark Asset resolution; a Dark Asset that already resolved
    // in the real state remains represented by its public hand and board result:
    // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Core Bench, Ability, draw, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Future-card-oracle boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Crobat probabilistic-connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#crobat-v-draw-connector-policy
    // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1526
    projected.state_.dark_asset_used = true;
    if (deck_seen_ || prizes_revealed_) {
"""
    if source.count(old) != 1:
        raise RuntimeError("Expected one Tate held-route shadow initialization")
    atomic_write(path, source.replace(old, new, 1))


def write_regression() -> None:
    path = ROOT / "tests" / "issue_1526_tate_dark_asset_tests.cpp"
    content = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = false;
  }

  static void play_basics(Engine& engine) {
    engine.play_basics_from_hand();
  }

  static void attach_manual(Engine& engine) {
    engine.attach_manual();
  }

  static bool tate_route_completes(const Engine& engine) {
    return engine.tate_draw_has_held_non_supporter_completion();
  }

  static const State& state(const Engine& engine) {
    return engine.state_;
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::DeckRecipe issue_recipe() {
  // Fixed-copy K0 projection leaves Regidrago VSTAR followed by Crispin in the
  // synthetic zone. Dark Asset draws from the back, so one legal draw sees only
  // Crispin while the shadow's extra second draw reaches Regidrago VSTAR:
  // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K0 projection contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1526
  return {{sim::Card::RegidragoV, 1},     {sim::Card::CrobatV, 1},
          {sim::Card::TateLiza, 1},       {sim::Card::Grass, 6},
          {sim::Card::Fire, 1},           {sim::Card::MegaDragonite, 1},
          {sim::Card::RegidragoVstar, 1}, {sim::Card::Crispin, 1}};
}

sim::State seven_card_pre_attachment_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1};
  state.hand = {sim::Card::TateLiza, sim::Card::CrobatV,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  return state;
}

void test_extra_shadow_dark_asset_draw_cannot_prove_completion() {
  const sim::Scenario scenario{"issue-1526", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = issue_recipe();
  std::mt19937_64 rng{1526};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, seven_card_pre_attachment_state());

  // At seven cards Crobat V cannot draw after leaving the hand. The real turn first
  // attaches Grass, reaches six cards at choose_supporter(), and can draw only one
  // card if Tate & Liza is held. Removing Tate before the shadow turn would instead
  // let Crobat draw two synthetic cards and reach the unavailable second VSTAR card:
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Manual attachment, Bench, Ability, draw, Supporter, and evolution procedure:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1526
  sim::EngineTestAccess::play_basics(engine);
  expect(sim::EngineTestAccess::state(engine).hand.size() == 7U,
         "Crobat V was played before the seven-card attachment transition");
  expect(!sim::EngineTestAccess::state(engine).dark_asset_used,
         "Dark Asset resolved before the reported Supporter decision");

  sim::EngineTestAccess::attach_manual(engine);
  expect(sim::EngineTestAccess::state(engine).hand.size() == 6U,
         "Manual attachment did not create the six-card Tate decision");
  expect(sim::EngineTestAccess::state(engine).active->grass == 2,
         "The legal Grass attachment did not complete GGF");
  expect(!sim::EngineTestAccess::tate_route_completes(engine),
         "Tate classifier still used unresolved Dark Asset cards as deterministic proof");
}

sim::State deterministic_vstar_state(const bool dark_asset_resolved) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1};
  state.hand = {sim::Card::TateLiza, sim::Card::RegidragoVstar,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass};
  state.deck = {sim::Card::Crispin};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.manual_energy_used = true;
  state.dark_asset_used = dark_asset_resolved;
  if (dark_asset_resolved) {
    state.bench = {sim::Pokemon{sim::Card::CrobatV, 2}};
  } else {
    state.hand.push_back(sim::Card::CrobatV);
  }
  return state;
}

void test_public_held_vstar_completion_with_unresolved_crobat_remains_live() {
  const sim::Scenario scenario{"issue-1526-public", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = issue_recipe();
  std::mt19937_64 rng{152601};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, deterministic_vstar_state(false));

  // The held VSTAR is public and deterministic. An unused Crobat V in the same hand
  // cannot make that known evolution route probabilistic or force Tate & Liza to be spent:
  // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Earliest deterministic route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1526
  expect(sim::EngineTestAccess::tate_route_completes(engine),
         "Public held VSTAR completion was suppressed by an unused Crobat V");
}

void test_completion_after_real_dark_asset_resolution_remains_live() {
  const sim::Scenario scenario{"issue-1526-resolved", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = issue_recipe();
  std::mt19937_64 rng{152602};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, deterministic_vstar_state(true));

  // Once Dark Asset has legally resolved in the real public state, the held VSTAR is
  // known and the classifier may preserve the deterministic continuation:
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Ability, draw, Supporter, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1526
  expect(sim::EngineTestAccess::tate_route_completes(engine),
         "A legally resolved Dark Asset state lost its deterministic held completion");
}

}  // namespace

int main() {
  test_extra_shadow_dark_asset_draw_cannot_prove_completion();
  test_public_held_vstar_completion_with_unresolved_crobat_remains_live();
  test_completion_after_real_dark_asset_resolution_remains_live();
  return 0;
}
'''
    atomic_write(path, content)


def main() -> int:
    update_policy()
    write_regression()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
