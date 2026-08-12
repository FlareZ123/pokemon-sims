#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1797_wonder_tag_steven_route_available();
  }
  static Card supporter_after_search(const Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::DciProfile dci,
                       const sim::LockMode locks = sim::LockMode::None,
                       const bool going_first = true) {
  return sim::Scenario{"issue-3165-wonder-tag-jit-gate", dci, locks,
                       going_first, 3};
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::Grass,
  };
  state.deck = {
      sim::Card::StevensResolve,
      sim::Card::Crispin,
      sim::Card::EarthenVessel,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
  };
  state.discard = {sim::Card::QuickBall, sim::Card::TateLiza};
  return state;
}

void shared_jit_profiles_choose_steven() {
  std::mt19937_64 rng(3165);
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  for (const sim::DciProfile dci :
       {sim::DciProfile::StrictJit, sim::DciProfile::MatchupFlexJit}) {
    sim::Engine engine(scenario(dci), recipe, rng);
    sim::EngineTestAccess::set_state(engine, route_state());

    // Wonder Tag may search any Supporter after Quick Ball establishes K1.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Both JIT profiles require the Dragon payload on the eventual ready turn:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed selector regression: https://github.com/FlareZ123/pokemon-sims/issues/3165
    expect(sim::EngineTestAccess::route_available(engine),
           "A same-ready-turn JIT profile was rejected by the #1797 selector");
    expect(sim::EngineTestAccess::supporter_after_search(engine) ==
               sim::Card::StevensResolve,
           "Wonder Tag did not select Steven's Resolve for the #1797 route");
  }
}

void semantic_negatives_remain_enforced() {
  std::mt19937_64 rng(3165001);
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  sim::Engine control(
      scenario(sim::DciProfile::NoDiscardControl), recipe, rng);
  sim::EngineTestAccess::set_state(control, route_state());
  // NoDiscardControl is a distinct early-banking model, so it must stay outside
  // this same-ready-turn JIT route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  expect(!sim::EngineTestAccess::route_available(control),
         "NoDiscardControl incorrectly entered the #1797 JIT route");

  sim::Engine k0(
      scenario(sim::DciProfile::MatchupFlexJit), recipe, rng);
  sim::EngineTestAccess::set_state(k0, route_state(), false);
  // Quick Ball's deck search is what establishes K1 for this witness:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  expect(!sim::EngineTestAccess::route_available(k0),
         "K0 incorrectly entered the #1797 post-search route");

  sim::Engine ability_locked(
      scenario(sim::DciProfile::MatchupFlexJit,
               sim::LockMode::FullRuleBoxAbility),
      recipe, rng);
  sim::EngineTestAccess::set_state(ability_locked, route_state());
  // Wonder Tag is a Pokémon-GX Ability and remains unavailable under the modeled
  // Rule Box Ability lock: https://api.pokemontcg.io/v2/cards/sm2-60
  // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  expect(!sim::EngineTestAccess::route_available(ability_locked),
         "Rule Box Ability lock incorrectly admitted Wonder Tag");

  sim::Engine second(
      scenario(sim::DciProfile::MatchupFlexJit, sim::LockMode::None, false),
      recipe, rng);
  sim::EngineTestAccess::set_state(second, route_state());
  // #3165 intentionally preserves the original T1-going-first route boundary.
  // Confirmed scope: https://github.com/FlareZ123/pokemon-sims/issues/3165
  expect(!sim::EngineTestAccess::route_available(second),
         "#3165 accidentally widened the preserved seat boundary");
}
}  // namespace

int main() {
  try {
    shared_jit_profiles_choose_steven();
    semantic_negatives_remain_enforced();
    std::cout << "Issue 3165 Wonder Tag JIT gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
