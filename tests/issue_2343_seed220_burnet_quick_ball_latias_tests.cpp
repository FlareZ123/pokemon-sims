#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }

  static bool route_available(const Engine& engine) {
    return engine.issue_2343_burnet_quick_ball_latias_route_available();
  }

  static Card wonder_tag_target(const Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }

  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State seed220_t3_pre_wonder_tag_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 0,
                   sim::Tool::ForestSealStone},
      sim::Pokemon{sim::Card::TapuLeleGX, 3, 0, 0, sim::Tool::None},
  };
  state.hand = {sim::Card::QuickBall, sim::Card::StevensResolve,
                sim::Card::Fire};
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::Crispin,
                sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::Grass};
  return state;
}

void test_wonder_tag_selects_burnet_for_complete_t3_route() {
  sim::Scenario scenario{"issue-2343/flex", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  std::mt19937_64 rng{2343};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::EngineTestAccess::set_state(engine, seed220_t3_pre_wonder_tag_state());

  // The held Fire is the final manual GGF attachment. Wonder Tag can therefore
  // take Professor Burnet, Quick Ball can spend route-replaced Steven's Resolve
  // for Latias ex, Burnet supplies the same-turn Dragon payload, and Skyliner
  // supplies the free promotion. Crispin would spend the Supporter slot on an
  // Energy axis already covered by the held manual attachment.
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, Supporter, attachment, Bench, Ability, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed source-bound bug: https://github.com/FlareZ123/pokemon-sims/issues/2343
  expect(sim::EngineTestAccess::route_available(engine),
         "#2343 complete K1 T3 route was not recognized.");
  expect(sim::EngineTestAccess::wonder_tag_target(engine) ==
             sim::Card::ProfessorBurnet,
         "#2343 Wonder Tag still preferred Crispin over the complete Burnet route.");
}

void test_quick_ball_spends_replaced_steven_after_burnet_search() {
  sim::Scenario scenario{"issue-2343/flex", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  std::mt19937_64 rng{23431};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::State state = seed220_t3_pre_wonder_tag_state();
  state.hand = {sim::Card::QuickBall, sim::Card::StevensResolve,
                sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::Grass};
  state.bench.front().fire = 1;
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::route_available(engine),
         "#2343 route did not survive after the final manual attachment.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "#2343 Quick Ball did not resolve with Steven as the final route cost.");
  const sim::State& result = engine.state();
  expect(std::find(result.discard.begin(), result.discard.end(),
                   sim::Card::StevensResolve) != result.discard.end(),
         "#2343 Quick Ball did not discard route-replaced Steven's Resolve.");
  expect(std::find(result.hand.begin(), result.hand.end(), sim::Card::LatiasEx) !=
             result.hand.end(),
         "#2343 Quick Ball did not search Latias ex.");
}

void test_route_requires_every_completion_gate() {
  sim::Scenario scenario{"issue-2343/flex", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  std::mt19937_64 rng{23432};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};

  sim::State state = seed220_t3_pre_wonder_tag_state();
  sim::EngineTestAccess::set_state(engine, state, false);
  expect(!sim::EngineTestAccess::route_available(engine),
         "#2343 K1-only route was admitted at K0.");

  state = seed220_t3_pre_wonder_tag_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::ProfessorBurnet), state.deck.end());
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "#2343 route was admitted without Burnet.");

  state = seed220_t3_pre_wonder_tag_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::LatiasEx), state.deck.end());
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "#2343 route was admitted without Latias ex in the known deck.");

  state = seed220_t3_pre_wonder_tag_state();
  state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(),
                                  sim::is_payload), state.deck.end());
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "#2343 route was admitted without a Burnet payload.");

  state = seed220_t3_pre_wonder_tag_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::Fire),
                   state.hand.end());
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "#2343 route was admitted before the Energy axis could complete.");

  state = seed220_t3_pre_wonder_tag_state();
  state.retreat_used = true;
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "#2343 route was admitted after retreat was already used.");

  state = seed220_t3_pre_wonder_tag_state();
  state.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None});
  state.bench.push_back(sim::Pokemon{sim::Card::CrobatV, 1, 0, 0, sim::Tool::None});
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "#2343 route was admitted with a full Bench.");

  state = seed220_t3_pre_wonder_tag_state();
  state.hand.push_back(sim::Card::MegaDragonite);
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "#2343 indirect Burnet route displaced a held payload route.");

  state = seed220_t3_pre_wonder_tag_state();
  state.hand.push_back(sim::Card::BrilliantBlender);
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "#2343 indirect Burnet route displaced a live Blender route.");
}

void test_route_rejects_lock_modes() {
  const sim::State state = seed220_t3_pre_wonder_tag_state();
  for (const sim::LockMode lock : {sim::LockMode::TurnTwoItem,
                                   sim::LockMode::FullRuleBoxAbility,
                                   sim::LockMode::FullSupporter,
                                   sim::LockMode::FullCombined}) {
    sim::Scenario scenario{"issue-2343/lock", sim::DciProfile::MatchupFlexJit,
                           lock, false, 4};
    std::mt19937_64 rng{23433 + static_cast<unsigned>(lock)};
    sim::Engine engine{scenario, sim::baseline_recipe(), rng};
    sim::EngineTestAccess::set_state(engine, state);
    expect(!sim::EngineTestAccess::route_available(engine),
           "#2343 route ignored a lock gate.");
  }
}

void test_exact_seed_220_is_ready_on_t3() {
  sim::Scenario scenario{"matchup-flex-jit/go-second",
                         sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  std::mt19937_64 rng{220};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  const sim::TrialOutcome outcome = engine.run();
  expect(outcome.first_ready_turn == 3,
         "#2343 exact seed 220 did not improve from T4 to the proven T3 finish.");
}

}  // namespace

int main() {
  test_wonder_tag_selects_burnet_for_complete_t3_route();
  test_quick_ball_spends_replaced_steven_after_burnet_search();
  test_route_requires_every_completion_gate();
  test_route_rejects_lock_modes();
  test_exact_seed_220_is_ready_on_t3();
  return 0;
}
