#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1817_prelock_treasure_tapu_route_available();
  }
  static bool play_treasure(Engine& engine) {
    return engine.issue_1817_play_prelock_treasure_tapu_route();
  }
  static bool play_steven(Engine& engine) {
    return engine.issue_1817_play_prelock_steven_package();
  }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static void stage_wonder_tag(Engine& engine) {
    if (!engine.remove_one(engine.state_.hand, Card::TapuLeleGX)) {
      throw std::logic_error("Tapu Lele-GX missing from staged hand");
    }
    engine.state_.bench.push_back(
        Pokemon{Card::TapuLeleGX, engine.state_.turn, 0, 0, Tool::None});
    if (!engine.move_deck_to_hand(Card::StevensResolve)) {
      throw std::logic_error("Steven's Resolve missing from staged deck");
    }
  }
  static void set_burnet_finish_pending(Engine& engine) {
    engine.issue_1817_burnet_finish_pending_ = true;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Scenario scenario(
    const sim::LockMode lock = sim::LockMode::TurnTwoItem,
    const bool going_first = false, const int max_turn = 5) {
  return sim::Scenario{"issue-1817-prelock-route", sim::DciProfile::StrictJit,
                       lock, going_first, max_turn};
}

sim::State base_state() {
  sim::State state;
  state.turn = 1;
  state.active =
      sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::MysteriousTreasure,
      sim::Card::Dipplin,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Dragapult,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::StevensResolve,
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::ProfessorBurnet,
      sim::Card::MegaDragonite,
      sim::Card::QuickBall,
  };
  state.prizes = {
      sim::Card::Gladion,
      sim::Card::ForestSealStone,
      sim::Card::Crispin,
      sim::Card::RoseannesBackup,
      sim::Card::Grass,
      sim::Card::TateLiza,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(sim::Scenario selected_scenario = scenario())
      : scenario_value(std::move(selected_scenario)),
        recipe(sim::baseline_recipe()),
        rng(1817),
        engine(scenario_value, recipe, rng) {}
};

void prelock_route_banks_complete_package() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state());

  // Vital Dance has established K1. Treasure can use the visible Dipplin cost,
  // search Tapu Lele-GX before Item lock, attach Fire, and let Wonder Tag find
  // Steven. Steven then reserves VSTAR, the second Grass, and Burnet:
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, scheduled lock, strict-JIT, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1817
  expect(sim::EngineTestAccess::route_available(fixture.engine),
         "The pre-lock Treasure-to-Tapu route was not recognized");
  expect(sim::EngineTestAccess::play_treasure(fixture.engine),
         "Mysterious Treasure did not play the pre-lock route");

  const sim::State& after_treasure = sim::EngineTestAccess::state(fixture.engine);
  expect(after_treasure.manual_energy_used,
         "The route did not spend the visible T1 attachment");
  expect(after_treasure.active->fire == 1,
         "The route did not attach Fire to Regidrago V");
  expect(contains(after_treasure.hand, sim::Card::TapuLeleGX),
         "Treasure did not search Tapu Lele-GX");
  expect(contains(after_treasure.discard, sim::Card::Dipplin),
         "Treasure did not use the visible Dipplin cost");

  sim::EngineTestAccess::stage_wonder_tag(fixture.engine);
  expect(sim::EngineTestAccess::play_steven(fixture.engine),
         "Steven did not resolve the scheduled-lock package");
  const sim::State& after_steven = sim::EngineTestAccess::state(fixture.engine);
  expect(after_steven.turn_ended, "Steven did not end the turn");
  expect(contains(after_steven.hand, sim::Card::RegidragoVstar),
         "Steven did not search Regidrago VSTAR");
  expect(std::count(after_steven.hand.begin(), after_steven.hand.end(),
                    sim::Card::Grass) >= 2,
         "Steven did not reserve the second Grass attachment");
  expect(contains(after_steven.hand, sim::Card::ProfessorBurnet),
         "Steven did not reserve Professor Burnet");
}

void hidden_or_missing_axes_reject_route() {
  Fixture k0;
  sim::EngineTestAccess::set_state(k0.engine, base_state(), false);
  expect(!sim::EngineTestAccess::route_available(k0.engine),
         "The route read exact deck identities at K0");

  for (const sim::Card missing : {sim::Card::TapuLeleGX,
                                  sim::Card::StevensResolve,
                                  sim::Card::RegidragoVstar,
                                  sim::Card::Grass,
                                  sim::Card::ProfessorBurnet}) {
    Fixture fixture;
    sim::State state = base_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), missing),
                     state.deck.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "The route invented a missing searched axis");
  }
}

void public_constraints_reject_route() {
  Fixture wrong_lock{scenario(sim::LockMode::None)};
  sim::EngineTestAccess::set_state(wrong_lock.engine, base_state());
  expect(!sim::EngineTestAccess::route_available(wrong_lock.engine),
         "The scheduled-lock route activated without the scheduled lock");

  Fixture going_first{scenario(sim::LockMode::TurnTwoItem, true)};
  sim::EngineTestAccess::set_state(going_first.engine, base_state());
  expect(!sim::EngineTestAccess::route_available(going_first.engine),
         "The going-second route activated while going first");

  Fixture missing_cost;
  sim::State no_dipplin = base_state();
  no_dipplin.hand.erase(std::remove(no_dipplin.hand.begin(), no_dipplin.hand.end(),
                                    sim::Card::Dipplin),
                        no_dipplin.hand.end());
  sim::EngineTestAccess::set_state(missing_cost.engine, std::move(no_dipplin));
  expect(!sim::EngineTestAccess::route_available(missing_cost.engine),
         "The route invented the Treasure discard cost");

  Fixture full_bench;
  sim::State crowded = base_state();
  while (crowded.bench.size() < 5U) {
    crowded.bench.push_back(
        sim::Pokemon{sim::Card::CrobatV, 1, 0, 0, sim::Tool::None});
  }
  sim::EngineTestAccess::set_state(full_bench.engine, std::move(crowded));
  expect(!sim::EngineTestAccess::route_available(full_bench.engine),
         "The route invented a Tapu Lele-GX Bench slot");
}

void deterministic_finish_preserves_legacy_star() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 0, 1, 1,
                              sim::Tool::None};
  state.manual_energy_used = true;
  state.hand = {sim::Card::Grass, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Dragapult, sim::Card::MegaDragonite,
                sim::Card::QuickBall};
  state.prizes = {sim::Card::Gladion, sim::Card::Crispin,
                  sim::Card::TapuLeleGX, sim::Card::Fire,
                  sim::Card::RegidragoV, sim::Card::TateLiza};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_burnet_finish_pending(fixture.engine);

  expect(!sim::EngineTestAccess::use_legacy_star(fixture.engine),
         "Legacy Star was spent despite the guaranteed Burnet finish");
  expect(!sim::EngineTestAccess::state(fixture.engine).vstar_power_used,
         "The deterministic route consumed the one-use VSTAR Power");
}

void exact_seed_reaches_turn_three() {
  const auto selected_scenario =
      sim::scenario_by_label("strict-jit-turn2-item-lock/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected_scenario.has_value(), "Missing scheduled Item-lock scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{314159};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  // Source-bound regression: https://github.com/FlareZ123/pokemon-sims/issues/1817
  // CI reproduction: https://github.com/FlareZ123/pokemon-sims/actions/runs/30454351797
  expect(outcome.first_ready_turn == 3,
         "Seed 314159 did not reach the deterministic T3 route");
  expect(trace_contains("searched Tapu Lele-GX for the pre-lock Steven"),
         "Seed 314159 did not choose Tapu Lele-GX");
  expect(trace_contains("Regidrago VSTAR, Grass Energy, Professor Burnet"),
         "Steven did not reserve the exact pre-lock package");
  expect(trace_contains("T3 | READY"), "Seed 314159 did not become ready on T3");
  expect(!trace_contains("T2 | LEGACY STAR"),
         "Seed 314159 still spent Legacy Star on a dominated route");
}

}  // namespace

int main() {
  try {
    prelock_route_banks_complete_package();
    hidden_or_missing_axes_reject_route();
    public_constraints_reject_route();
    deterministic_finish_preserves_legacy_star();
    exact_seed_reaches_turn_three();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
