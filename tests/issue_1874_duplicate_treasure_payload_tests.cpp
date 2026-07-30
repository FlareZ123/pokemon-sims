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
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1874_duplicate_treasure_payload_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1874_duplicate_treasure_payload_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::Scenario strict(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1874", sim::DciProfile::StrictJit, lock, true, 5};
}

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 0};
  state.hand = {sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure,
                sim::Card::EarthenVessel,
                sim::Card::Fire,
                sim::Card::ErikasInvitation};
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV,
                sim::Card::Dragapult, sim::Card::Grass};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true,
                        const bool prizes_revealed = false) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen,
                                   prizes_revealed);
  return engine;
}

void test_exact_route_and_k1_boundaries() {
  std::mt19937_64 rng(1874);
  const sim::Scenario scenario = strict();
  sim::Engine engine = make_engine(scenario, rng, base_state());

  // Held Fire completes GGF. The first Treasure spends route-replaced Vessel and
  // searches Mega Dragonite ex; the second Treasure discards that Dragon during
  // the current turn and still has another legal Psychic or Dragon target.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, cost, search, discard, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI/JIT, connector contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1874
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact issue-1874 route was unavailable.");
  expect(sim::EngineTestAccess::complete_route(engine),
         "The exact issue-1874 route did not complete.");
  expect(engine.state().active->grass == 2 && engine.state().active->fire == 1,
         "The issue-1874 route did not complete GGF.");
  expect(std::find(engine.state().discarded_this_turn.begin(),
                   engine.state().discarded_this_turn.end(),
                   sim::Card::MegaDragonite) !=
             engine.state().discarded_this_turn.end(),
         "The second Treasure did not create the current-turn payload.");
  expect(std::find(engine.state().discard.begin(), engine.state().discard.end(),
                   sim::Card::EarthenVessel) != engine.state().discard.end(),
         "Route-replaced Earthen Vessel did not pay the first Treasure.");

  sim::Engine prize_only_k1 =
      make_engine(scenario, rng, base_state(), false, true);
  expect(sim::EngineTestAccess::route_available(prize_only_k1),
         "Full Prize inspection K1 did not admit the route.");

  sim::Engine k0 = make_engine(scenario, rng, base_state(), false, false);
  expect(!sim::EngineTestAccess::route_available(k0),
         "K0 admitted the deterministic duplicate-Treasure route.");
}

void test_lock_resource_and_target_boundaries() {
  std::mt19937_64 rng(169);
  const sim::Scenario no_lock = strict();
  const sim::Scenario item_lock = strict(sim::LockMode::FullItem);

  sim::Engine locked = make_engine(item_lock, rng, base_state());
  expect(!sim::EngineTestAccess::route_available(locked),
         "Item lock admitted the duplicate-Treasure route.");

  sim::State one_treasure = base_state();
  one_treasure.hand.erase(std::find(one_treasure.hand.begin(),
                                    one_treasure.hand.end(),
                                    sim::Card::MysteriousTreasure));
  sim::Engine missing_copy =
      make_engine(no_lock, rng, std::move(one_treasure));
  expect(!sim::EngineTestAccess::route_available(missing_copy),
         "One Mysterious Treasure admitted a two-Item bridge.");

  sim::State no_vessel = base_state();
  no_vessel.hand.erase(std::find(no_vessel.hand.begin(), no_vessel.hand.end(),
                                 sim::Card::EarthenVessel));
  sim::Engine protected_cost = make_engine(no_lock, rng, std::move(no_vessel));
  expect(!sim::EngineTestAccess::route_available(protected_cost),
         "The route was admitted without its route-replaced first cost.");

  sim::State no_payload = base_state();
  no_payload.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                     sim::Card::Grass};
  sim::Engine missing_payload = make_engine(no_lock, rng, std::move(no_payload));
  expect(!sim::EngineTestAccess::route_available(missing_payload),
         "The route was admitted without a searchable payload.");

  sim::State no_second_target = base_state();
  no_second_target.deck = {sim::Card::MegaDragonite, sim::Card::Grass};
  sim::Engine targetless_second =
      make_engine(no_lock, rng, std::move(no_second_target));
  expect(!sim::EngineTestAccess::route_available(targetless_second),
         "The route was admitted without a second legal Treasure target.");

  sim::State missing_fire = base_state();
  missing_fire.hand.erase(std::find(missing_fire.hand.begin(),
                                    missing_fire.hand.end(), sim::Card::Fire));
  sim::Engine no_held_fire = make_engine(no_lock, rng, std::move(missing_fire));
  expect(!sim::EngineTestAccess::route_available(no_held_fire),
         "The route was admitted without the held final Fire attachment.");

  sim::State live_vessel = base_state();
  live_vessel.active->grass = 1;
  sim::Engine energy_incomplete = make_engine(no_lock, rng, std::move(live_vessel));
  expect(!sim::EngineTestAccess::route_available(energy_incomplete),
         "The route discarded Vessel while an additional Grass was still missing.");

  sim::State spent_attachment = base_state();
  spent_attachment.manual_energy_used = true;
  sim::Engine attachment_spent =
      make_engine(no_lock, rng, std::move(spent_attachment));
  expect(!sim::EngineTestAccess::route_available(attachment_spent),
         "A spent manual attachment admitted the route.");
}

void test_registered_seed_169_reaches_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1874 fixture is unavailable.");

  std::mt19937_64 rng(169);
  sim::TraceLog trace{true, {}, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  expect(outcome.first_ready_turn == 3,
         "Registered seed 169 did not improve from failure to T3.");
  expect(trace_contains(trace, "First Mysterious Treasure") &&
             trace_contains(trace, "Second Mysterious Treasure") &&
             trace_contains(trace, "T3 | READY"),
         "Registered seed 169 did not execute the complete T3 bridge.");
}
}  // namespace

int main() {
  try {
    test_exact_route_and_k1_boundaries();
    test_lock_resource_and_target_boundaries();
    test_registered_seed_169_reaches_t3();
    std::cout << "Issue 1874 duplicate-Treasure payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
