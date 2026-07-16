#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State benched_regidrago_recovery_state(const sim::Card supporter) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None}};
  state.hand = {supporter, sim::Card::EvolutionIncense};
  state.deck = {sim::Card::Grass};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  return state;
}

void assert_completed_benched_recovery(const sim::State& state, const sim::Card supporter) {
  assert(state.supporter_used);
  assert(state.retreat_used);
  assert(state.active.has_value());
  assert(state.active->card == sim::Card::RegidragoVstar);
  assert(state.active->grass == 2);
  assert(state.active->fire == 1);
  assert(contains(state.discard, supporter));
  assert(contains(state.discard, sim::Card::EvolutionIncense));
  assert(!contains(state.discard, sim::Card::RegidragoVstar));
  assert(contains(state.discarded_this_turn, sim::Card::MegaDragonite));
  assert(std::any_of(state.bench.begin(), state.bench.end(), [](const sim::Pokemon& pokemon) {
    return pokemon.card == sim::Card::LatiasEx;
  }));
}

void test_roseanne_restores_benched_vstar_evolution_and_promotion() {
  const sim::Scenario scenario{"roseanne-benched-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3111};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::state(engine) = benched_regidrago_recovery_state(sim::Card::RoseannesBackup);
  sim::EngineTestAccess::set_deck_seen(engine);

  // Roseanne's Backup restores the discarded Pokémon, Evolution Incense searches
  // that Evolution, and a prior-turn Benched Regidrago V may evolve. Latias ex then
  // gives the Basic Active no Retreat Cost for promotion:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::run_turn(engine);
  assert_completed_benched_recovery(sim::EngineTestAccess::state(engine), sim::Card::RoseannesBackup);
}

void test_team_yell_restores_benched_vstar_evolution_and_promotion() {
  const sim::Scenario scenario{"team-yell-benched-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3112};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::state(engine) = benched_regidrago_recovery_state(sim::Card::TeamYellsCheer);
  sim::EngineTestAccess::set_deck_seen(engine);

  // Team Yell's Cheer restores the discarded Pokémon, Evolution Incense searches
  // that Evolution, and the legal evolution target may be Benched. Latias ex supplies
  // the free-retreat promotion after the evolution resolves:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::run_turn(engine);
  assert_completed_benched_recovery(sim::EngineTestAccess::state(engine), sim::Card::TeamYellsCheer);
}

void test_final_bench_slot_is_reserved_for_held_latias_promotion() {
  const sim::Scenario scenario{"latias-final-slot", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{4031};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None},
      sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::MawileGX, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                sim::Card::RegidragoV};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};

  // Latias ex must occupy the final Bench slot before the redundant Regidrago V.
  // The prior-turn Regidrago V can then evolve, and Skyliner supplies the legal
  // free-retreat promotion into the GGF VSTAR:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == sim::Card::RegidragoVstar);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(state.retreat_used);
  assert(contains(state.hand, sim::Card::RegidragoV));
  assert(std::any_of(state.bench.begin(), state.bench.end(), [](const sim::Pokemon& pokemon) {
    return pokemon.card == sim::Card::LatiasEx;
  }));
}

sim::State searchable_vstar_final_slot_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None},
      sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::MawileGX, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {sim::Card::LatiasEx, sim::Card::RegidragoV,
                sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoVstar, sim::Card::Fire};
  state.discard = {sim::Card::MegaDragonite};
  return state;
}

void test_final_bench_slot_is_reserved_for_searchable_vstar_promotion() {
  const sim::Scenario scenario{"searchable-vstar-latias-final-slot",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{6871};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state = searchable_vstar_final_slot_state();

  // A second Mysterious Treasure is a legal discard cost for the first, which can
  // search Regidrago VSTAR. Reserve the final Bench slot for held Latias ex before
  // the redundant Regidrago V, then use Skyliner to promote the powered VSTAR:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/687
  sim::EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == sim::Card::RegidragoVstar);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(state.retreat_used);
  assert(contains(state.hand, sim::Card::RegidragoV));
  assert(std::any_of(state.bench.begin(), state.bench.end(), [](const sim::Pokemon& pokemon) {
    return pokemon.card == sim::Card::LatiasEx;
  }));
}

void test_final_bench_slot_is_reserved_for_evolution_incense_vstar_route() {
  const sim::Scenario scenario{"incense-vstar-latias-final-slot",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{6875};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state = searchable_vstar_final_slot_state();
  state.hand = {sim::Card::LatiasEx, sim::Card::RegidragoV,
                sim::Card::EvolutionIncense};

  // Evolution Incense directly searches the pending Evolution Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/687
  sim::EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == sim::Card::RegidragoVstar);
  assert(state.retreat_used);
  assert(contains(state.hand, sim::Card::RegidragoV));
}

void test_final_bench_slot_is_reserved_for_payable_ultra_ball_vstar_route() {
  const sim::Scenario scenario{"ultra-vstar-latias-final-slot",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{6876};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state = searchable_vstar_final_slot_state();
  state.hand = {sim::Card::LatiasEx, sim::Card::RegidragoV, sim::Card::UltraBall,
                sim::Card::Dipplin, sim::Card::Arven, sim::Card::Arven};

  // Ultra Ball has two distinct legal costs and can search Regidrago VSTAR:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/687
  sim::EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == sim::Card::RegidragoVstar);
  assert(state.retreat_used);
  assert(contains(state.hand, sim::Card::RegidragoV));
}

void test_searchable_vstar_does_not_reserve_latias_when_item_locked() {
  const sim::Scenario scenario{"searchable-vstar-latias-item-lock",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::FullItem, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{6872};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state = searchable_vstar_final_slot_state();

  sim::EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == sim::Card::TapuLeleGX);
  assert(contains(state.hand, sim::Card::LatiasEx));
  assert(!contains(state.hand, sim::Card::RegidragoV));
}

void test_searchable_vstar_does_not_reserve_latias_for_unpayable_item() {
  const sim::Scenario scenario{"searchable-vstar-latias-unpayable",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{6873};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state = searchable_vstar_final_slot_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::MysteriousTreasure));

  sim::EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == sim::Card::TapuLeleGX);
  assert(contains(state.hand, sim::Card::LatiasEx));
  assert(!contains(state.hand, sim::Card::RegidragoV));
}

void test_searchable_vstar_does_not_reserve_latias_when_vstar_known_absent() {
  const sim::Scenario scenario{"searchable-vstar-latias-known-absent",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{6874};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state = searchable_vstar_final_slot_state();
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_deck_seen(engine);

  sim::EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == sim::Card::TapuLeleGX);
  assert(contains(state.hand, sim::Card::LatiasEx));
  assert(!contains(state.hand, sim::Card::RegidragoV));
}

void test_tate_route_preserves_redundant_regidrago_bench_behavior() {
  const sim::Scenario scenario{"tate-no-latias-reservation", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{4032};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None},
      sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::MawileGX, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                sim::Card::RegidragoV, sim::Card::TateLiza};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};

  // Tate & Liza supplies the alternate switch route, so Latias ex does not need the
  // final Bench slot and the established redundant-Regidrago behavior remains legal:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == sim::Card::RegidragoVstar);
  assert(state.supporter_used);
  assert(!state.retreat_used);
  assert(contains(state.hand, sim::Card::LatiasEx));
  assert(!contains(state.hand, sim::Card::RegidragoV));
}

}  // namespace

int main() {
  test_roseanne_restores_benched_vstar_evolution_and_promotion();
  test_team_yell_restores_benched_vstar_evolution_and_promotion();
  test_final_bench_slot_is_reserved_for_held_latias_promotion();
  test_final_bench_slot_is_reserved_for_searchable_vstar_promotion();
  test_final_bench_slot_is_reserved_for_evolution_incense_vstar_route();
  test_final_bench_slot_is_reserved_for_payable_ultra_ball_vstar_route();
  test_searchable_vstar_does_not_reserve_latias_when_item_locked();
  test_searchable_vstar_does_not_reserve_latias_for_unpayable_item();
  test_searchable_vstar_does_not_reserve_latias_when_vstar_known_absent();
  test_tate_route_preserves_redundant_regidrago_bench_behavior();
  return 0;
}
