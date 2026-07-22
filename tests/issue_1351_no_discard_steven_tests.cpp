#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static Card choose_supporter_after_search_started(Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const sim::DciProfile profile, const sim::LockMode locks,
          const std::uint64_t seed)
      : scenario{"issue-1351-no-discard-steven", profile, locks, true, 4},
        recipe{sim::baseline_recipe()},
        rng{seed},
        engine{scenario, recipe, rng} {}
};

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State turn_one_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::BrilliantBlender};
  state.deck = {
      sim::Card::StevensResolve,
      sim::Card::RegidragoVstar,
      sim::Card::LatiasEx,
      sim::Card::Grass,
      sim::Card::MegaDragonite,
      sim::Card::Arven,
  };
  return state;
}

sim::State turn_two_banked_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0}};
  state.hand = {sim::Card::StevensResolve, sim::Card::Fire};
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::LatiasEx,
      sim::Card::Grass,
      sim::Card::Arven,
  };
  state.discard = {sim::Card::BrilliantBlender, sim::Card::MegaDragonite};
  return state;
}

void no_discard_wonder_tag_banks_steven() {
  Fixture fixture{sim::DciProfile::NoDiscardControl, sim::LockMode::None, 135101};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(engine, turn_one_state());

  // Wonder Tag may bank Steven on the first player's opening turn. Under
  // no-discard-control, Brilliant Blender may bank the Dragon payload immediately,
  // so Steven's VSTAR, Grass, and Latias package remains the deterministic T3 route:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1351
  expect(sim::EngineTestAccess::choose_supporter_after_search_started(engine) ==
             sim::Card::StevensResolve,
         "No-discard Wonder Tag did not bank Steven's Resolve");
}

void banked_payload_continuation_searches_every_t3_axis() {
  Fixture fixture{sim::DciProfile::NoDiscardControl, sim::LockMode::None, 135102};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(engine, turn_two_banked_state());

  expect(sim::EngineTestAccess::should_play_steven(engine),
         "Banked no-discard payload did not admit the T2 Steven continuation");
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& state = sim::EngineTestAccess::state(engine);
  // Blender and a modeled Dragon in discard are public proof that the payload axis
  // was completed on T1. The T2 Fire attachment plus Steven's unrestricted search
  // reserve the remaining T3 evolution, Grass, and free-Retreat axes:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1351
  expect(contains(state.hand, sim::Card::RegidragoVstar),
         "Steven did not reserve Regidrago VSTAR");
  expect(contains(state.hand, sim::Card::Grass),
         "Steven did not reserve the direct third Grass Energy");
  expect(contains(state.hand, sim::Card::LatiasEx),
         "Steven did not reserve Latias ex");
  expect(state.bench.front().grass == 1 && state.bench.front().fire == 1,
         "The held Fire was not attached before Steven resolved");
  expect(contains(state.discard, sim::Card::BrilliantBlender) &&
             contains(state.discard, sim::Card::MegaDragonite),
         "The public T1 Blender payload proof was lost");
  expect(state.turn_ended, "Steven's Resolve did not end turn two");
}

void no_discard_continuation_requires_public_banking_proof() {
  {
    Fixture fixture{sim::DciProfile::NoDiscardControl, sim::LockMode::None, 135103};
    sim::Engine& engine = fixture.engine;
    sim::State state = turn_two_banked_state();
    state.discard.erase(std::find(state.discard.begin(), state.discard.end(),
                                  sim::Card::BrilliantBlender));
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::should_play_steven(engine),
           "Steven was admitted without public Brilliant Blender proof");
  }
  {
    Fixture fixture{sim::DciProfile::NoDiscardControl, sim::LockMode::None, 135104};
    sim::Engine& engine = fixture.engine;
    sim::State state = turn_two_banked_state();
    state.discard.erase(std::find(state.discard.begin(), state.discard.end(),
                                  sim::Card::MegaDragonite));
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::should_play_steven(engine),
           "Steven was admitted without a banked Dragon payload");
  }
  {
    Fixture fixture{sim::DciProfile::NoDiscardControl,
                    sim::LockMode::TurnTwoItem, 135105};
    sim::Engine& engine = fixture.engine;
    sim::EngineTestAccess::set_state(engine, turn_two_banked_state());
    expect(!sim::EngineTestAccess::should_play_steven(engine),
           "The no-lock route was broadened through scheduled Item lock");
  }
}

void strict_jit_route_remains_unchanged() {
  Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None, 135106};
  sim::Engine& engine = fixture.engine;
  sim::State state = turn_two_banked_state();
  state.discard.clear();
  state.hand.push_back(sim::Card::BrilliantBlender);
  state.deck.push_back(sim::Card::MegaDragonite);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Strict JIT still requires Blender to remain held and a payload to remain in deck
  // for the ready-turn discard. The no-discard branch must not weaken that contract:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/946
  // https://github.com/FlareZ123/pokemon-sims/issues/1351
  expect(sim::EngineTestAccess::should_play_steven(engine),
         "The existing strict-JIT banked Steven route regressed");
}

}  // namespace

int main() {
  no_discard_wonder_tag_banks_steven();
  banked_payload_continuation_searches_every_t3_axis();
  no_discard_continuation_requires_public_banking_proof();
  strict_jit_route_remains_unchanged();
  return 0;
}
