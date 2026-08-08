#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

// Current-main reclaim validation: https://github.com/FlareZ123/pokemon-sims/issues/2315
namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static bool use_legacy_star(Engine& engine) {
    return engine.use_legacy_star();
  }

  static void choose_supporter(Engine& engine) {
    engine.choose_supporter();
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

sim::State seed21_post_evolution_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None}};
  state.hand = {
      sim::Card::MegaDragonite, sim::Card::Guzma,
      sim::Card::GoodraVstar, sim::Card::Dragapult,
      sim::Card::FieldBlower, sim::Card::ErikasInvitation,
      sim::Card::Fire, sim::Card::MegaDragonite};
  state.discard = {sim::Card::StevensResolve, sim::Card::Crispin};
  state.deck = {
      sim::Card::Grass, sim::Card::Grass, sim::Card::Appletun,
      sim::Card::RegidragoV, sim::Card::MysteriousTreasure,
      sim::Card::QuickBall, sim::Card::LatiasEx,
      sim::Card::TateLiza, sim::Card::Grass, sim::Card::HisuianHeavyBall,
      sim::Card::Grass, sim::Card::Gladion, sim::Card::Channeler,
      sim::Card::EarthenVessel};
  return state;
}

sim::State run_case(sim::State state, const std::uint64_t seed) {
  const sim::Scenario scenario{"issue-2315", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng, &trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(sim::EngineTestAccess::use_legacy_star(engine),
         "Legacy Star did not resolve in the issue-2315 fixture.");
  return sim::EngineTestAccess::state(engine);
}

void test_tate_replaces_redundant_energy_recovery() {
  // After Legacy Star reveals Tate & Liza, Grass, and Earthen Vessel, Vessel can
  // search one of the two remaining Grass Energy and discard the held Dragon payload.
  // Tate therefore completes the independent Active-position axis and has greater
  // discrete recovery value than the directly recovered Grass:
  // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Tate & Liza switch mode: https://api.pokemontcg.io/v2/cards/sm7-148
  // Mega Dragonite ex payload: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Dialga-GX Basic Active: https://api.pokemontcg.io/v2/cards/sm5-100
  // Official Supporter, Item, discard, search, attachment, and switch procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 / DCI / earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Existing Vessel bridge and confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/417 https://github.com/FlareZ123/pokemon-sims/issues/2315
  const sim::State result = run_case(seed21_post_evolution_state(), 231501);
  expect(std::count(result.hand.begin(), result.hand.end(), sim::Card::EarthenVessel) == 1,
         "Legacy Star did not retain the Earthen Vessel recovery.");
  expect(std::count(result.hand.begin(), result.hand.end(), sim::Card::TateLiza) == 1,
         "Legacy Star did not replace redundant Grass with Tate & Liza.");
  expect(std::count(result.hand.begin(), result.hand.end(), sim::Card::Grass) == 0,
         "Redundant direct Grass recovery remained in hand.");
}

void test_tate_switch_mode_finishes_position_axis() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None}};
  state.hand = {sim::Card::TateLiza, sim::Card::Grass, sim::Card::ErikasInvitation};
  state.discard = {sim::Card::EarthenVessel, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.deck = {sim::Card::QuickBall, sim::Card::Fire, sim::Card::Grass};

  const sim::Scenario scenario{"issue-2315-switch", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng(231504);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng, &trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);

  // Tate & Liza has an explicit switch mode. With current-turn payload already in
  // discard and the final Grass held for the unused attachment, switching the G/F
  // VSTAR Active is the deterministic ready-turn action; draw mode would destroy it:
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, switch, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Earliest complete route and supporter contention: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed seed-21 route: https://github.com/FlareZ123/pokemon-sims/issues/2315
  expect(result.active && result.active->card == sim::Card::RegidragoVstar,
         "Tate & Liza used draw mode instead of switching the Benched VSTAR Active.");
  expect(result.supporter_used,
         "Tate & Liza switch did not consume the Supporter play.");
  expect(std::count(result.discard.begin(), result.discard.end(), sim::Card::TateLiza) == 1,
         "Tate & Liza switch did not move the Supporter to discard.");
}

void test_spent_supporter_preserves_energy_recovery() {
  sim::State state = seed21_post_evolution_state();
  state.supporter_used = true;
  const sim::State result = run_case(std::move(state), 231502);
  expect(std::count(result.hand.begin(), result.hand.end(), sim::Card::TateLiza) == 0,
         "Spent Supporter slot still replaced Energy recovery with Tate & Liza.");
  expect(std::count(result.hand.begin(), result.hand.end(), sim::Card::Grass) == 1,
         "Spent Supporter control did not preserve direct Grass recovery.");
}

void test_no_searchable_energy_preserves_direct_energy_recovery() {
  sim::State state = seed21_post_evolution_state();
  state.deck.erase(
      std::remove(state.deck.begin(), state.deck.end(), sim::Card::Grass),
      state.deck.end());
  state.deck.insert(state.deck.end(), {
      sim::Card::TateLiza, sim::Card::Grass, sim::Card::HisuianHeavyBall,
      sim::Card::Grass, sim::Card::Gladion, sim::Card::Channeler,
      sim::Card::EarthenVessel});
  const sim::State result = run_case(std::move(state), 231503);
  expect(std::count(result.hand.begin(), result.hand.end(), sim::Card::TateLiza) == 0,
         "Tate recovery was chosen without searchable Basic Energy for Vessel.");
  expect(std::count(result.hand.begin(), result.hand.end(), sim::Card::Grass) == 1,
         "No-searchable-Energy control did not preserve direct Grass recovery.");
}
}  // namespace

int main() {
  try {
    test_tate_replaces_redundant_energy_recovery();
    test_tate_switch_mode_finishes_position_axis();
    test_spent_supporter_preserves_energy_recovery();
    test_no_searchable_energy_preserves_direct_energy_recovery();
    std::cout << "Issue 2315 Legacy Star Tate recovery tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
