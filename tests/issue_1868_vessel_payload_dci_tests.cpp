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
  static bool play_vessel(Engine& engine) {
    return engine.play_earthen_vessel(false);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::DciProfile profile = sim::DciProfile::MatchupFlexJit,
                       const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1868-vessel-payload-dci", profile, lock, true, 5};
}

sim::State complete_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::Guzma, sim::Card::DialgaGX,
                sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass, sim::Card::Grass,
                sim::Card::QuickBall};
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(
      const sim::DciProfile profile = sim::DciProfile::MatchupFlexJit,
      const sim::LockMode lock = sim::LockMode::None)
      : scenario_value(scenario(profile, lock)),
        recipe(sim::deck_by_id("regidrago-shell")->recipe),
        rng(1868),
        engine(scenario_value, recipe, rng) {}
};

void exact_k1_state_selects_the_dragon_payload() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, complete_state());
  // The same mandatory Earthen Vessel cost that pays for the final Grass search
  // must create the current-turn Dragon payload instead of spending a redundant
  // Regidrago VSTAR and leaving the ready state incomplete:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/1868
  expect(sim::EngineTestAccess::play_vessel(fixture.engine),
         "The complete Vessel route was rejected");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(std::find(after.discard.begin(), after.discard.end(),
                   sim::Card::DialgaGX) != after.discard.end(),
         "Dialga-GX did not enter discard as the Vessel cost");
  expect(std::find(after.discarded_this_turn.begin(),
                   after.discarded_this_turn.end(), sim::Card::DialgaGX) !=
             after.discarded_this_turn.end(),
         "The payload was not recorded for current-turn JIT");
  expect(std::find(after.hand.begin(), after.hand.end(), sim::Card::Grass) !=
             after.hand.end(),
         "Vessel did not search the final Grass Energy");
}

void selector_preserves_the_documented_boundaries() {
  const auto did_discard_dialga = [](const sim::Engine& engine) {
    const sim::State& state = sim::EngineTestAccess::state(engine);
    return std::find(state.discarded_this_turn.begin(),
                     state.discarded_this_turn.end(), sim::Card::DialgaGX) !=
           state.discarded_this_turn.end();
  };
  {
    Fixture strict{sim::DciProfile::StrictJit};
    sim::EngineTestAccess::set_state(strict.engine, complete_state());
    sim::EngineTestAccess::play_vessel(strict.engine);
    expect(!did_discard_dialga(strict.engine),
           "Matchup-flex payload override escaped into strict JIT");
  }
  {
    Fixture k0;
    sim::EngineTestAccess::set_state(k0.engine, complete_state(), false);
    sim::EngineTestAccess::play_vessel(k0.engine);
    expect(!did_discard_dialga(k0.engine),
           "K0 state spent a payload without a legal inspection");
  }
  {
    Fixture attachment_spent;
    sim::State state = complete_state();
    state.manual_energy_used = true;
    sim::EngineTestAccess::set_state(attachment_spent.engine, std::move(state));
    sim::EngineTestAccess::play_vessel(attachment_spent.engine);
    expect(!did_discard_dialga(attachment_spent.engine),
           "Spent attachment window still consumed the payload");
  }
  {
    Fixture no_grass;
    sim::State state = complete_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::Grass), state.deck.end());
    sim::EngineTestAccess::set_state(no_grass.engine, std::move(state));
    sim::EngineTestAccess::play_vessel(no_grass.engine);
    expect(!did_discard_dialga(no_grass.engine),
           "Missing Grass target still consumed the payload");
  }
  {
    Fixture item_lock{sim::DciProfile::MatchupFlexJit,
                      sim::LockMode::FullItem};
    sim::EngineTestAccess::set_state(item_lock.engine, complete_state());
    sim::EngineTestAccess::play_vessel(item_lock.engine);
    expect(!did_discard_dialga(item_lock.engine),
           "Item lock still consumed the payload");
  }
  {
    Fixture payload_complete;
    sim::State state = complete_state();
    state.discard.push_back(sim::Card::MegaDragonite);
    state.discarded_this_turn.push_back(sim::Card::MegaDragonite);
    sim::EngineTestAccess::set_state(payload_complete.engine, std::move(state));
    sim::EngineTestAccess::play_vessel(payload_complete.engine);
    expect(!did_discard_dialga(payload_complete.engine),
           "A completed current-turn payload axis consumed another Dragon");
  }
}

void exact_seed_reaches_turn_four(const char* label) {
  const auto selected = sim::scenario_by_label(label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value(), "Missing matchup-flex scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{199};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  expect(outcome.first_ready_turn == 4,
         "Seed 199 did not improve to deterministic T4 readiness");
  expect(contains("Dialga-GX (Earthen Vessel cost)"),
         "Seed 199 did not use Dialga-GX as the Vessel payload cost");
  expect(contains("T4 | READY"), "Seed 199 was not ready on T4");
}

}  // namespace

int main() {
  try {
    exact_k1_state_selects_the_dragon_payload();
    selector_preserves_the_documented_boundaries();
    exact_seed_reaches_turn_four("matchup-flex-jit/go-first");
    exact_seed_reaches_turn_four("matchup-flex-jit/go-second");
  } catch (const std::exception& error) {
    std::cerr << "issue-1868 Vessel payload DCI test failure: "
              << error.what() << '\n';
    return 1;
  }
  return 0;
}
