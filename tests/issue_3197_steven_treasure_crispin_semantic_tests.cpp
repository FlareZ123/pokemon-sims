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
    engine.prizes_revealed_ = known;
  }

  static bool route_available(const Engine& engine) {
    return engine.issue_3197_steven_treasure_crispin_route_available();
  }

  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven();
  }

  static bool play_steven(Engine& engine) {
    return engine.play_steven();
  }

  static const State& state(const Engine& engine) {
    return engine.state_;
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn, 0, 1};
  state.hand = {
      sim::Card::StevensResolve,
      sim::Card::MysteriousTreasure,
      sim::Card::MegaDragonite,
      sim::Card::QuickBall,
  };
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Dragapult,
      sim::Card::BrilliantBlender,
      sim::Card::ProfessorBurnet,
      sim::Card::RegidragoV,
      sim::Card::TapuLeleGX,
  };
  state.prizes = {
      sim::Card::Arven,
      sim::Card::FieldBlower,
      sim::Card::Guzma,
      sim::Card::Klara,
      sim::Card::Lusamine,
      sim::Card::PathToPeak,
  };
  return state;
}

sim::Engine make_engine(const sim::DciProfile dci, const sim::LockMode locks,
                        const bool going_first, const int turn,
                        std::mt19937_64& rng, const bool known = true) {
  const sim::Scenario scenario{"issue-3197", dci, locks, going_first, turn + 1};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::EngineTestAccess::set_state(engine, route_state(turn), known);
  return engine;
}

void expect_semantic_route(const sim::DciProfile dci, const sim::LockMode locks,
                           const bool going_first, const int turn,
                           const std::uint64_t seed) {
  std::mt19937_64 rng{seed};
  sim::Engine engine = make_engine(dci, locks, going_first, turn, rng);

  // Steven ends this turn after searching any three cards. The held Mysterious
  // Treasure plus held Dragon already owns the next-turn payload channel, so the
  // connector-dominant targets are VSTAR, Crispin, and Grass. Crispin supplies
  // one Grass and keeps Fire searchable, while Treasure discards the held Dragon
  // on the same ready turn required by both JIT profiles.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Supporter, Item, evolution, search, discard, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // JIT and lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3197
  expect(sim::EngineTestAccess::route_available(engine),
         "Semantic Treasure-Crispin route was not admitted");
  expect(sim::EngineTestAccess::should_play_steven(engine),
         "Steven selector rejected the complete semantic route");
  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven failed to execute the complete semantic route");

  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(state.turn_ended && state.supporter_used,
         "Steven did not end the turn after consuming the Supporter action");
  expect(std::find(state.hand.begin(), state.hand.end(), sim::Card::RegidragoVstar) != state.hand.end() &&
             std::find(state.hand.begin(), state.hand.end(), sim::Card::Crispin) != state.hand.end() &&
             std::find(state.hand.begin(), state.hand.end(), sim::Card::Grass) != state.hand.end(),
         "Steven did not reserve VSTAR + Crispin + Grass");
  expect(std::find(state.hand.begin(), state.hand.end(), sim::Card::BrilliantBlender) == state.hand.end() &&
             std::find(state.hand.begin(), state.hand.end(), sim::Card::ProfessorBurnet) == state.hand.end(),
         "Steven searched a connector-dominated payload outlet");
  expect(std::find(state.deck.begin(), state.deck.end(), sim::Card::Fire) != state.deck.end() &&
             std::find(state.deck.begin(), state.deck.end(), sim::Card::Dragapult) != state.deck.end(),
         "Steven consumed Crispin's Fire or Treasure's surviving target");
}

void test_semantic_equivalents() {
  expect_semantic_route(sim::DciProfile::MatchupFlexJit,
                        sim::LockMode::None, false, 1, 319701);
  expect_semantic_route(sim::DciProfile::StrictJit,
                        sim::LockMode::FullRuleBoxAbility, false, 1, 319702);
  expect_semantic_route(sim::DciProfile::StrictJit,
                        sim::LockMode::None, true, 2, 319703);
}

void test_real_blockers_remain() {
  {
    std::mt19937_64 rng{319704};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 1, rng);
    // Going first on turn one cannot use a Supporter. The semantic route relies
    // on the actual Supporter rule rather than a permanent seat identity gate:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Official turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3197
    expect(!sim::EngineTestAccess::route_available(engine),
           "First-turn going-first illegally admitted Steven");
  }
  {
    std::mt19937_64 rng{319705};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::TurnTwoItem, false, 1, rng);
    // Mysterious Treasure is an Item and must remain blocked when the next turn
    // is the modeled Item-lock turn: https://api.pokemontcg.io/v2/cards/sm6-113
    // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    expect(!sim::EngineTestAccess::route_available(engine),
           "Next-turn Item lock illegally admitted the Treasure route");
  }
  {
    std::mt19937_64 rng{319706};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::FullSupporter, false, 1, rng);
    // Steven and Crispin are Supporters, so a Supporter lock is a real blocker:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sv7-133
    expect(!sim::EngineTestAccess::route_available(engine),
           "Supporter lock illegally admitted the Steven route");
  }
  {
    std::mt19937_64 rng{319707};
    sim::Engine engine = make_engine(sim::DciProfile::NoDiscardControl,
                                     sim::LockMode::None, false, 1, rng);
    expect(!sim::EngineTestAccess::route_available(engine),
           "NoDiscardControl illegally admitted the same-turn Treasure payload");
  }
  {
    std::mt19937_64 rng{319708};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, false, 1, rng, false);
    // K0 cannot preselect a package by looking through hidden deck identities.
    // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    expect(!sim::EngineTestAccess::route_available(engine),
           "K0 state illegally used hidden deck composition");
  }
  {
    std::mt19937_64 rng{319709};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, false, 1, rng);
    sim::State state = sim::EngineTestAccess::state(engine);
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Dragapult),
                     state.deck.end());
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::TapuLeleGX),
                     state.deck.end());
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::RegidragoV),
                     state.deck.end());
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    // After reserving the VSTAR, Treasure must still have a legal Psychic/Dragon
    // search target: https://api.pokemontcg.io/v2/cards/sm6-113
    expect(!sim::EngineTestAccess::route_available(engine),
           "Treasure route was admitted without a surviving search target");
  }
}
}  // namespace

int main() {
  try {
    test_semantic_equivalents();
    test_real_blockers_remain();
    std::cout << "Issue 3197 semantic Steven route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
