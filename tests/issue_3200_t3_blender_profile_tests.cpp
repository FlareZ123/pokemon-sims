#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
  }
  static bool initial_route(const Engine& engine) {
    return engine.wonder_tag_can_bank_steven_for_known_t3_blender_route();
  }
  static Card choose_supporter(Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
  static bool banked_route(const Engine& engine) {
    return engine.banked_steven_has_known_t3_blender_route();
  }
  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven();
  }
  static bool play_banked_steven(Engine& engine) {
    return engine.play_banked_steven_for_known_t3_blender_route();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State initial_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::BrilliantBlender, sim::Card::Grass,
                sim::Card::Fire};
  state.deck = {sim::Card::StevensResolve, sim::Card::RegidragoVstar,
                sim::Card::LatiasEx, sim::Card::Grass,
                sim::Card::MegaDragonite, sim::Card::QuickBall};
  return state;
}

sim::State banked_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 1, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::StevensResolve, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                sim::Card::Grass, sim::Card::MegaDragonite,
                sim::Card::QuickBall};
  state.manual_energy_used = true;
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::DciProfile dci, sim::State state,
                   const bool known = true)
      : scenario{"issue-3200", dci, sim::LockMode::None, true, 3},
        recipe(sim::baseline_recipe()),
        rng(3200),
        engine(scenario, recipe, rng) {
    sim::EngineTestAccess::set_state(engine, std::move(state), known);
  }
};

void shared_jit_initial_route() {
  for (const sim::DciProfile dci :
       {sim::DciProfile::StrictJit, sim::DciProfile::MatchupFlexJit}) {
    Fixture fixture{dci, initial_state()};
    // Brilliant Blender is held until the T3 ready turn, where it searches the deck
    // and discards the Dragon payload. Both repository JIT profiles require that same
    // ready-turn discard timing, so the already-proven Wonder Tag -> Steven route is
    // equally legal under either profile.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Shared JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3200
    expect(sim::EngineTestAccess::initial_route(fixture.engine),
           "Same-ready-turn JIT profile hid the T3 Blender Wonder Tag route");
    expect(sim::EngineTestAccess::choose_supporter(fixture.engine) ==
               sim::Card::StevensResolve,
           "Wonder Tag did not bank Steven for the T3 Blender route");
  }
}

void shared_jit_banked_route() {
  for (const sim::DciProfile dci :
       {sim::DciProfile::StrictJit, sim::DciProfile::MatchupFlexJit}) {
    Fixture fixture{dci, banked_state()};
    // The banked state already proves K1, first-player timing, a prior-turn Regidrago,
    // legal Latias mobility, Energy completion, held Blender, a deck payload, and T3
    // horizon. The only profile-dependent rule is the same-ready-turn payload timing.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Shared JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3200
    expect(sim::EngineTestAccess::banked_route(fixture.engine),
           "Same-ready-turn JIT profile hid the banked T3 Blender route");
    expect(sim::EngineTestAccess::should_play_steven(fixture.engine),
           "T2 Steven selector rejected the proven T3 Blender continuation");
    expect(sim::EngineTestAccess::play_banked_steven(fixture.engine),
           "Banked Steven did not resolve for the shared JIT route");

    const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
    expect(state.turn_ended && state.supporter_used,
           "Steven did not consume the Supporter action and end the turn");
    expect(std::find(state.hand.begin(), state.hand.end(),
                     sim::Card::RegidragoVstar) != state.hand.end(),
           "Steven did not find Regidrago VSTAR");
    expect(std::find(state.hand.begin(), state.hand.end(), sim::Card::Grass) !=
               state.hand.end(),
           "Steven did not find the completing Grass Energy");
    expect(std::find(state.hand.begin(), state.hand.end(), sim::Card::LatiasEx) !=
               state.hand.end(),
           "Steven did not find Latias ex");
  }
}

void no_discard_control_stays_separate() {
  Fixture initial{sim::DciProfile::NoDiscardControl, initial_state()};
  // NoDiscardControl has its established early-payload branch and remains supported
  // independently of strict_payload_timing().
  // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  expect(sim::EngineTestAccess::initial_route(initial.engine),
         "NoDiscardControl initial route changed while adding JIT parity");

  sim::State banked = banked_state();
  banked.discard.push_back(sim::Card::MegaDragonite);
  Fixture banked_fixture{sim::DciProfile::NoDiscardControl, std::move(banked)};
  expect(sim::EngineTestAccess::banked_route(banked_fixture.engine),
         "NoDiscardControl banked payload route changed while adding JIT parity");
}

void k0_still_blocks_known_route() {
  Fixture fixture{sim::DciProfile::MatchupFlexJit, initial_state(), false};
  // This exact route reads known deck targets, so it remains unavailable at K0.
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  expect(!sim::EngineTestAccess::initial_route(fixture.engine),
         "K0 exposed the deterministic known-deck T3 Blender route");
}
}  // namespace

int main() {
  try {
    shared_jit_initial_route();
    shared_jit_banked_route();
    no_discard_control_stays_separate();
    k0_still_blocks_known_route();
    std::cout << "Issue 3200 shared JIT profile tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
