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
  static bool route(const Engine& engine) {
    return engine.late_steven_vstar_vessel_route_available();
  }
  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven_issue1030_original();
  }
  static Card choose_supporter(Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn, const bool steven_in_hand = true) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 2, 1};
  state.hand = {sim::Card::EarthenVessel, sim::Card::MegaDragonite};
  if (steven_in_hand) state.hand.push_back(sim::Card::StevensResolve);
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::StevensResolve,
      sim::Card::Crispin,
      sim::Card::TapuLeleGX,
  };
  return state;
}

sim::Engine make_engine(const sim::DciProfile dci, const sim::LockMode locks,
                        const int turn, const int max_turn, std::mt19937_64& rng,
                        const bool known = true, const bool steven_in_hand = true) {
  const sim::Scenario scenario{"issue-3202", dci, locks, false, max_turn};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::EngineTestAccess::set_state(engine, route_state(turn, steven_in_hand), known);
  return engine;
}

void test_semantic_equivalents() {
  for (const auto [dci, locks, seed] : {
           std::tuple{sim::DciProfile::StrictJit, sim::LockMode::None, 320201ULL},
           std::tuple{sim::DciProfile::MatchupFlexJit, sim::LockMode::None, 320202ULL},
           std::tuple{sim::DciProfile::StrictJit, sim::LockMode::FullRuleBoxAbility, 320203ULL},
       }) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(dci, locks, 2, 3, rng);
    // Steven reserves the missing VSTAR while held Vessel + Dragon already own the
    // next-turn payload channel. Vessel's printed discard happens on that ready turn,
    // which satisfies both same-ready-turn JIT profiles. Path-style Rule Box Ability
    // lock does not stop these Trainer/evolution actions.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // JIT/lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3202
    expect(sim::EngineTestAccess::route(engine),
           "Semantic Steven-Vessel route was rejected");
    expect(sim::EngineTestAccess::should_play_steven(engine),
           "Steven admission did not consume the semantic route");
  }
}

void test_wonder_tag_selector_uses_semantic_route() {
  std::mt19937_64 rng{320204};
  sim::Engine engine = make_engine(sim::DciProfile::MatchupFlexJit,
                                   sim::LockMode::FullRuleBoxAbility,
                                   2, 3, rng, true, false);
  // Wonder Tag itself would be suppressed by Rule Box lock in a full game, yet this
  // selector is also the post-search policy surface. Given legal inspected access to
  // Steven, it must recognize Steven as the complete VSTAR/Vessel connector.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(sim::EngineTestAccess::choose_supporter(engine) == sim::Card::StevensResolve,
         "Post-search Supporter selector missed semantic Steven-Vessel route");
}

void test_real_blockers_remain() {
  for (const auto [locks, seed] : {
           std::pair{sim::LockMode::TurnTwoItem, 320205ULL},
           std::pair{sim::LockMode::FullItem, 320206ULL},
           std::pair{sim::LockMode::FullCombined, 320207ULL},
           std::pair{sim::LockMode::FullSupporter, 320208ULL},
       }) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit, locks, 2, 3, rng);
    expect(!sim::EngineTestAccess::route(engine),
           "A required Trainer lock illegally admitted the route");
  }
  {
    std::mt19937_64 rng{320209};
    sim::Engine engine = make_engine(sim::DciProfile::NoDiscardControl,
                                     sim::LockMode::None, 2, 3, rng);
    expect(!sim::EngineTestAccess::route(engine),
           "NoDiscardControl was conflated with same-ready-turn JIT");
  }
  {
    std::mt19937_64 rng{320210};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, 2, 2, rng);
    expect(!sim::EngineTestAccess::route(engine),
           "Expired next-turn horizon admitted Steven");
  }
  {
    std::mt19937_64 rng{320211};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, 2, 3, rng, false);
    // K0 may not inspect hidden deck identities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    expect(!sim::EngineTestAccess::route(engine), "K0 admitted the K1 route");
  }
  {
    std::mt19937_64 rng{320212};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, 2, 3, rng);
    sim::State state = route_state(2);
    state.active->entered_turn = 2;
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    // A Basic played this turn cannot evolve this turn: https://www.pokemon.com/us/pokemon-tcg/rules
    expect(!sim::EngineTestAccess::route(engine), "Same-turn Basic admitted deferred evolution route");
  }
  {
    std::mt19937_64 rng{320213};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, 2, 3, rng);
    sim::State state = route_state(2);
    state.active->grass = 1;
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(!sim::EngineTestAccess::route(engine), "Incomplete Apex Energy admitted route");
  }
  {
    std::mt19937_64 rng{320214};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, 2, 3, rng);
    sim::State state = route_state(2);
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite), state.hand.end());
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(!sim::EngineTestAccess::route(engine), "Missing held payload admitted route");
  }
  {
    std::mt19937_64 rng{320215};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, 2, 3, rng);
    sim::State state = route_state(2);
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::EarthenVessel), state.hand.end());
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::EarthenVessel), state.deck.end());
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(!sim::EngineTestAccess::route(engine), "Missing Vessel admitted route");
  }
  {
    std::mt19937_64 rng{320216};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, 2, 3, rng);
    sim::State state = route_state(2);
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Grass), state.deck.end());
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Fire), state.deck.end());
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(!sim::EngineTestAccess::route(engine), "Targetless Vessel search admitted route");
  }
}
}  // namespace

int main() {
  try {
    test_semantic_equivalents();
    test_wonder_tag_selector_uses_semantic_route();
    test_real_blockers_remain();
    std::cout << "Issue 3202 semantic Steven-Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
