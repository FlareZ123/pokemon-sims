#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static bool play_route(Engine& engine) {
    return engine.play_issue_1868_vessel_payload_finish();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool has(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::Scenario scenario(const sim::DciProfile profile = sim::DciProfile::StrictJit,
                       const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-2224", profile, lock, false, 4};
}

sim::State route_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 2, 1, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::EarthenVessel, sim::Card::MegaDragonite,
                sim::Card::ErikasInvitation};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoV};
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const sim::DciProfile profile, const sim::LockMode lock,
          sim::State state, const bool deck_seen = true)
      : scenario_value(scenario(profile, lock)),
        recipe(sim::baseline_recipe()),
        rng(2224001),
        engine(scenario_value, recipe, rng) {
    sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  }
};

void test_strict_and_matchup_flex_parity() {
  for (const sim::DciProfile profile : {sim::DciProfile::StrictJit,
                                        sim::DciProfile::MatchupFlexJit}) {
    Fixture fixture{profile, sim::LockMode::None, route_state()};

    // Earthen Vessel may discard the held Dragon and search Basic Grass; the unused
    // manual attachment can then complete GGF on the same current-turn JIT route.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict/matchup-flex JIT, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2224
    expect(sim::EngineTestAccess::play_route(fixture.engine),
           "The proven held-payload Vessel route was rejected.");
    expect(has(fixture.engine.state().discard, sim::Card::MegaDragonite) &&
               has(fixture.engine.state().discard, sim::Card::EarthenVessel),
           "Vessel did not use the held Dragon as its current-turn payload cost.");
    expect(has(fixture.engine.state().hand, sim::Card::Grass),
           "Vessel did not search the missing Basic Grass Energy.");
  }
}

void test_route_boundaries() {
  {
    Fixture fixture{sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                    route_state()};
    expect(!sim::EngineTestAccess::play_route(fixture.engine),
           "No-discard-control entered the strict/matchup-flex JIT override.");
  }
  {
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None,
                    route_state(), false};
    expect(!sim::EngineTestAccess::play_route(fixture.engine),
           "K0 entered an exact-deck held-payload Vessel route.");
  }
  {
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::FullItem,
                    route_state()};
    expect(!sim::EngineTestAccess::play_route(fixture.engine),
           "Item lock admitted Earthen Vessel.");
  }
  {
    sim::State state = route_state();
    state.manual_energy_used = true;
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None,
                    std::move(state)};
    expect(!sim::EngineTestAccess::play_route(fixture.engine),
           "A spent manual attachment admitted the one-Energy finish.");
  }
  {
    sim::State state = route_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::Grass), state.deck.end());
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None,
                    std::move(state)};
    expect(!sim::EngineTestAccess::play_route(fixture.engine),
           "K1 admitted the route without a Basic Grass target.");
  }
  {
    sim::State state = route_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::MegaDragonite), state.hand.end());
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None,
                    std::move(state)};
    expect(!sim::EngineTestAccess::play_route(fixture.engine),
           "The route ran without a held permitted Dragon payload.");
  }
  {
    sim::State state = route_state();
    state.active->grass = 2;
    state.active->fire = 0;
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None,
                    std::move(state)};
    expect(!sim::EngineTestAccess::play_route(fixture.engine),
           "The missing-Fire orientation leaked into the #1868 Grass route.");
  }
  {
    sim::State state = route_state();
    state.discard.push_back(sim::Card::Dragapult);
    state.discarded_this_turn.push_back(sim::Card::Dragapult);
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None,
                    std::move(state)};
    expect(!sim::EngineTestAccess::play_route(fixture.engine),
           "An already-established current-turn payload spent another Dragon.");
  }

  // Boundary legality and timing sources: https://api.pokemontcg.io/v2/cards/sv4-163 https://api.pokemontcg.io/v2/cards/swsh12-136 https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1 and JIT boundaries: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Original and strict regressions: https://github.com/FlareZ123/pokemon-sims/issues/1868 https://github.com/FlareZ123/pokemon-sims/issues/2224
}

void test_registered_seed_939_reaches_t4() {
  const auto selected = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value() && deck != nullptr,
         "The registered issue-2224 fixture is unavailable.");

  std::mt19937_64 rng(939);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*selected, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Source-bound witness and expected legal T4 finish:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2224
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "strict-jit/go-second seed 939 did not reach T4 readiness.");
  expect(trace_has(trace, "T4 | PLAY SUPPORTER | rules: R-GLADION-01") &&
             trace_has(trace, "Mega Dragonite ex (Earthen Vessel cost)") &&
             trace_has(trace, "T4 | READY"),
         "seed 939 omitted the Gladion-Vessel held-payload T4 finish.");
}

}  // namespace

int main() {
  test_strict_and_matchup_flex_parity();
  test_route_boundaries();
  test_registered_seed_939_reaches_t4();
}
