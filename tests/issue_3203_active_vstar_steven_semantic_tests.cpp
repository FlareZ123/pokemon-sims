#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }
  static bool route(const Engine& engine) {
    return engine.issue_3203_active_vstar_steven_crispin_treasure_route_available();
  }
  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven_issue1030_original();
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
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, turn - 1, 0, 1};
  state.hand = {
      sim::Card::StevensResolve,
      sim::Card::MysteriousTreasure,
      sim::Card::MegaDragonite,
  };
  state.deck = {
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Dragapult,
      sim::Card::RegidragoV,
  };
  return state;
}

sim::Engine make_engine(const sim::DciProfile dci, const sim::LockMode locks,
                        const bool going_first, const int turn,
                        const int max_turn, std::mt19937_64& rng,
                        const bool known = true) {
  const sim::Scenario scenario{"issue-3203", dci, locks, going_first, max_turn};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::EngineTestAccess::set_state(engine, route_state(turn), known);
  return engine;
}

void test_semantic_parity() {
  const std::tuple<sim::DciProfile, sim::LockMode, bool, int, std::uint64_t> cases[] = {
      {sim::DciProfile::StrictJit, sim::LockMode::None, true, 2, 320301ULL},
      {sim::DciProfile::MatchupFlexJit, sim::LockMode::None, true, 2, 320302ULL},
      {sim::DciProfile::StrictJit, sim::LockMode::None, false, 2, 320303ULL},
      {sim::DciProfile::StrictJit, sim::LockMode::None, false, 3, 320304ULL},
      {sim::DciProfile::StrictJit, sim::LockMode::FullRuleBoxAbility, true, 2, 320305ULL},
  };
  for (const auto& [dci, locks, going_first, turn, seed] : cases) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(dci, locks, going_first, turn, turn + 1, rng);
    // The Active VSTAR already has Fire. Steven reserves Crispin + Grass; next turn
    // Crispin and the normal attachment provide GG, while held Treasure discards the
    // held Dragon on that same ready turn. Seat and absolute turn do not alter those
    // physical actions once Supporter/Item legality and horizon are equivalent.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // JIT/lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3203
    expect(sim::EngineTestAccess::route(engine),
           "Equivalent active-VSTAR Steven route was rejected");
    expect(sim::EngineTestAccess::should_play_steven(engine),
           "Steven admission missed the semantic active-VSTAR route");
  }
}

void test_real_blockers_remain() {
  for (const auto [locks, seed] : {
           std::pair{sim::LockMode::TurnTwoItem, 320306ULL},
           std::pair{sim::LockMode::FullItem, 320307ULL},
           std::pair{sim::LockMode::FullCombined, 320308ULL},
           std::pair{sim::LockMode::FullSupporter, 320309ULL},
       }) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit, locks, true, 2, 3, rng);
    expect(!sim::EngineTestAccess::route(engine),
           "Current required Trainer lock illegally admitted route");
  }
  {
    std::mt19937_64 rng{320310};
    sim::Engine engine = make_engine(sim::DciProfile::NoDiscardControl,
                                     sim::LockMode::None, true, 2, 3, rng);
    expect(!sim::EngineTestAccess::route(engine),
           "NoDiscardControl was conflated with same-ready-turn JIT");
  }
  {
    std::mt19937_64 rng{320311};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 2, 2, rng);
    expect(!sim::EngineTestAccess::route(engine), "Expired horizon admitted route");
  }
  {
    std::mt19937_64 rng{320312};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 2, 3, rng, false);
    // K0 cannot inspect hidden deck identities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    expect(!sim::EngineTestAccess::route(engine), "K0 admitted K1 route");
  }
  {
    std::mt19937_64 rng{320313};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 2, 3, rng);
    sim::State state = route_state(2);
    state.manual_energy_used = true;
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(!sim::EngineTestAccess::route(engine), "Spent manual attachment admitted route");
  }
  {
    std::mt19937_64 rng{320314};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 2, 3, rng);
    sim::State state = route_state(2);
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::StevensResolve), state.hand.end());
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(!sim::EngineTestAccess::route(engine), "Missing Steven admitted route");
  }
  {
    std::mt19937_64 rng{320315};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 2, 3, rng);
    sim::State state = route_state(2);
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::MysteriousTreasure), state.hand.end());
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(!sim::EngineTestAccess::route(engine), "Missing Treasure admitted route");
  }
  {
    std::mt19937_64 rng{320316};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 2, 3, rng);
    sim::State state = route_state(2);
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite), state.hand.end());
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(!sim::EngineTestAccess::route(engine), "Missing held payload admitted route");
  }
  {
    std::mt19937_64 rng{320317};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 2, 3, rng);
    sim::State state = route_state(2);
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Crispin), state.deck.end());
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(!sim::EngineTestAccess::route(engine), "Missing Crispin admitted route");
  }
  {
    std::mt19937_64 rng{320318};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 2, 3, rng);
    sim::State state = route_state(2);
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Dragapult), state.deck.end());
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::RegidragoV), state.deck.end());
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    // Treasure requires a legal Psychic/Dragon target after its discard cost: https://api.pokemontcg.io/v2/cards/sm6-113
    expect(!sim::EngineTestAccess::route(engine), "Targetless Treasure admitted route");
  }
}
}  // namespace

int main() {
  try {
    test_semantic_parity();
    test_real_blockers_remain();
    std::cout << "Issue 3203 active-VSTAR semantic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
