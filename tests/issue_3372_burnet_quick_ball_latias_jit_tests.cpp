#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = false;
  }

  static bool route_available(const Engine& engine) {
    return engine.issue_2343_burnet_quick_ball_latias_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State complete_route_state() {
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

bool available(const sim::DciProfile dci, sim::State state) {
  sim::Scenario scenario{"issue-3372", dci, sim::LockMode::None, false, 4};
  std::mt19937_64 rng{3372};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::route_available(engine);
}

void test_same_ready_turn_jit_profiles_share_route() {
  // Professor Burnet places the Dragon payload into discard on the ready turn,
  // while Quick Ball searches Latias ex and Skyliner supplies the Basic Active's
  // retreat axis. Repository policy assigns this same-ready-turn payload contract
  // to both StrictJit and MatchupFlexJit, so the route cannot depend on profile name.
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Same-ready-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3372
  expect(available(sim::DciProfile::StrictJit, complete_route_state()),
         "StrictJit rejected the complete Burnet-Quick Ball-Latias route.");
  expect(available(sim::DciProfile::MatchupFlexJit, complete_route_state()),
         "MatchupFlexJit rejected the established Burnet-Quick Ball-Latias route.");
  expect(!available(sim::DciProfile::NoDiscardControl, complete_route_state()),
         "NoDiscardControl entered a same-ready-turn JIT-only route.");
}

void test_required_route_resources_remain_required() {
  // The profile generalization preserves the physical route gates from #2343:
  // Burnet must be available, Latias ex must be searchable, and a permitted Dragon
  // payload must remain in the known deck for Burnet to discard this turn.
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3372
  sim::State missing_burnet = complete_route_state();
  missing_burnet.deck.erase(
      std::remove(missing_burnet.deck.begin(), missing_burnet.deck.end(),
                  sim::Card::ProfessorBurnet),
      missing_burnet.deck.end());
  expect(!available(sim::DciProfile::StrictJit, std::move(missing_burnet)),
         "StrictJit route was admitted without Professor Burnet.");

  sim::State missing_latias = complete_route_state();
  missing_latias.deck.erase(
      std::remove(missing_latias.deck.begin(), missing_latias.deck.end(),
                  sim::Card::LatiasEx),
      missing_latias.deck.end());
  expect(!available(sim::DciProfile::StrictJit, std::move(missing_latias)),
         "StrictJit route was admitted without Latias ex.");

  sim::State missing_payload = complete_route_state();
  missing_payload.deck.erase(
      std::remove_if(missing_payload.deck.begin(), missing_payload.deck.end(),
                     sim::is_payload),
      missing_payload.deck.end());
  expect(!available(sim::DciProfile::StrictJit, std::move(missing_payload)),
         "StrictJit route was admitted without a Burnet payload.");
}
}  // namespace

int main() {
  test_same_ready_turn_jit_profiles_share_route();
  test_required_route_resources_remain_required();
  return 0;
}
