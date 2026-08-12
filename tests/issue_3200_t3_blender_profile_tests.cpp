#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
  }

  static bool initial_route(const Engine& engine) {
    return engine.issue_3200_matchup_flex_t3_blender_initial_route();
  }

  static Card choose_supporter(Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }

  static bool banked_route(const Engine& engine) {
    return engine.issue_3200_matchup_flex_t3_blender_banked_route();
  }

  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven_issue3200();
  }

  static bool play_banked_steven(Engine& engine) {
    return engine.play_banked_steven_for_known_t3_blender_route();
  }

  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
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

sim::Engine make_engine(const sim::DciProfile dci, sim::State state,
                        const bool known = true) {
  const sim::Scenario scenario{"issue-3200", dci, sim::LockMode::None,
                               true, 3};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3200);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return engine;
}

void test_matchup_flex_initial_wonder_tag_route() {
  // Brilliant Blender discards the Dragon payload on the T3 ready turn, so both
  // same-ready-turn JIT profiles admit the same deterministic banked route:
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Same-ready-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Advanced turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3200
  sim::Engine engine = make_engine(sim::DciProfile::MatchupFlexJit,
                                   initial_state());
  expect(sim::EngineTestAccess::initial_route(engine),
         "MatchupFlexJit hid the known T3 Blender Wonder Tag route");
  expect(sim::EngineTestAccess::choose_supporter(engine) ==
             sim::Card::StevensResolve,
         "Wonder Tag did not bank Steven for the MatchupFlexJit T3 route");
}

void test_matchup_flex_banked_steven_route() {
  sim::Engine engine = make_engine(sim::DciProfile::MatchupFlexJit,
                                   banked_state());
  expect(sim::EngineTestAccess::banked_route(engine),
         "MatchupFlexJit hid the banked T3 Blender continuation");
  expect(sim::EngineTestAccess::should_play_steven(engine),
         "T2 Steven selector rejected the MatchupFlexJit continuation");
  expect(sim::EngineTestAccess::play_banked_steven(engine),
         "Banked Steven did not resolve for MatchupFlexJit");

  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(state.turn_ended, "Steven's Resolve did not end the turn");
  expect(state.supporter_used, "Steven's Resolve did not consume Supporter use");
  expect(std::find(state.hand.begin(), state.hand.end(), sim::Card::RegidragoVstar) != state.hand.end(),
         "Steven did not find Regidrago VSTAR");
  expect(std::find(state.hand.begin(), state.hand.end(), sim::Card::Grass) != state.hand.end(),
         "Steven did not find the completing Grass Energy");
  expect(std::find(state.hand.begin(), state.hand.end(), sim::Card::LatiasEx) != state.hand.end(),
         "Steven did not find Latias ex");
}

void test_profile_and_knowledge_boundaries() {
  sim::Engine strict = make_engine(sim::DciProfile::StrictJit, initial_state());
  expect(!sim::EngineTestAccess::initial_route(strict),
         "Issue-3200 additive helper should leave StrictJit to the historical route");

  sim::Engine no_control = make_engine(sim::DciProfile::NoDiscardControl,
                                       initial_state());
  expect(!sim::EngineTestAccess::initial_route(no_control),
         "Issue-3200 additive helper changed NoDiscardControl semantics");

  sim::Engine k0 = make_engine(sim::DciProfile::MatchupFlexJit,
                               initial_state(), false);
  expect(!sim::EngineTestAccess::initial_route(k0),
         "K0 exposed the K1 deterministic T3 Blender route");
}

}  // namespace

int main() {
  test_matchup_flex_initial_wonder_tag_route();
  test_matchup_flex_banked_steven_route();
  test_profile_and_knowledge_boundaries();
  return 0;
}
