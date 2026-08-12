#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1875_quick_ball_tapu_crispin_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1875_quick_ball_tapu_crispin_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn = 3) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, turn, 1, 0};
  state.hand = {sim::Card::Fire, sim::Card::QuickBall, sim::Card::Dragapult};
  state.deck = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire,
                sim::Card::Crispin, sim::Card::TapuLeleGX};
  return state;
}

sim::Scenario scenario(const char* label, const sim::DciProfile dci,
                       const sim::LockMode locks, const bool going_first) {
  return sim::Scenario{label, dci, locks, going_first, 5};
}

void test_route_uses_shared_jit_timing_semantics() {
  std::mt19937_64 rng(2744);
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario strict_scenario =
      scenario("issue-2744-strict", sim::DciProfile::StrictJit,
               sim::LockMode::None, true);
  const sim::Scenario flex_scenario =
      scenario("issue-2744-flex", sim::DciProfile::MatchupFlexJit,
               sim::LockMode::None, true);
  const sim::Scenario control_scenario =
      scenario("issue-2744-control", sim::DciProfile::NoDiscardControl,
               sim::LockMode::None, true);
  sim::Engine strict(strict_scenario, recipe, rng);
  sim::Engine flex(flex_scenario, recipe, rng);
  sim::Engine control(control_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(strict, route_state());
  sim::EngineTestAccess::set_state(flex, route_state());
  sim::EngineTestAccess::set_state(control, route_state());

  // Strict JIT and matchup-flex JIT share same-ready-turn payload timing.
  // No-discard-control permits earlier payload banking and stays outside this route.
  // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Engine lifetime contract: https://github.com/FlareZ123/pokemon-sims/issues/869
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2744
  expect(sim::EngineTestAccess::route_available(strict),
         "Strict JIT lost the issue-1875 Tapu-Crispin route.");
  expect(sim::EngineTestAccess::route_available(flex),
         "Matchup-flex JIT still rejects the issue-1875 Tapu-Crispin route.");
  expect(!sim::EngineTestAccess::route_available(control),
         "No-discard-control incorrectly entered the JIT-specific Tapu-Crispin route.");

  // Execute the complete matchup-flex route from the same K1 public state.
  // Quick Ball supplies the current-turn Dragon discard; Wonder Tag converts the
  // Bench action into Crispin; Crispin searches two different Basic Energy types
  // so one can be attached by its printed effect to complete Apex Dragon's GGF.
  // Earliest route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Official rulebook: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2744
  expect(sim::EngineTestAccess::complete_route(flex),
         "Matchup-flex JIT could detect but not complete the issue-1875 Tapu-Crispin route.");
}

void test_route_is_not_bound_to_absolute_turn_three() {
  std::mt19937_64 rng(3284);
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario first =
      scenario("issue-3284-first", sim::DciProfile::StrictJit,
               sim::LockMode::None, true);
  const sim::Scenario second =
      scenario("issue-3284-second", sim::DciProfile::StrictJit,
               sim::LockMode::None, false);

  for (const int turn : {2, 3, 4}) {
    sim::Engine first_engine(first, recipe, rng);
    sim::Engine second_engine(second, recipe, rng);
    sim::EngineTestAccess::set_state(first_engine, route_state(turn));
    sim::EngineTestAccess::set_state(second_engine, route_state(turn));

    // This packet is wholly current-turn. Once Item, Supporter, Wonder Tag,
    // attachment, K1, Energy, payload, and target predicates are true, no card
    // text imposes an absolute player-turn identity.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/3284
    expect(sim::EngineTestAccess::route_available(first_engine),
           "Issue-3284 route still depends on absolute turn for the first seat.");
    expect(sim::EngineTestAccess::route_available(second_engine),
           "Issue-3284 route still depends on absolute turn for the second seat.");
  }

  sim::Engine completion(first, recipe, rng);
  sim::EngineTestAccess::set_state(completion, route_state(2));
  expect(sim::EngineTestAccess::complete_route(completion),
         "Issue-3284 earlier legal turn was detected but did not complete.");
}

void test_route_keeps_semantic_legality_and_resource_gates() {
  std::mt19937_64 rng(3284001);
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario legal =
      scenario("issue-3284-legal", sim::DciProfile::StrictJit,
               sim::LockMode::None, true);

  sim::Engine k0(legal, recipe, rng);
  sim::EngineTestAccess::set_state(k0, route_state(2), false);
  expect(!sim::EngineTestAccess::route_available(k0),
         "Issue-3284 route bypassed the K1 requirement.");

  sim::Engine item_locked(
      scenario("issue-3284-item-lock", sim::DciProfile::StrictJit,
               sim::LockMode::FullItem, true),
      recipe, rng);
  sim::EngineTestAccess::set_state(item_locked, route_state(2));
  expect(!sim::EngineTestAccess::route_available(item_locked),
         "Issue-3284 route ignored Item lock.");

  sim::Engine supporter_locked(
      scenario("issue-3284-supporter-lock", sim::DciProfile::StrictJit,
               sim::LockMode::FullSupporter, true),
      recipe, rng);
  sim::EngineTestAccess::set_state(supporter_locked, route_state(2));
  expect(!sim::EngineTestAccess::route_available(supporter_locked),
         "Issue-3284 route ignored Supporter lock.");

  sim::Engine ability_locked(
      scenario("issue-3284-ability-lock", sim::DciProfile::StrictJit,
               sim::LockMode::FullRuleBoxAbility, true),
      recipe, rng);
  sim::EngineTestAccess::set_state(ability_locked, route_state(2));
  expect(!sim::EngineTestAccess::route_available(ability_locked),
         "Issue-3284 route ignored Wonder Tag Ability lock.");

  sim::State spent_attachment = route_state(2);
  spent_attachment.manual_energy_used = true;
  sim::Engine spent(legal, recipe, rng);
  sim::EngineTestAccess::set_state(spent, std::move(spent_attachment));
  expect(!sim::EngineTestAccess::route_available(spent),
         "Issue-3284 route ignored the spent manual attachment.");

  sim::State dde_complete = route_state(2);
  dde_complete.active->double_dragon = 1;
  sim::Engine dde(legal, recipe, rng);
  sim::EngineTestAccess::set_state(dde, std::move(dde_complete));
  // Double Dragon Energy supplies two flexible Energy to a Dragon Pokémon. With
  // one Grass already attached, Apex Dragon's GGF cost is paid, so spending Fire,
  // Quick Ball, Wonder Tag, and Crispin would be connector-dominated.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Connector/resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2238
  // Absolute-turn regression: https://github.com/FlareZ123/pokemon-sims/issues/3284
  expect(!sim::EngineTestAccess::route_available(dde),
         "Issue-3284 route regressed the DDE-aware Energy-completion guard.");

  sim::State missing_quick_ball = route_state(2);
  missing_quick_ball.hand = {sim::Card::Fire, sim::Card::Dragapult};
  sim::Engine missing(legal, recipe, rng);
  sim::EngineTestAccess::set_state(missing, std::move(missing_quick_ball));
  expect(!sim::EngineTestAccess::route_available(missing),
         "Issue-3284 route ignored the missing Quick Ball resource.");
}
}  // namespace

int main() {
  try {
    test_route_uses_shared_jit_timing_semantics();
    test_route_is_not_bound_to_absolute_turn_three();
    test_route_keeps_semantic_legality_and_resource_gates();
    std::cout << "Issue 2744/3284 Tapu-Crispin gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
