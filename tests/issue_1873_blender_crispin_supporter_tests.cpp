#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  static bool route_available(Engine& engine) {
    return engine.issue_1873_blender_crispin_preempts_gladion();
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool play_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1873};
  sim::Engine engine;

  Fixture(const sim::DciProfile dci = sim::DciProfile::StrictJit,
          const sim::LockMode locks = sim::LockMode::None)
      : scenario{"issue-1873", dci, locks, false, 5},
        engine{scenario, recipe, rng} {}
};

sim::State exact_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.manual_energy_used = true;
  state.hand = {sim::Card::Crispin, sim::Card::Gladion,
                sim::Card::BrilliantBlender, sim::Card::EarthenVessel,
                sim::Card::Fire};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::GoodraVstar};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::RegidragoV,
                  sim::Card::HisuianHeavyBall, sim::Card::ProfessorBurnet,
                  sim::Card::Appletun, sim::Card::DialgaGX};
  return state;
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

void test_deck_search_and_prize_inspection_k1() {
  Fixture deck_fixture;
  sim::EngineTestAccess::set_state(deck_fixture.engine, exact_state(), true,
                                   false);

  // Crispin must spend the Supporter action on the final Grass while Blender
  // supplies the same-turn Dragon payload. Either legal deck inspection or a
  // complete Prize inspection establishes public K1 under repository policy:
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1873
  expect(sim::EngineTestAccess::route_available(deck_fixture.engine),
         "Deck-search K1 did not admit the Blender-Crispin route.");

  Fixture prize_fixture;
  sim::EngineTestAccess::set_state(prize_fixture.engine, exact_state(), false,
                                   true);
  expect(sim::EngineTestAccess::route_available(prize_fixture.engine),
         "Prize-inspection K1 did not admit the Blender-Crispin route.");

  Fixture k0_fixture;
  sim::EngineTestAccess::set_state(k0_fixture.engine, exact_state(), false,
                                   false);
  expect(!sim::EngineTestAccess::route_available(k0_fixture.engine),
         "True K0 admitted the public-K1-only Blender-Crispin route.");
}

void test_exact_route_uses_crispin_then_blender() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, exact_state());
  sim::EngineTestAccess::choose_supporter(fixture.engine);

  const sim::State& after_crispin = sim::EngineTestAccess::state(fixture.engine);
  expect(after_crispin.supporter_used,
         "Crispin did not consume the available Supporter action.");
  expect(after_crispin.active && after_crispin.active->grass >= 2 &&
             after_crispin.active->fire >= 1,
         "Crispin did not complete GGF in the exact state.");
  expect(std::find(after_crispin.hand.begin(), after_crispin.hand.end(),
                   sim::Card::Gladion) != after_crispin.hand.end(),
         "The stronger route did not preserve Gladion.");
  expect(sim::EngineTestAccess::play_blender(fixture.engine),
         "The protected Brilliant Blender was not playable.");
  expect(!sim::EngineTestAccess::state(fixture.engine)
              .discarded_this_turn.empty(),
         "Brilliant Blender did not establish a same-turn payload.");
}

void test_legality_and_resource_boundaries() {
  {
    Fixture fixture(sim::DciProfile::StrictJit, sim::LockMode::FullItem);
    sim::EngineTestAccess::set_state(fixture.engine, exact_state());
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "Item lock did not block Brilliant Blender.");
  }
  {
    Fixture fixture(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter);
    sim::EngineTestAccess::set_state(fixture.engine, exact_state());
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "Supporter lock did not block Crispin.");
  }
  {
    sim::State state = exact_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::Fire),
                     state.deck.end());
    Fixture fixture;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "One searchable Energy type admitted the route.");
  }
  {
    sim::State state = exact_state();
    state.deck = {sim::Card::Grass, sim::Card::Fire};
    Fixture fixture;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "A deck without a Blender payload admitted the route.");
  }
  for (const sim::Card missing : {sim::Card::BrilliantBlender,
                                  sim::Card::Crispin}) {
    sim::State state = exact_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), missing));
    Fixture fixture;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "A missing route connector was ignored.");
  }
  {
    sim::State state = exact_state();
    state.supporter_used = true;
    Fixture fixture;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "A spent Supporter action admitted the route.");
  }
  {
    Fixture fixture(sim::DciProfile::NoDiscardControl);
    sim::EngineTestAccess::set_state(fixture.engine, exact_state());
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "A non-JIT policy admitted the route override.");
  }
}

void test_registered_seed_38_reaches_t3() {
  const auto selected = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value() && deck != nullptr,
         "The registered issue-1873 fixture is unavailable.");

  std::mt19937_64 rng{38};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*selected, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 3,
         "Registered seed 38 did not improve from T4 to T3.");
  expect(trace_contains(trace, "T3 | PLAY SUPPORTER") &&
             trace_contains(trace, "T3 | PLAY ITEM") &&
             trace_contains(trace, "T3 | READY"),
         "Registered seed 38 did not execute the T3 Crispin-Blender finish.");
}

}  // namespace

int main() {
  test_deck_search_and_prize_inspection_k1();
  test_exact_route_uses_crispin_then_blender();
  test_legality_and_resource_boundaries();
  test_registered_seed_38_reaches_t3();
  return 0;
}
