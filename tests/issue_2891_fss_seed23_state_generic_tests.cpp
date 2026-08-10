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

  static bool fss_grass_route(const Engine& engine) {
    return engine.fss_should_take_grass_for_seed23_latias_burnet_route();
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
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, entered_turn, 0, 0,
                   sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::Crispin, sim::Card::QuickBall,
                sim::Card::TateLiza, sim::Card::ProfessorBurnet,
                sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Dragapult};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void test_projected_ready_turn_and_jit_profiles() {
  // Star Alchemy may search Grass; Crispin can search differently typed Basic
  // Energy and attach one; Quick Ball can search Basic Latias ex; Skyliner makes
  // Basic Pokemon in play have no Retreat Cost; Professor Burnet can put the
  // ready-turn Dragon payload into discard; Regidrago VSTAR's Apex Dragon costs GGF.
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced evolution, Item, Supporter, attachment, and Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Shared JIT timing and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2891
  const sim::Scenario strict_t4{"issue-2891-strict-t4", sim::DciProfile::StrictJit,
                                sim::LockMode::None, true, 4};
  std::mt19937_64 t4_rng{289100};
  sim::Engine t4 = make_engine(strict_t4, t4_rng, route_state(3, 3));
  expect(sim::EngineTestAccess::fss_grass_route(t4),
         "The historical T3-to-T4 Forest Seal Stone route must remain live.");

  const sim::Scenario strict_t3{"issue-2891-strict-t3", sim::DciProfile::StrictJit,
                                sim::LockMode::None, true, 3};
  std::mt19937_64 t3_rng{289101};
  sim::Engine t3 = make_engine(strict_t3, t3_rng, route_state(2, 2));
  expect(sim::EngineTestAccess::fss_grass_route(t3),
         "The same physical route must work from T2 toward T3.");

  const sim::Scenario flex_t3{"issue-2891-flex-t3", sim::DciProfile::MatchupFlexJit,
                              sim::LockMode::None, true, 3};
  std::mt19937_64 flex_rng{289102};
  sim::Engine flex = make_engine(flex_t3, flex_rng, route_state(2, 2));
  expect(sim::EngineTestAccess::fss_grass_route(flex),
         "Matchup-flex JIT must admit the same projected ready-turn route.");

  // TurnTwoItem is scheduled from T2 onward. On T1 going second, Quick Ball and
  // Forest Seal Stone remain legal now, while the T2 continuation uses evolution,
  // a manual Energy attachment, and Professor Burnet rather than another Item.
  // Current-action lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
  // Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2891
  const sim::Scenario scheduled_t2_lock{
      "issue-2891-scheduled-t2-item-lock", sim::DciProfile::StrictJit,
      sim::LockMode::TurnTwoItem, false, 2};
  std::mt19937_64 scheduled_rng{289103};
  sim::Engine scheduled =
      make_engine(scheduled_t2_lock, scheduled_rng, route_state(1, 1));
  expect(sim::EngineTestAccess::fss_grass_route(scheduled),
         "A future T2 Item lock must not suppress legal T1 route actions.");
}

void test_physical_and_timing_controls() {
  // These controls retain the route's real K1, current-action legality, Bench,
  // evolution, resource, and horizon boundaries while removing witness labels.
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K1 and JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2891
  const sim::Scenario strict_t3{"issue-2891-controls", sim::DciProfile::StrictJit,
                                sim::LockMode::None, true, 3};

  std::mt19937_64 k0_rng{289104};
  sim::Engine k0 = make_engine(strict_t3, k0_rng, route_state(2, 2), false);
  expect(!sim::EngineTestAccess::fss_grass_route(k0),
         "The Forest Seal Stone route must retain its K1 requirement.");

  const sim::Scenario current_item_lock{
      "issue-2891-current-item-lock", sim::DciProfile::StrictJit,
      sim::LockMode::TurnTwoItem, true, 3};
  std::mt19937_64 item_rng{289105};
  sim::Engine item_locked =
      make_engine(current_item_lock, item_rng, route_state(2, 2));
  expect(!sim::EngineTestAccess::fss_grass_route(item_locked),
         "Current T2 Item lock must still block Quick Ball.");

  const sim::Scenario ability_lock{
      "issue-2891-ability-lock", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, true, 3};
  std::mt19937_64 ability_rng{289106};
  sim::Engine ability_locked =
      make_engine(ability_lock, ability_rng, route_state(2, 2));
  expect(!sim::EngineTestAccess::fss_grass_route(ability_locked),
         "Rule Box Ability lock must still block Latias ex Skyliner.");

  const sim::Scenario supporter_lock{
      "issue-2891-supporter-lock", sim::DciProfile::StrictJit,
      sim::LockMode::FullSupporter, true, 3};
  std::mt19937_64 supporter_lock_rng{289107};
  sim::Engine supporter_locked =
      make_engine(supporter_lock, supporter_lock_rng, route_state(2, 2));
  expect(!sim::EngineTestAccess::fss_grass_route(supporter_locked),
         "Current Supporter lock must still block Crispin.");

  sim::State full_bench_state = route_state(2, 2);
  full_bench_state.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  full_bench_state.bench.push_back(sim::Pokemon{sim::Card::CrobatV, 1});
  full_bench_state.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 1});
  full_bench_state.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  std::mt19937_64 bench_rng{289108};
  sim::Engine full_bench =
      make_engine(strict_t3, bench_rng, std::move(full_bench_state));
  expect(!sim::EngineTestAccess::fss_grass_route(full_bench),
         "Latias ex cannot enter a full Bench.");

  sim::State supporter_spent_state = route_state(2, 2);
  supporter_spent_state.supporter_used = true;
  std::mt19937_64 supporter_rng{289109};
  sim::Engine supporter_spent =
      make_engine(strict_t3, supporter_rng, std::move(supporter_spent_state));
  expect(!sim::EngineTestAccess::fss_grass_route(supporter_spent),
         "The route requires the current Supporter action for Crispin.");

  sim::State manual_spent_state = route_state(2, 2);
  manual_spent_state.manual_energy_used = true;
  std::mt19937_64 manual_rng{289110};
  sim::Engine manual_spent =
      make_engine(strict_t3, manual_rng, std::move(manual_spent_state));
  expect(!sim::EngineTestAccess::fss_grass_route(manual_spent),
         "The route requires the current manual Energy attachment.");

  sim::State retreat_spent_state = route_state(2, 2);
  retreat_spent_state.retreat_used = true;
  std::mt19937_64 retreat_rng{289111};
  sim::Engine retreat_spent =
      make_engine(strict_t3, retreat_rng, std::move(retreat_spent_state));
  expect(!sim::EngineTestAccess::fss_grass_route(retreat_spent),
         "The Latias replacement route requires the remaining Retreat action.");

  const sim::Scenario short_horizon{
      "issue-2891-short-horizon", sim::DciProfile::StrictJit,
      sim::LockMode::None, true, 2};
  std::mt19937_64 horizon_rng{289112};
  sim::Engine horizon =
      make_engine(short_horizon, horizon_rng, route_state(2, 2));
  expect(!sim::EngineTestAccess::fss_grass_route(horizon),
         "A projected route outside the configured horizon must remain blocked.");

  std::mt19937_64 evolution_rng{289113};
  sim::Engine invalid_evolution =
      make_engine(strict_t3, evolution_rng, route_state(2, 3));
  expect(!sim::EngineTestAccess::fss_grass_route(invalid_evolution),
         "A Regidrago V entering on the projected ready turn cannot evolve then.");

  const sim::Scenario no_control{
      "issue-2891-no-control", sim::DciProfile::NoDiscardControl,
      sim::LockMode::None, true, 3};
  std::mt19937_64 no_control_rng{289114};
  sim::Engine no_control_engine =
      make_engine(no_control, no_control_rng, route_state(2, 2));
  expect(!sim::EngineTestAccess::fss_grass_route(no_control_engine),
         "The specialized route must remain confined to ready-turn JIT profiles.");
}

void test_required_route_resources() {
  // The selector must retain every physical connector from the confirmed route.
  // Card texts: https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sv7-133 https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/sv8-76 https://api.pokemontcg.io/v2/cards/sm7-148 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2891
  const sim::Scenario strict_t3{"issue-2891-resources", sim::DciProfile::StrictJit,
                                sim::LockMode::None, true, 3};

  const auto expect_missing_hand_card = [&](const sim::Card card,
                                            const std::uint64_t seed,
                                            const char* message) {
    sim::State state = route_state(2, 2);
    remove_card(state.hand, card);
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(strict_t3, rng, std::move(state));
    expect(!sim::EngineTestAccess::fss_grass_route(engine), message);
  };
  expect_missing_hand_card(sim::Card::Crispin, 289115,
                           "The route requires held Crispin.");
  expect_missing_hand_card(sim::Card::QuickBall, 289116,
                           "The route requires held Quick Ball.");
  expect_missing_hand_card(sim::Card::TateLiza, 289117,
                           "The route requires route-replaced Tate & Liza.");
  expect_missing_hand_card(sim::Card::ProfessorBurnet, 289118,
                           "The route requires held Professor Burnet.");
  expect_missing_hand_card(sim::Card::RegidragoVstar, 289119,
                           "The projected route requires held Regidrago VSTAR.");

  sim::State no_latias_state = route_state(2, 2);
  remove_card(no_latias_state.deck, sim::Card::LatiasEx);
  std::mt19937_64 latias_rng{289120};
  sim::Engine no_latias =
      make_engine(strict_t3, latias_rng, std::move(no_latias_state));
  expect(!sim::EngineTestAccess::fss_grass_route(no_latias),
         "Quick Ball requires Latias ex in the inspected deck.");

  sim::State one_grass_state = route_state(2, 2);
  remove_card(one_grass_state.deck, sim::Card::Grass);
  std::mt19937_64 grass_rng{289121};
  sim::Engine one_grass =
      make_engine(strict_t3, grass_rng, std::move(one_grass_state));
  expect(!sim::EngineTestAccess::fss_grass_route(one_grass),
         "The route requires two deck Grass before Star Alchemy resolves.");

  sim::State no_fire_state = route_state(2, 2);
  remove_card(no_fire_state.deck, sim::Card::Fire);
  std::mt19937_64 fire_rng{289122};
  sim::Engine no_fire =
      make_engine(strict_t3, fire_rng, std::move(no_fire_state));
  expect(!sim::EngineTestAccess::fss_grass_route(no_fire),
         "Crispin requires a differently typed Fire target for this route.");

  sim::State no_payload_state = route_state(2, 2);
  remove_card(no_payload_state.deck, sim::Card::MegaDragonite);
  remove_card(no_payload_state.deck, sim::Card::Dragapult);
  std::mt19937_64 payload_rng{289123};
  sim::Engine no_payload =
      make_engine(strict_t3, payload_rng, std::move(no_payload_state));
  expect(!sim::EngineTestAccess::fss_grass_route(no_payload),
         "Professor Burnet requires a permitted Dragon payload in the inspected deck.");
}
}  // namespace

int main() {
  try {
    test_projected_ready_turn_and_jit_profiles();
    test_physical_and_timing_controls();
    test_required_route_resources();
    std::cout << "Issue 2891 FSS seed-23 state-generic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
