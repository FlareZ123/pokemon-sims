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
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Scenario scenario(const sim::DciProfile profile) {
  return sim::Scenario{"issue-2765-pineco-jit-profile", profile,
                       sim::LockMode::FullRuleBoxAbility, true, 4};
}

sim::State exact_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::Fire,
                sim::Card::Dragapult, sim::Card::Grant};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::MysteriousTreasure, sim::Card::SecretBox,
                sim::Card::Dragapult, sim::Card::MegaDragonite,
                sim::Card::Dipplin, sim::Card::QuickBall};
  state.prizes = {sim::Card::TapuLeleGX, sim::Card::Gladion,
                  sim::Card::Dawn, sim::Card::ProfessorBurnet,
                  sim::Card::Fire, sim::Card::RegidragoV};
  return state;
}

void same_turn_jit_profiles_share_the_route() {
  // Steven ends T2 after banking VSTAR + Grass + Mysterious Treasure. On T3,
  // Treasure can discard the held Dragon payload on the exact ready turn, which
  // is the payload timing shared by Strict JIT and MatchupFlex JIT.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, Item, search, discard, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and shared same-turn JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1816
  // Confirmed cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2765
  for (const sim::DciProfile profile : {sim::DciProfile::StrictJit,
                                        sim::DciProfile::MatchupFlexJit}) {
    std::mt19937_64 rng{2765};
    sim::DeckRecipe recipe = sim::pineco_recipe();
    sim::Engine engine{scenario(profile), recipe, rng};
    sim::EngineTestAccess::set_state(engine, exact_state());

    expect(sim::EngineTestAccess::public_preflight(engine),
           "A same-turn JIT profile rejected the issue-1816 route");
    expect(sim::EngineTestAccess::play_route(engine),
           "A same-turn JIT profile failed to play the issue-1816 route");
    expect(engine.state().turn_ended,
           "Steven's Resolve did not end the projected turn");
    expect(engine.state().active->fire == 1,
           "The route did not make the required Fire attachment");
    expect(contains(engine.state().hand, sim::Card::RegidragoVstar) &&
               contains(engine.state().hand, sim::Card::Grass) &&
               contains(engine.state().hand, sim::Card::MysteriousTreasure),
           "Steven did not bank the complete next-turn package");
    expect(contains(engine.state().deck, sim::Card::SecretBox),
           "The direct route consumed the preserved Secret Box axis");
  }
}

void no_discard_control_stays_outside_the_jit_route() {
  // NoDiscardControl has different earlier payload-banking semantics and should
  // continue through its ordinary policy rather than this same-turn-JIT fast path.
  // DCI/JIT profile contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2765
  std::mt19937_64 rng{2765};
  sim::DeckRecipe recipe = sim::pineco_recipe();
  sim::Engine engine{scenario(sim::DciProfile::NoDiscardControl), recipe, rng};
  sim::EngineTestAccess::set_state(engine, exact_state());
  expect(!sim::EngineTestAccess::public_preflight(engine),
         "NoDiscardControl entered the same-turn-JIT Pineco route");
}
}  // namespace

int main() {
  try {
    same_turn_jit_profiles_share_the_route();
    no_discard_control_stays_outside_the_jit_route();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
