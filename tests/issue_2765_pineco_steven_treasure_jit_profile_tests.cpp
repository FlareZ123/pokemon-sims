#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool public_preflight(const Engine& engine) {
    return engine.issue_1816_public_preflight();
  }
  static bool play_route(Engine& engine) {
    return engine.play_issue_1816_direct_t3_route();
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

sim::Scenario scenario(const sim::DciProfile dci) {
  return sim::Scenario{"issue-2765-jit-profile", dci,
                       sim::LockMode::FullRuleBoxAbility, true, 4};
}

sim::State exact_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.hand = {
      sim::Card::StevensResolve,
      sim::Card::Fire,
      sim::Card::Dragapult,
      sim::Card::Grant,
  };
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::MysteriousTreasure,
      sim::Card::SecretBox,
      sim::Card::Dragapult,
      sim::Card::MegaDragonite,
      sim::Card::Dipplin,
      sim::Card::QuickBall,
  };
  state.prizes = {
      sim::Card::TapuLeleGX,
      sim::Card::Gladion,
      sim::Card::Dawn,
      sim::Card::ProfessorBurnet,
      sim::Card::Fire,
      sim::Card::RegidragoV,
  };
  return state;
}

void same_turn_jit_profiles_share_the_steven_package() {
  // Both JIT profiles require the Dragon payload on the T3 ready turn:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Steven's Resolve can bank VSTAR, Grass, and Treasure and then ends T2: https://api.pokemontcg.io/v2/cards/sm7-145
  // Mysterious Treasure discards the held Dragon on T3: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dragapult ex is the held Dragon payload: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR's Apex Dragon costs GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, Item, discard, search, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1816
  // Confirmed cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2765
  for (const sim::DciProfile dci : {sim::DciProfile::StrictJit,
                                    sim::DciProfile::MatchupFlexJit}) {
    sim::Scenario selected = scenario(dci);
    sim::DeckRecipe recipe = sim::pineco_recipe();
    std::mt19937_64 rng{2765};
    sim::Engine engine{selected, recipe, rng};
    sim::EngineTestAccess::set_state(engine, exact_state());

    expect(sim::EngineTestAccess::public_preflight(engine),
           "A same-turn JIT profile rejected the Pineco Steven-Treasure route");
    expect(sim::EngineTestAccess::play_route(engine),
           "A same-turn JIT profile failed to bank the T3 Treasure package");

    const sim::State& state = sim::EngineTestAccess::state(engine);
    expect(state.turn_ended,
           "Steven's Resolve did not end T2 after banking the route");
    expect(contains(state.hand, sim::Card::RegidragoVstar) &&
               contains(state.hand, sim::Card::Grass) &&
               contains(state.hand, sim::Card::MysteriousTreasure),
           "Steven did not bank all three deterministic T3 targets");
    expect(contains(state.hand, sim::Card::Dragapult),
           "The Dragon payload was consumed before the T3 Treasure cost");
    expect(contains(state.deck, sim::Card::SecretBox),
           "The direct route unnecessarily consumed Secret Box");
  }
}

void no_discard_control_stays_outside_the_jit_preflight() {
  // No-discard-control has different payload timing and remains outside this helper:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed scope: https://github.com/FlareZ123/pokemon-sims/issues/2765
  sim::Scenario selected = scenario(sim::DciProfile::NoDiscardControl);
  sim::DeckRecipe recipe = sim::pineco_recipe();
  std::mt19937_64 rng{2765};
  sim::Engine engine{selected, recipe, rng};
  sim::EngineTestAccess::set_state(engine, exact_state());

  expect(!sim::EngineTestAccess::public_preflight(engine),
         "No-discard-control entered the same-turn-JIT-specific preflight");
}

}  // namespace

int main() {
  try {
    same_turn_jit_profiles_share_the_steven_package();
    no_discard_control_stays_outside_the_jit_preflight();
    std::cout << "issue 2765 JIT profile tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "issue 2765 JIT profile tests failed: " << error.what() << '\n';
    return 1;
  }
}
