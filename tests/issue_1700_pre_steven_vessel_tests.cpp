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
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route(const Engine& engine) {
    return engine.issue_1700_pre_steven_vessel_route_available();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

sim::State issue_1926_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0}};
  state.hand = {sim::Card::StevensResolve, sim::Card::EarthenVessel,
                sim::Card::BrilliantBlender, sim::Card::RegidragoVstar,
                sim::Card::Dragapult};
  state.deck = {sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Crispin, sim::Card::Gladion};
  return state;
}

void verify_k1_provenance_equivalence() {
  const auto route_is_live = [](const bool deck_seen,
                                const bool prizes_revealed,
                                const std::uint64_t seed) {
    const sim::Scenario scenario{
        "issue-1926", sim::DciProfile::StrictJit, sim::LockMode::None, false, 5};
    const sim::DeckRecipe recipe = sim::baseline_recipe();
    std::mt19937_64 rng{seed};
    sim::Engine engine{scenario, recipe, rng};
    sim::EngineTestAccess::set_state(
        engine, issue_1926_state(), deck_seen, prizes_revealed);
    return sim::EngineTestAccess::route(engine);
  };

  // Either legal inspection supplies the same K1 knowledge for the pre-Steven
  // Vessel route. K0 still cannot inspect the deck or Prize identities:
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Correction precedent: https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Official Item, search, attachment, Supporter, evolution, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1926
  expect(route_is_live(true, false, 192601),
         "Deck-search K1 rejected the pre-Steven Vessel route");
  expect(route_is_live(false, true, 192602),
         "Prize-inspection K1 rejected the pre-Steven Vessel route");
  expect(!route_is_live(false, false, 192603),
         "K0 used the pre-Steven Vessel route");
}

void verify_modeling_seed_218() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat1-heavy-ball");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1700 modeling setup is unavailable.");

  std::mt19937_64 rng{218};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Vessel may discard the held Dragon, search Grass and Fire, and attach Grass
  // before Steven ends T1. Steven then searches Latias ex and the second Grass.
  // The prior-turn Regidrago evolves and retreats on T2, then Fire plus Blender
  // establishes the matchup-flex current-turn payload and T3 ready state:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1700
  expect(outcome.first_ready_turn == 3,
         "Modeling seed 218 must complete the deterministic T3 route.");
  expect(trace_contains(trace,
                        "Earthen Vessel searched Grass and Fire before Steven's Resolve") &&
             trace_contains(trace,
                            "Grass Energy manually to Regidrago V before Steven's Resolve") &&
             trace_contains(trace,
                            "Searched the complete post-Vessel T3 route: Latias ex, Grass Energy") &&
             trace_contains(trace, "T3 | READY"),
         "Modeling seed 218 did not preserve the complete post-Vessel route.");
}

void verify_registered_seed_218(const std::string& label) {
  const auto scenario = sim::scenario_by_label(label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1700 registered setup is unavailable.");

  std::mt19937_64 rng{218};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // All Fire Energy is known prized after Wonder Tag. Vessel searches two Grass and
  // makes the first attachment before Steven. Steven preserves the missing VSTAR axis
  // when its held copy paid the strict-JIT cost, then searches Latias ex and Gladion.
  // Gladion recovers Fire on T2 and the final attachment plus Blender reaches T3:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1700
  expect(outcome.first_ready_turn == 3,
         "Registered seed 218 must complete the deterministic T3 route.");
  expect(trace_contains(trace,
                        "Earthen Vessel searched two Grass before Steven's Resolve; Fire is known prized") &&
             trace_contains(trace, "Gladion") &&
             trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-GLADION-01") &&
             trace_contains(trace, "T3 | READY"),
         "Registered seed 218 did not preserve the Prize-recovery route.");
}

void verify_lock_control() {
  const sim::Scenario scenario{
      "issue-1700-synthetic-full-item-lock", sim::DciProfile::StrictJit,
      sim::LockMode::FullItem, false, 5};
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "Issue 1700 lock-control deck is unavailable.");

  std::mt19937_64 rng{218};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, deck->recipe, rng, &trace);
  static_cast<void>(engine.run());
  // FullItem remains available only as an explicit synthetic/historical fixture.
  // Current-paper aggregate labels for turn-one Item lock are retired:
  // https://assets.pokemon.com/assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/2247
  expect(!trace_contains(trace, "post-Vessel T3 route"),
         "The synthetic full-lock control must not play Earthen Vessel through Item lock.");
}

}  // namespace

int main() {
  try {
    verify_k1_provenance_equivalence();
    verify_modeling_seed_218();
    verify_registered_seed_218("strict-jit/go-second");
    verify_registered_seed_218("matchup-flex-jit/go-second");
    verify_registered_seed_218("no-discard-control/go-second");
    verify_lock_control();
    std::cout << "Issue 1700 pre-Steven Vessel route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
