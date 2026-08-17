#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace sim {
struct EngineTestAccess_issue_3203 {
  static void set_state(Engine& engine, State state, const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route(const Engine& engine) {
    return engine.issue_3203_active_vstar_steven_crispin_treasure_route_available();
  }
  static bool package(const Engine& engine) {
    return engine.late_steven_package_available();
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
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
  sim::Engine engine(
      sim::Scenario{"issue-3203", dci, locks, going_first, max_turn},
      sim::baseline_recipe(), rng);
  sim::EngineTestAccess_issue_3203::set_state(
      engine, route_state(turn), deck_seen, prizes_revealed);
  return engine;
}

void test_semantic_parity_and_selector_integration() {
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
    // Steven reserves Crispin + Grass. On the following turn Crispin and the normal
    // attachment provide GG while held Treasure discards the Dragon on that ready turn.
    // Rule Box Ability suppression does not block this Trainer/attachment packet.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // JIT/lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3203
    expect(sim::EngineTestAccess_issue_3203::route(engine),
           "Equivalent active-VSTAR Steven route was rejected");
    expect(sim::EngineTestAccess_issue_3203::package(engine),
           "Central late-Steven selector missed the semantic route");
  }
}

void test_k1_provenance() {
  // A complete Prize inspection establishes the same fixed-list K1 as a legal deck
  // search, while true K0 cannot consume hidden deck identities.
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3203
  {
    std::mt19937_64 rng{320307};
    auto engine = make_engine(sim::DciProfile::StrictJit, sim::LockMode::None,
                              false, 3, 4, rng, false, true);
    expect(sim::EngineTestAccess_issue_3203::route(engine),
           "Prize-inspection K1 was rejected");
  }
  {
    std::mt19937_64 rng{320308};
    auto engine = make_engine(sim::DciProfile::StrictJit, sim::LockMode::None,
                              false, 3, 4, rng, false, false);
    expect(!sim::EngineTestAccess_issue_3203::route(engine),
           "True K0 entered the K1 route");
  }
}

void test_real_blockers_remain() {
  for (const auto [locks, seed] : {
           std::pair{sim::LockMode::TurnTwoItem, 320309ULL},
           std::pair{sim::LockMode::FullItem, 320310ULL},
           std::pair{sim::LockMode::FullCombined, 320311ULL},
           std::pair{sim::LockMode::FullSupporter, 320312ULL},
       }) {
    std::mt19937_64 rng{seed};
    auto engine = make_engine(sim::DciProfile::StrictJit, locks, true, 2, 3, rng);
    // TurnTwoItem begins on T2 and remains active thereafter, so the projected T3
    // Mysterious Treasure is illegal just like Treasure under persistent/full locks.
    // Lock specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    expect(!sim::EngineTestAccess_issue_3203::route(engine),
           "Required Trainer lock illegally admitted route");
  }
  {
    std::mt19937_64 rng{320313};
    auto engine = make_engine(sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                              true, 2, 3, rng);
    expect(!sim::EngineTestAccess_issue_3203::route(engine),
           "NoDiscardControl was conflated with same-ready-turn JIT");
  }
  {
    std::mt19937_64 rng{320314};
    auto engine = make_engine(sim::DciProfile::StrictJit, sim::LockMode::None,
                              true, 2, 2, rng);
    expect(!sim::EngineTestAccess_issue_3203::route(engine),
           "Expired horizon admitted route");
  }
  {
    std::mt19937_64 rng{320315};
    auto engine = make_engine(sim::DciProfile::StrictJit, sim::LockMode::None,
                              true, 2, 3, rng);
    auto state = route_state(2);
    state.manual_energy_used = true;
    sim::EngineTestAccess_issue_3203::set_state(engine, std::move(state));
    // The manual attachment is once per turn. Steven's Resolve ends this turn, and
    // begin_turn() resets the attachment marker before the route's required normal
    // attachment on the following turn, so spending today's attachment is legal.
    // Advanced turn/attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Canonical turn reset: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Confirmed cross-turn correction: https://github.com/FlareZ123/pokemon-sims/issues/4130
    expect(sim::EngineTestAccess_issue_3203::route(engine),
           "Current-turn attachment incorrectly blocked the following-turn route");
  }
  {
    std::mt19937_64 rng{320316};
    auto engine = make_engine(sim::DciProfile::StrictJit, sim::LockMode::None,
                              true, 2, 3, rng);
    auto state = route_state(2);
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::MysteriousTreasure), state.hand.end());
    sim::EngineTestAccess_issue_3203::set_state(engine, std::move(state));
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    expect(!sim::EngineTestAccess_issue_3203::route(engine),
           "Missing Treasure admitted route");
  }
  {
    std::mt19937_64 rng{320317};
    auto engine = make_engine(sim::DciProfile::StrictJit, sim::LockMode::None,
                              true, 2, 3, rng);
    auto state = route_state(2);
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::Crispin), state.deck.end());
    sim::EngineTestAccess_issue_3203::set_state(engine, std::move(state));
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    expect(!sim::EngineTestAccess_issue_3203::route(engine),
           "Missing Crispin admitted route");
  }
}
}  // namespace

int main() {
  try {
    test_semantic_parity_and_selector_integration();
    test_k1_provenance();
    test_real_blockers_remain();
    std::cout << "Issue 3203 active-VSTAR semantic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
