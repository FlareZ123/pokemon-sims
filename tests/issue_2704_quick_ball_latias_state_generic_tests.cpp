#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }

  static bool latias_route(const Engine& engine) {
    return engine.quick_ball_seed23_latias_route_ready();
  }

  static std::optional<Card> tate_cost(const Engine& engine) {
    return engine.quick_ball_latias_replaced_tate_cost();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn, const int entered_turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1},
                 sim::Pokemon{sim::Card::RegidragoV, entered_turn, 2, 0,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::QuickBall, sim::Card::TateLiza,
                sim::Card::RegidragoVstar, sim::Card::ProfessorBurnet,
                sim::Card::Fire};
  state.deck = {sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::RegidragoV};
  state.discard = {sim::Card::Crispin, sim::Card::StevensResolve};
  state.vstar_power_used = true;
  state.supporter_used = true;
  state.manual_energy_used = true;
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void test_projected_next_turn_route() {
  // Quick Ball can search Basic Latias ex after Tate & Liza becomes the discard
  // cost, then Skyliner can remove the Basic Active's Retreat Cost. A Regidrago V
  // already in play may evolve on the following turn, Professor Burnet may supply
  // the JIT Dragon payload, and the held Basic Energy completes the modeled attack
  // cost. The route is therefore governed by projected state instead of one seed's
  // literal T3/T4 witness labels:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced evolution / turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2704
  const sim::Scenario strict_t4{"issue-2704-strict-t4", sim::DciProfile::StrictJit,
                                sim::LockMode::None, true, 4};
  std::mt19937_64 original_rng{270400};
  sim::Engine original = make_engine(strict_t4, original_rng, route_state(3, 3));
  expect(sim::EngineTestAccess::latias_route(original),
         "The historical T3-to-T4 route must remain live.");
  expect(sim::EngineTestAccess::tate_cost(original) == sim::Card::TateLiza,
         "Latias must still replace Tate & Liza on the historical route.");

  const sim::Scenario strict_t3{"issue-2704-strict-t3", sim::DciProfile::StrictJit,
                                sim::LockMode::None, true, 3};
  std::mt19937_64 earlier_rng{270401};
  sim::Engine earlier = make_engine(strict_t3, earlier_rng, route_state(2, 2));
  expect(sim::EngineTestAccess::latias_route(earlier),
         "The same legal physical route must work from T2 toward T3.");

  const sim::Scenario flex_t3{"issue-2704-flex-t3", sim::DciProfile::MatchupFlexJit,
                              sim::LockMode::None, true, 3};
  std::mt19937_64 flex_rng{270402};
  sim::Engine flex = make_engine(flex_t3, flex_rng, route_state(2, 2));
  expect(sim::EngineTestAccess::latias_route(flex),
         "Matchup-flex JIT must admit the same next-turn route.");

  std::mt19937_64 older_rng{270403};
  sim::Engine older = make_engine(strict_t4, older_rng, route_state(3, 2));
  expect(sim::EngineTestAccess::latias_route(older),
         "A Regidrago V already in play before this turn remains evolvable next turn.");
}

void test_projection_controls() {
  // These controls keep the generalized route bounded by the same physical and
  // policy requirements. The next-turn horizon and evolution timing are direct
  // legality predicates rather than seed labels:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced evolution / turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2704
  const sim::Scenario short_horizon{"issue-2704-horizon", sim::DciProfile::StrictJit,
                                    sim::LockMode::None, true, 2};
  std::mt19937_64 horizon_rng{270404};
  sim::Engine horizon = make_engine(short_horizon, horizon_rng, route_state(2, 2));
  expect(!sim::EngineTestAccess::latias_route(horizon),
         "A route outside the configured next-turn horizon must stay blocked.");

  const sim::Scenario no_control{"issue-2704-no-control",
                                 sim::DciProfile::NoDiscardControl,
                                 sim::LockMode::None, true, 3};
  std::mt19937_64 no_control_rng{270405};
  sim::Engine no_control_engine =
      make_engine(no_control, no_control_rng, route_state(2, 2));
  expect(!sim::EngineTestAccess::latias_route(no_control_engine),
         "The specialized discard route must remain limited to JIT profiles.");

  const sim::Scenario strict_t3{"issue-2704-invalid-evolution",
                                sim::DciProfile::StrictJit,
                                sim::LockMode::None, true, 3};
  std::mt19937_64 evolution_rng{270406};
  sim::Engine invalid_evolution =
      make_engine(strict_t3, evolution_rng, route_state(2, 3));
  expect(!sim::EngineTestAccess::latias_route(invalid_evolution),
         "A Regidrago entering on the projected ready turn cannot evolve that turn.");

  std::mt19937_64 k0_rng{270407};
  sim::Engine k0 = make_engine(strict_t3, k0_rng, route_state(2, 2), false);
  expect(!sim::EngineTestAccess::latias_route(k0),
         "The route must retain its K1 requirement.");
}
}  // namespace

int main() {
  try {
    test_projected_next_turn_route();
    test_projection_controls();
    std::cout << "Issue 2704 Quick Ball Latias state-generic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
