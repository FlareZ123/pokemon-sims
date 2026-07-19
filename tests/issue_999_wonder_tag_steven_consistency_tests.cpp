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
  static void set_state(Engine& engine, State state,
                        const bool wonder_tag_selected_steven) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.wonder_tag_selected_steven_this_turn_ =
        wonder_tag_selected_steven;
  }

  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven();
  }

  static bool play_steven(Engine& engine) {
    return engine.play_steven();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine(const sim::Scenario& scenario) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{999};
  return sim::Engine(scenario, recipe, rng);
}

sim::State admitted_steven_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::StevensResolve, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::LatiasEx};
  return state;
}

void test_wonder_tag_admission_survives_later_item_state_change() {
  const sim::Scenario scenario{"issue-999", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  sim::Engine engine = make_engine(scenario);
  sim::EngineTestAccess::set_state(engine, admitted_steven_state(), true);

  // Wonder Tag selected Steven before Quick Ball established the Basic. The newly
  // Benched Regidrago V cannot evolve during that same turn, while Steven legally
  // searches up to three cards and ends the turn:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/999
  expect(sim::EngineTestAccess::should_play_steven(engine),
         "The Wonder Tag-admitted Steven route must survive the later Basic setup action.");
  expect(sim::EngineTestAccess::play_steven(engine),
         "The admitted Steven must resolve instead of remaining unused.");
  expect(engine.state().supporter_used && engine.state().turn_ended,
         "Steven must consume the Supporter play and end the turn.");
}

void test_unadmitted_one_axis_state_does_not_force_steven() {
  const sim::Scenario scenario{"issue-999-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  sim::Engine engine = make_engine(scenario);
  sim::EngineTestAccess::set_state(engine, admitted_steven_state(), false);

  expect(!sim::EngineTestAccess::should_play_steven(engine),
         "An independently held Steven must keep the ordinary mutable-axis predicate.");
}

void test_proven_faster_crispin_mysterious_route_still_wins() {
  const sim::Scenario scenario{"issue-999-yield", sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, false, 5};
  sim::Engine engine = make_engine(scenario);
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure,
                sim::Card::Dipplin, sim::Card::Grass};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // Dipplin has no legal evolution route in this deck and is the strict-DCI search
  // cost that keeps the established Mysterious Treasure plus Crispin line payable:
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/774
  // https://github.com/FlareZ123/pokemon-sims/issues/999
  expect(!sim::EngineTestAccess::should_play_steven(engine),
         "A proven faster current route must still suppress the banked Steven play.");
}

void test_seed_17_plays_the_searched_steven() {
  const sim::Scenario scenario{"strict-jit/go-second", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{17};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Exact confirmed reproduction:
  // https://github.com/FlareZ123/pokemon-sims/issues/999
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sm7-145
  expect(outcome.used_steven,
         "Seed 17 must play the Steven searched by Wonder Tag on turn one.");
  expect(std::any_of(trace.lines.begin(), trace.lines.end(),
                     [](const std::string& line) {
                       return line.find("T1 | PLAY SUPPORTER") != std::string::npos &&
                              line.find("Searched up to 3 cards") != std::string::npos;
                     }),
         "The readable trace must contain the turn-one Steven resolution.");
}
}  // namespace

int main() {
  try {
    test_wonder_tag_admission_survives_later_item_state_change();
    test_unadmitted_one_axis_state_does_not_force_steven();
    test_proven_faster_crispin_mysterious_route_still_wins();
    test_seed_17_plays_the_searched_steven();
    std::cout << "Issue 999 Wonder Tag Steven consistency tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
