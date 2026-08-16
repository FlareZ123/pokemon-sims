#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static bool finish_available(const Engine& engine) {
    return engine.quick_ball_latias_finish_available();
  }

  static bool play_finish(Engine& engine) {
    return engine.play_quick_ball_latias_finish();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count_card(const std::vector<sim::Card>& zone, const sim::Card card) {
  return static_cast<int>(std::count(zone.begin(), zone.end(), card));
}

sim::State route_state(const int turn, const int entered_turn,
                       const sim::Card active = sim::Card::Oricorio,
                       const sim::Card payload = sim::Card::Appletun) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{active, 0};  // Eligible Basic Active is semantic, not Dialga-specific: https://github.com/FlareZ123/pokemon-sims/issues/4109
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, entered_turn, 2, 0}};  // Prior-turn GG Regidrago evolution source: https://api.pokemontcg.io/v2/cards/swsh12-135
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoVstar,
                sim::Card::Fire, payload};  // Quick Ball cost plus GGF completion: https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/swsh12-136
  state.deck = {sim::Card::LatiasEx, sim::Card::Grass,
                sim::Card::MysteriousTreasure};  // Latias ex is the Basic search target and Skyliner retreat connector: https://api.pokemontcg.io/v2/cards/sv8-76
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_state_generic_finish_and_payload() {
  // The route is legal on a later current turn while going second, with Oricorio
  // Active and Appletun as the canonical permitted payload. None of those physical
  // actions requires the historical seed-69 seat, T4, Dialga-GX, or Mega Dragonite:
  // Advanced Item, evolution, Ability, attachment, and Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed systemic overfit: https://github.com/FlareZ123/pokemon-sims/issues/4109
  const sim::Scenario scenario{"issue-4109-generic", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng{410900};
  sim::Engine engine = make_engine(scenario, rng, route_state(3, 2));

  expect(sim::EngineTestAccess::finish_available(engine),
         "Equivalent going-second T3 state with a non-Dialga Basic Active and non-Mega payload must be admitted.");
  expect(sim::EngineTestAccess::play_finish(engine),
         "The state-generic Quick Ball Latias finish must resolve.");
  expect(count_card(engine.state().discard, sim::Card::Appletun) == 1,
         "The canonical payload selector must spend the permitted Appletun payload.");
  expect(count_card(engine.state().hand, sim::Card::LatiasEx) == 0,
         "Latias ex must leave hand after the resolver Benches it.");
  expect(std::any_of(engine.state().bench.begin(), engine.state().bench.end(),
                     [](const sim::Pokemon& pokemon) {
                       return pokemon.card == sim::Card::LatiasEx;
                     }),
         "Latias ex must be Benched to establish Skyliner.");
}

void test_evolution_and_active_controls() {
  // Evolution is illegal on the player's first turn even for an opening Basic,
  // and a Pokémon put into play this turn cannot evolve. The Active also has to be
  // Basic for Skyliner to solve the retreat axis:
  // Advanced evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Confirmed systemic overfit: https://github.com/FlareZ123/pokemon-sims/issues/4109
  const sim::Scenario first_turn{"issue-4109-first-turn", sim::DciProfile::StrictJit,
                                 sim::LockMode::None, false, 3};
  std::mt19937_64 first_rng{410901};
  sim::Engine first = make_engine(first_turn, first_rng, route_state(1, 0));
  expect(!sim::EngineTestAccess::finish_available(first),
         "Opening Regidrago V must not become evolvable on the player's first turn.");

  const sim::Scenario later{"issue-4109-controls", sim::DciProfile::StrictJit,
                            sim::LockMode::None, true, 3};
  std::mt19937_64 same_turn_rng{410902};
  sim::Engine same_turn = make_engine(later, same_turn_rng, route_state(3, 3));
  expect(!sim::EngineTestAccess::finish_available(same_turn),
         "A Regidrago V that entered this turn must remain ineligible to evolve.");

  std::mt19937_64 evolved_active_rng{410903};
  sim::Engine evolved_active = make_engine(
      later, evolved_active_rng,
      route_state(3, 2, sim::Card::Dragapult));  // Dragapult ex is not Basic, so Skyliner does not erase its Retreat Cost: https://api.pokemontcg.io/v2/cards/sv6-130
  expect(!sim::EngineTestAccess::finish_available(evolved_active),
         "A non-Basic Active must remain outside the Skyliner promotion route.");

  std::mt19937_64 regi_active_rng{410904};
  sim::Engine regi_active = make_engine(
      later, regi_active_rng, route_state(3, 2, sim::Card::RegidragoV));
  expect(!sim::EngineTestAccess::finish_available(regi_active),
         "An Active Regidrago V does not need the Latias promotion connector.");
}

void test_resource_controls() {
  // The generalization preserves the physical Quick Ball, Latias, payload, Bench,
  // Ability, retreat, and final-Energy requirements from #2164:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Original route contract: https://github.com/FlareZ123/pokemon-sims/issues/2164
  const sim::Scenario scenario{"issue-4109-resource-controls",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 3};

  sim::State no_payload_state = route_state(3, 2);
  no_payload_state.hand.erase(std::remove(no_payload_state.hand.begin(),
                                          no_payload_state.hand.end(),
                                          sim::Card::Appletun),
                               no_payload_state.hand.end());
  std::mt19937_64 payload_rng{410905};
  sim::Engine no_payload = make_engine(scenario, payload_rng, std::move(no_payload_state));
  expect(!sim::EngineTestAccess::finish_available(no_payload),
         "The route must still require a permitted held Dragon payload.");

  sim::State no_energy_state = route_state(3, 2);
  no_energy_state.hand.erase(std::remove(no_energy_state.hand.begin(),
                                         no_energy_state.hand.end(),
                                         sim::Card::Fire),
                              no_energy_state.hand.end());
  std::mt19937_64 energy_rng{410906};
  sim::Engine no_energy = make_engine(scenario, energy_rng, std::move(no_energy_state));
  expect(!sim::EngineTestAccess::finish_available(no_energy),
         "The route must still require a held Basic Energy that completes Apex Dragon's GGF cost.");

  sim::State retreat_state = route_state(3, 2);
  retreat_state.retreat_used = true;
  std::mt19937_64 retreat_rng{410907};
  sim::Engine retreat_used = make_engine(scenario, retreat_rng, std::move(retreat_state));
  expect(!sim::EngineTestAccess::finish_available(retreat_used),
         "The route must stay blocked after the turn's Retreat action is spent.");
}
}  // namespace

int main() {
  try {
    test_state_generic_finish_and_payload();
    test_evolution_and_active_controls();
    test_resource_controls();
    std::cout << "Issue 4109 Quick Ball Latias state-generic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
