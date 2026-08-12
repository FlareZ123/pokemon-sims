#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess3216 {
  static void set_state(Engine& engine, State state, const bool banked,
                        const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.issue_1022_banked_route_ = banked;
  }

  static bool opening_visible(const Engine& engine) {
    return engine.wonder_tag_can_bank_steven_for_known_t4_fss_route();
  }

  static bool banked_visible(const Engine& engine) {
    return engine.banked_steven_has_known_t4_fss_route();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State opening_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Arven, sim::Card::BrilliantBlender,
                sim::Card::Grass, sim::Card::MegaDragonite};
  state.deck = {sim::Card::StevensResolve, sim::Card::RegidragoV,
                sim::Card::Crispin, sim::Card::Grass, sim::Card::Fire,
                sim::Card::ForestSealStone, sim::Card::RegidragoVstar};
  state.prizes = {sim::Card::FieldBlower, sim::Card::Serena,
                  sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::State banked_state() {
  sim::State state = opening_state();
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 1, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::Arven,
                sim::Card::BrilliantBlender, sim::Card::MegaDragonite};
  return state;
}

bool opening_visible(const sim::DciProfile dci, const sim::LockMode lock) {
  const sim::Scenario scenario{"issue-3216-opening", dci, lock, true, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3216001);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess3216::set_state(engine, opening_state(), false);
  return sim::EngineTestAccess3216::opening_visible(engine);
}

bool banked_visible(const sim::DciProfile dci, const sim::LockMode lock) {
  const sim::Scenario scenario{"issue-3216-banked", dci, lock, true, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3216002);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess3216::set_state(engine, banked_state(), true);
  return sim::EngineTestAccess3216::banked_visible(engine);
}

void test_matchup_flex_admission() {
  // Brilliant Blender creates the Dragon payload on the projected ready turn,
  // so StrictJit and MatchupFlexJit share the same physical timing requirement:
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Same-ready-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Advanced turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3216
  expect(opening_visible(sim::DciProfile::MatchupFlexJit, sim::LockMode::None),
         "MatchupFlexJit hid the opening T4 FSS bank route");
  expect(banked_visible(sim::DciProfile::MatchupFlexJit, sim::LockMode::None),
         "MatchupFlexJit hid the already-banked T4 FSS route");
}

void test_semantic_boundaries() {
  // NoDiscardControl has a different payload-timing policy. Rule Box Ability lock
  // disables the opening Wonder Tag/Regidrago VSTAR Ability-dependent package, and
  // scheduled Item lock blocks the T4 Forest Seal Stone/Blender continuation:
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Scenario lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3216
  expect(!opening_visible(sim::DciProfile::NoDiscardControl, sim::LockMode::None),
         "NoDiscardControl entered the same-ready-turn JIT route");
  expect(!opening_visible(sim::DciProfile::MatchupFlexJit,
                          sim::LockMode::FullRuleBoxAbility),
         "Rule Box Ability lock admitted the opening Wonder Tag/FSS package");
  expect(!banked_visible(sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::TurnTwoItem),
         "Scheduled Item lock admitted the T4 FSS/Blender continuation");
}

}  // namespace

int main() {
  test_matchup_flex_admission();
  test_semantic_boundaries();
  return 0;
}
