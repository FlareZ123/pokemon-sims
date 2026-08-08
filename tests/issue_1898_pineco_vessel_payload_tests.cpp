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

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::Scenario scenario(const sim::DciProfile profile =
                           sim::DciProfile::MatchupFlexJit,
                       const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1898-pineco-vessel-payload", profile, lock,
                       true, 5};
}

sim::State complete_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Guzma, sim::Card::Crispin,
                sim::Card::Dragapult, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Fire, sim::Card::Grass,
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
        recipe(sim::deck_by_id("regidrago-pineco")->recipe),
        rng(1898),
        engine(scenario_value, recipe, rng) {}
};

void exact_k1_state_uses_the_dragon_and_preserves_guzma() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, complete_state());

  // The mandatory Vessel cost and searched Fire complete both missing setup
  // axes in one public-state route, while Guzma keeps its discrete value:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Guzma: https://api.pokemontcg.io/v2/cards/sm3-115
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1898
  expect(sim::EngineTestAccess::play_vessel(fixture.engine),
         "The complete issue-1898 Vessel route was rejected.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.discarded_this_turn, sim::Card::Dragapult),
         "Dragapult ex did not become the current-turn Vessel payload.");
  expect(contains(after.hand, sim::Card::Guzma),
         "The route discarded Guzma instead of preserving its discrete value.");
  expect(contains(after.hand, sim::Card::Fire),
         "Vessel did not search the missing Fire Energy.");
}

void selector_preserves_required_boundaries() {
  const auto spent_payload = [](const sim::Engine& engine) {
    return contains(sim::EngineTestAccess::state(engine).discarded_this_turn,
                    sim::Card::Dragapult);
  };
  {
    Fixture strict{sim::DciProfile::StrictJit};
    sim::EngineTestAccess::set_state(strict.engine, complete_state());
    sim::EngineTestAccess::play_vessel(strict.engine);
    // Strict JIT has the same current-turn payload requirement in this public
    // GG state, so Vessel's mandatory Dragon discard is the strongest legal cost.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Strict-JIT contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed strict regression: https://github.com/FlareZ123/pokemon-sims/issues/2231
    expect(spent_payload(strict.engine),
           "The confirmed strict-JIT GG Vessel route did not spend its payload.");
  }
  {
    Fixture k0;
    sim::EngineTestAccess::set_state(k0.engine, complete_state(), false);
    sim::EngineTestAccess::play_vessel(k0.engine);
    expect(!spent_payload(k0.engine), "K0 spent a payload without inspection.");
  }
  {
    Fixture attachment_spent;
    sim::State state = complete_state();
    state.manual_energy_used = true;
    sim::EngineTestAccess::set_state(attachment_spent.engine, std::move(state));
    sim::EngineTestAccess::play_vessel(attachment_spent.engine);
    expect(!spent_payload(attachment_spent.engine),
           "A spent manual attachment still consumed the payload.");
  }
  {
    Fixture no_fire;
    sim::State state = complete_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::Fire), state.deck.end());
    sim::EngineTestAccess::set_state(no_fire.engine, std::move(state));
    sim::EngineTestAccess::play_vessel(no_fire.engine);
    expect(!spent_payload(no_fire.engine),
           "A missing Fire target still consumed the payload.");
  }
  {
    Fixture item_lock{sim::DciProfile::MatchupFlexJit,
                      sim::LockMode::FullItem};
    sim::EngineTestAccess::set_state(item_lock.engine, complete_state());
    sim::EngineTestAccess::play_vessel(item_lock.engine);
    expect(!spent_payload(item_lock.engine),
           "Item lock still consumed the payload.");
  }
  {
    Fixture payload_complete;
    sim::State state = complete_state();
    state.discard.push_back(sim::Card::MegaDragonite);
    state.discarded_this_turn.push_back(sim::Card::MegaDragonite);
    sim::EngineTestAccess::set_state(payload_complete.engine, std::move(state));
    sim::EngineTestAccess::play_vessel(payload_complete.engine);
    expect(!spent_payload(payload_complete.engine),
           "A completed payload axis consumed another Dragon.");
  }
}

void registered_seed_258_reaches_t3() {
  const auto selected = sim::scenario_by_label("matchup-flex-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(selected.has_value() && deck != nullptr,
         "The registered issue-1898 fixture is unavailable.");

  std::mt19937_64 rng(258);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*selected, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 3,
         "Pineco seed 258 did not improve to T3 readiness.");
  expect(trace_contains(trace, "Dragapult ex (Earthen Vessel cost)"),
         "Seed 258 did not use Dragapult ex as the Vessel payload cost.");
  expect(!trace_contains(trace, "Guzma (Earthen Vessel cost)"),
         "Seed 258 still discarded Guzma to Earthen Vessel.");
  expect(trace_contains(trace, "T3 | READY"),
         "Seed 258 was not ready on T3.");
}
}  // namespace

int main() {
  try {
    exact_k1_state_uses_the_dragon_and_preserves_guzma();
    selector_preserves_required_boundaries();
    registered_seed_258_reaches_t3();
    std::cout << "Issue 1898 Pineco Vessel payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}