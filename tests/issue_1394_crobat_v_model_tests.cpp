#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static State& state(Engine& engine) { return engine.state_; }
  static TrialOutcome& outcome(Engine& engine) { return engine.outcome_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve) {
    return engine.bench_from_hand(card, resolve);
  }
  static void resolve_entry_ability(Engine& engine, const Card card) {
    engine.resolve_entry_ability(card);
  }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
  static bool bench_crobat_if_useful(Engine& engine) {
    return engine.bench_crobat_if_useful();
  }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"issue-1394-crobat", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::make_crobat_modeling_deck(
      "crobat1-erika", {sim::Card::ErikasInvitation}).recipe};
  std::mt19937_64 rng{1394};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{scenario, recipe, rng, &trace};
};

void test_identity_registry_and_modeling_boundary() {
  // Crobat V is a Basic Rule Box Pokémon V with Retreat Cost 1:
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  // Modeling variants stay outside the two named decks:
  // https://github.com/FlareZ123/pokemon-sims/issues/1394
  if (!sim::is_basic(sim::Card::CrobatV) || !sim::is_pokemon(sim::Card::CrobatV) ||
      !sim::is_pokemon_v(sim::Card::CrobatV) ||
      !sim::is_rule_box_pokemon(sim::Card::CrobatV) ||
      sim::retreat_cost(sim::Card::CrobatV) != 1) {
    throw std::runtime_error("Crobat V classification is incomplete.");
  }
  if (sim::deck_registry().size() != 2U ||
      sim::deck_by_id("crobat1-erika") != nullptr ||
      sim::crobat_modeling_deck_by_id("crobat1-erika") == nullptr) {
    throw std::runtime_error("Crobat modeling variants leaked into the named registry.");
  }

  // Safe defaults preserve the canonical baseline and reproduce the registered matrix:
  // https://github.com/FlareZ123/pokemon-sims/issues/1394
  if (sim::crobat_model_default_output() != "results/crobat_variant_model.csv" ||
      sim::crobat_model_default_seed() != 20260723ULL) {
    throw std::runtime_error("Crobat modeling defaults do not match the reproduction contract.");
  }

}

void test_direct_bench_draws_to_six_and_once_per_turn() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::CrobatV, sim::Card::Grass, sim::Card::Fire};
  state.deck = {sim::Card::QuickBall, sim::Card::Crispin, sim::Card::Arven,
                sim::Card::RegidragoVstar, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Dark Asset triggers only from a hand-to-Bench play and draws until six:
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  if (!sim::EngineTestAccess::bench_from_hand(
          fixture.engine, sim::Card::CrobatV, true)) {
    throw std::runtime_error("Crobat V could not be Benched from hand.");
  }
  sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand.size() != 6U || !after.dark_asset_used ||
      !sim::EngineTestAccess::outcome(fixture.engine).used_dark_asset ||
      sim::EngineTestAccess::outcome(fixture.engine).dark_asset_cards_drawn != 4U) {
    throw std::runtime_error("Dark Asset did not draw the exact number of cards to six.");
  }

  after.hand = {sim::Card::CrobatV};
  after.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::QuickBall};
  if (!sim::EngineTestAccess::bench_from_hand(
          fixture.engine, sim::Card::CrobatV, true)) {
    throw std::runtime_error("The second Crobat V should still be legally Benched.");
  }
  if (!after.hand.empty() || after.deck.size() != 3U ||
      sim::EngineTestAccess::outcome(fixture.engine).dark_asset_cards_drawn != 4U) {
    throw std::runtime_error("A second Dark Asset resolved during the same turn.");
  }
}

void test_setup_and_rule_box_lock_do_not_trigger_dark_asset() {
  Fixture fixture;
  sim::State setup_state;
  setup_state.active = sim::Pokemon{sim::Card::CrobatV, 0};
  setup_state.hand = {sim::Card::Grass};
  setup_state.deck = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(setup_state));
  // Setup places Basics before turns begin, so it does not satisfy Dark Asset's
  // hand-to-Bench-during-turn condition: https://api.pokemontcg.io/v2/cards/swsh3-104
  if (sim::EngineTestAccess::state(fixture.engine).dark_asset_used ||
      sim::EngineTestAccess::outcome(fixture.engine).used_dark_asset) {
    throw std::runtime_error("An opening Crobat V incorrectly used Dark Asset.");
  }

  Fixture locked;
  locked.scenario.locks = sim::LockMode::FullRuleBoxAbility;
  sim::State locked_state;
  locked_state.turn = 2;
  locked_state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  locked_state.hand = {sim::Card::CrobatV, sim::Card::Grass};
  locked_state.deck = {sim::Card::Fire, sim::Card::QuickBall};
  sim::EngineTestAccess::set_state(locked.engine, std::move(locked_state));
  // Path-style Rule Box Ability lock suppresses Dark Asset:
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  if (!sim::EngineTestAccess::bench_from_hand(
          locked.engine, sim::Card::CrobatV, true)) {
    throw std::runtime_error("Ability lock must not prohibit Benching Crobat V.");
  }
  const sim::State& after_lock = sim::EngineTestAccess::state(locked.engine);
  if (after_lock.hand.size() != 1U || after_lock.deck.size() != 2U ||
      after_lock.dark_asset_used ||
      sim::EngineTestAccess::outcome(locked.engine).used_dark_asset) {
    throw std::runtime_error("Dark Asset resolved through Rule Box Ability lock.");
  }
}

void test_empty_deck_does_not_spend_crobat_connector() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::CrobatV, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Dark Asset cannot draw from an empty deck, so the setup policy preserves
  // Crobat V and its Bench slot: https://api.pokemontcg.io/v2/cards/swsh3-104
  if (sim::EngineTestAccess::bench_crobat_if_useful(fixture.engine) ||
      std::count(sim::EngineTestAccess::state(fixture.engine).hand.begin(),
                 sim::EngineTestAccess::state(fixture.engine).hand.end(),
                 sim::Card::CrobatV) == 0) {
    throw std::runtime_error("The policy spent Crobat V when Dark Asset could draw no cards.");
  }
}

void test_bench_limit_quick_ball_and_forest_seal_stone() {
  Fixture full;
  sim::State full_state;
  full_state.turn = 2;
  full_state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  full_state.hand = {sim::Card::CrobatV};
  full_state.deck = {sim::Card::Grass};
  for (int i = 0; i < 5; ++i) {
    full_state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1});
  }
  sim::EngineTestAccess::set_state(full.engine, std::move(full_state));
  // A player may have at most five Benched Pokémon:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  if (sim::EngineTestAccess::bench_from_hand(
          full.engine, sim::Card::CrobatV, true)) {
    throw std::runtime_error("Crobat V exceeded the five-Pokémon Bench limit.");
  }

  Fixture searched;
  sim::State search_state;
  search_state.turn = 2;
  search_state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  search_state.hand = {sim::Card::QuickBall, sim::Card::Dipplin};
  search_state.deck = {sim::Card::CrobatV, sim::Card::Grass,
                       sim::Card::Fire, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(searched.engine, std::move(search_state));
  sim::EngineTestAccess::set_deck_seen(searched.engine);
  // Quick Ball searches a Basic after discarding one card, and the fetched Crobat
  // may then be played from hand to trigger Dark Asset:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  if (!sim::EngineTestAccess::play_quick_ball(searched.engine) ||
      !sim::EngineTestAccess::bench_crobat_if_useful(searched.engine)) {
    throw std::runtime_error("Quick Ball did not complete the Crobat V draw route.");
  }
  if (!sim::EngineTestAccess::outcome(searched.engine).used_dark_asset) {
    throw std::runtime_error("Searched Crobat V failed to use Dark Asset.");
  }

  Fixture tool;
  sim::State tool_state;
  tool_state.turn = 2;
  tool_state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  tool_state.bench = {sim::Pokemon{sim::Card::CrobatV, 1}};
  tool_state.hand = {sim::Card::ForestSealStone};
  sim::EngineTestAccess::set_state(tool.engine, std::move(tool_state));
  // Forest Seal Stone may attach to a Pokémon V, including Crobat V:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  if (!sim::EngineTestAccess::attach_fss(tool.engine) ||
      sim::EngineTestAccess::state(tool.engine).bench.front().tool !=
          sim::Tool::ForestSealStone) {
    throw std::runtime_error("Forest Seal Stone did not attach to Crobat V.");
  }
}

}  // namespace

int main() {
  try {
    test_identity_registry_and_modeling_boundary();
    test_direct_bench_draws_to_six_and_once_per_turn();
    test_setup_and_rule_box_lock_do_not_trigger_dark_asset();
    test_empty_deck_does_not_spend_crobat_connector();
    test_bench_limit_quick_ball_and_forest_seal_stone();
    std::cout << "issue 1394 Crobat V modeling tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
