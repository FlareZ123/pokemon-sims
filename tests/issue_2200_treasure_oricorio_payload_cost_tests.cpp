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
                        const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static std::optional<Card> treasure_cost(const Engine& engine) {
    return engine.choose_discard_issue1876(
        false, true, true, Card::MysteriousTreasure);
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

sim::Scenario scenario(const sim::DciProfile profile = sim::DciProfile::StrictJit,
                       const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-2200", profile, lock, true, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.supporter_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Klara,
                sim::Card::Dragapult, sim::Card::HisuianHeavyBall,
                sim::Card::Grass};
  state.deck = {sim::Card::Oricorio, sim::Card::Fire, sim::Card::Grass,
                sim::Card::RegidragoV};
  return state;
}

sim::Engine make_engine(const sim::Scenario& selected, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(selected, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void test_exact_cost_and_profile_boundaries() {
  std::mt19937_64 rng(2200001);
  const sim::Scenario strict = scenario();
  sim::Engine strict_engine = make_engine(strict, rng, route_state());

  // Mysterious Treasure discards the visible Dragon before searching Oricorio.
  // Vital Dance then exposes Basic Fire Energy for the unused manual attachment:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Bench, Ability, and Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and dynamic DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Reopened confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2200
  expect(sim::EngineTestAccess::treasure_cost(strict_engine) ==
             sim::Card::Dragapult,
         "Strict-JIT GG Oricorio route did not select the Dragon payload.");

  const sim::Scenario flex = scenario(sim::DciProfile::MatchupFlexJit);
  sim::Engine flex_engine = make_engine(flex, rng, route_state());
  expect(sim::EngineTestAccess::treasure_cost(flex_engine) ==
             sim::Card::Dragapult,
         "Matchup-flex GG Oricorio route did not select the Dragon payload.");
}

void test_route_gates() {
  std::mt19937_64 rng(2200002);
  const sim::Scenario strict = scenario();

  sim::Engine k0 = make_engine(strict, rng, route_state(), false);
  expect(sim::EngineTestAccess::treasure_cost(k0) != sim::Card::Dragapult,
         "K0 admitted the exact-deck Oricorio payload override.");

  sim::State spent_attachment = route_state();
  spent_attachment.manual_energy_used = true;
  sim::Engine spent = make_engine(strict, rng, std::move(spent_attachment));
  expect(sim::EngineTestAccess::treasure_cost(spent) != sim::Card::Dragapult,
         "A spent manual attachment admitted the Oricorio payload override.");

  sim::State no_oricorio = route_state();
  no_oricorio.deck.erase(std::remove(no_oricorio.deck.begin(),
                                     no_oricorio.deck.end(),
                                     sim::Card::Oricorio),
                          no_oricorio.deck.end());
  sim::Engine missing_oricorio =
      make_engine(strict, rng, std::move(no_oricorio));
  expect(sim::EngineTestAccess::treasure_cost(missing_oricorio) !=
             sim::Card::Dragapult,
         "A missing Oricorio target admitted the payload override.");

  sim::State no_fire = route_state();
  no_fire.deck.erase(std::remove(no_fire.deck.begin(), no_fire.deck.end(),
                                 sim::Card::Fire),
                     no_fire.deck.end());
  sim::Engine missing_fire = make_engine(strict, rng, std::move(no_fire));
  expect(sim::EngineTestAccess::treasure_cost(missing_fire) !=
             sim::Card::Dragapult,
         "A missing Basic Fire target admitted the payload override.");

  sim::State full_bench = route_state();
  for (int index = 0; index < 5; ++index) {
    full_bench.bench.push_back(
        sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0});
  }
  sim::Engine blocked_bench = make_engine(strict, rng, std::move(full_bench));
  expect(sim::EngineTestAccess::treasure_cost(blocked_bench) !=
             sim::Card::Dragapult,
         "A full Bench admitted the Oricorio payload override.");

  const sim::Scenario locked = scenario(sim::DciProfile::StrictJit,
                                        sim::LockMode::FullItem);
  sim::Engine item_locked = make_engine(locked, rng, route_state());
  expect(sim::EngineTestAccess::treasure_cost(item_locked) !=
             sim::Card::Dragapult,
         "Item lock admitted the Treasure payload override.");

  sim::State already_payload = route_state();
  already_payload.discard = {sim::Card::DialgaGX};
  already_payload.discarded_this_turn = {sim::Card::DialgaGX};
  sim::Engine ready_payload =
      make_engine(strict, rng, std::move(already_payload));
  expect(sim::EngineTestAccess::treasure_cost(ready_payload) !=
             sim::Card::Dragapult,
         "An already-complete current-turn payload spent another Dragon.");

  sim::State wrong_energy = route_state();
  wrong_energy.active->grass = 1;
  wrong_energy.active->fire = 1;
  sim::Engine wrong_axis = make_engine(strict, rng, std::move(wrong_energy));
  expect(sim::EngineTestAccess::treasure_cost(wrong_axis) !=
             sim::Card::Dragapult,
         "The GF state without Crispin entered the GG Oricorio override.");
}

void test_registered_seed_242_reaches_t3() {
  const auto selected = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value() && deck != nullptr,
         "The registered issue-2200 fixture is unavailable.");

  std::mt19937_64 rng(242);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*selected, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The exact current-main witness must keep the existing Treasure-to-Oricorio
  // continuation while changing only its mandatory discard cost:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug and exact seed: https://github.com/FlareZ123/pokemon-sims/issues/2200
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "Issue-2200 seed 242 did not reach strict-JIT readiness on T3.");
  expect(trace_contains(trace, "Dragapult ex (Mysterious Treasure cost)"),
         "Seed 242 did not use the held Dragon as Treasure's cost.");
  expect(trace_contains(trace, "Searched a Psychic or Dragon Pokémon: Oricorio") ||
             trace_contains(trace, "searched Oricorio"),
         "Seed 242 did not preserve the Oricorio search route.");
  expect(trace_contains(trace, "Vital Dance") &&
             trace_contains(trace, "T3 | READY"),
         "Seed 242 omitted Vital Dance or the T3 ready check.");
}
}  // namespace

int main() {
  try {
    test_exact_cost_and_profile_boundaries();
    test_route_gates();
    test_registered_seed_242_reaches_t3();
    std::cout << "Issue 2200 Treasure Oricorio payload cost tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
