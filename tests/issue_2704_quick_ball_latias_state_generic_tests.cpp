#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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

void remove_card(std::vector<sim::Card>& zone, const sim::Card card) {
  const auto it = std::find(zone.begin(), zone.end(), card);
  if (it != zone.end()) zone.erase(it);
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
  // cost. The route is governed by projected state instead of one seed's literal
  // T3/T4 witness labels:
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
  // These controls preserve the physical K1, lock, Bench, Ability, Retreat,
  // evolution, and resource boundaries while removing only witness-specific timing:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Item, Ability, Bench, Retreat, evolution, and turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K1 and JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
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

  const sim::Scenario strict_t3{"issue-2704-controls", sim::DciProfile::StrictJit,
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

  const sim::Scenario item_lock{"issue-2704-item-lock", sim::DciProfile::StrictJit,
                                sim::LockMode::FullItem, true, 3};
  std::mt19937_64 item_lock_rng{270408};
  sim::Engine item_locked =
      make_engine(item_lock, item_lock_rng, route_state(2, 2));
  expect(!sim::EngineTestAccess::latias_route(item_locked),
         "Quick Ball must remain unavailable under Item lock.");

  const sim::Scenario ability_lock{"issue-2704-ability-lock",
                                   sim::DciProfile::StrictJit,
                                   sim::LockMode::FullRuleBoxAbility, true, 3};
  std::mt19937_64 ability_lock_rng{270409};
  sim::Engine ability_locked =
      make_engine(ability_lock, ability_lock_rng, route_state(2, 2));
  expect(!sim::EngineTestAccess::latias_route(ability_locked),
         "The route must remain unavailable when Skyliner is locked.");

  sim::State full_bench_state = route_state(2, 2);
  full_bench_state.bench.push_back(sim::Pokemon{sim::Card::CrobatV, 1});
  full_bench_state.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 1});
  full_bench_state.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  std::mt19937_64 full_bench_rng{270410};
  sim::Engine full_bench =
      make_engine(strict_t3, full_bench_rng, std::move(full_bench_state));
  expect(!sim::EngineTestAccess::latias_route(full_bench),
         "Latias cannot enter a full Bench.");

  sim::State retreat_spent_state = route_state(2, 2);
  retreat_spent_state.retreat_used = true;
  std::mt19937_64 retreat_rng{270411};
  sim::Engine retreat_spent =
      make_engine(strict_t3, retreat_rng, std::move(retreat_spent_state));
  expect(!sim::EngineTestAccess::latias_route(retreat_spent),
         "The Latias replacement route requires the remaining Retreat action.");

  sim::State no_vstar_state = route_state(2, 2);
  remove_card(no_vstar_state.hand, sim::Card::RegidragoVstar);
  std::mt19937_64 no_vstar_rng{270412};
  sim::Engine no_vstar =
      make_engine(strict_t3, no_vstar_rng, std::move(no_vstar_state));
  expect(!sim::EngineTestAccess::latias_route(no_vstar),
         "The projected route requires the held Regidrago VSTAR.");

  sim::State no_energy_state = route_state(2, 2);
  remove_card(no_energy_state.hand, sim::Card::Fire);
  std::mt19937_64 no_energy_rng{270413};
  sim::Engine no_energy =
      make_engine(strict_t3, no_energy_rng, std::move(no_energy_state));
  expect(!sim::EngineTestAccess::latias_route(no_energy),
         "The projected route requires a held completing Basic Energy.");

  sim::State no_burnet_state = route_state(2, 2);
  remove_card(no_burnet_state.hand, sim::Card::ProfessorBurnet);
  std::mt19937_64 no_burnet_rng{270414};
  sim::Engine no_burnet =
      make_engine(strict_t3, no_burnet_rng, std::move(no_burnet_state));
  expect(!sim::EngineTestAccess::latias_route(no_burnet),
         "The projected payload line requires held Professor Burnet.");

  sim::State no_latias_state = route_state(2, 2);
  remove_card(no_latias_state.deck, sim::Card::LatiasEx);
  std::mt19937_64 no_latias_rng{270415};
  sim::Engine no_latias =
      make_engine(strict_t3, no_latias_rng, std::move(no_latias_state));
  expect(!sim::EngineTestAccess::latias_route(no_latias),
         "Quick Ball requires Latias ex to remain in the inspected deck.");

  sim::State no_payload_state = route_state(2, 2);
  remove_card(no_payload_state.deck, sim::Card::MegaDragonite);
  remove_card(no_payload_state.deck, sim::Card::Dragapult);
  std::mt19937_64 no_payload_rng{270416};
  sim::Engine no_payload =
      make_engine(strict_t3, no_payload_rng, std::move(no_payload_state));
  expect(!sim::EngineTestAccess::latias_route(no_payload),
         "Professor Burnet requires a permitted Dragon payload in the inspected deck.");
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
