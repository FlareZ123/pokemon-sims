#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  static bool preserves_burnet_finish(const Engine& engine) {
    return engine.banked_tapu_paid_retreat_preserves_burnet_finish();
  }
  static bool paid_tapu_available(Engine& engine) {
    return engine.banked_tapu_paid_retreat_available();
  }
  static bool retreat_tapu(Engine& engine) {
    return engine.retreat_banked_tapu_to_regidrago();
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.vstar_power_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 1, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1}};
  state.hand = {sim::Card::TateLiza, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool known = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return engine;
}

void test_paid_tapu_preserves_same_turn_burnet_finish() {
  const sim::Scenario scenario{"issue-4157-exact",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{415700};
  sim::Engine engine = make_engine(scenario, rng, route_state());

  // Retreat and Supporter are distinct turn actions. Paying Tapu's already-banked
  // one-Energy Retreat Cost keeps the Supporter action for Professor Burnet,
  // whereas Tate & Liza's switch mode consumes that same Supporter action:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Advanced Retreat and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4157
  expect(sim::EngineTestAccess::preserves_burnet_finish(engine),
         "The projected paid-Tapu route did not recognize the same-turn Burnet finish.");
  expect(sim::EngineTestAccess::paid_tapu_available(engine),
         "Tate & Liza incorrectly suppressed the stronger paid-Tapu route.");
  expect(sim::EngineTestAccess::retreat_tapu(engine),
         "The legal paid-Tapu Retreat did not execute.");
  expect(engine.state().active &&
             engine.state().active->card == sim::Card::RegidragoVstar,
         "The Apex-ready Regidrago VSTAR was not promoted.");
  expect(engine.state().retreat_used && !engine.state().supporter_used,
         "Paid Retreat incorrectly consumed the Supporter action.");

  sim::EngineTestAccess::choose_supporter(engine);
  expect(engine.state().supporter_used,
         "Professor Burnet did not consume the preserved Supporter action.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Professor Burnet did not complete the current-turn payload axis.");
}

void test_tate_keeps_priority_without_live_burnet_finish() {
  const sim::Scenario scenario{"issue-4157-controls",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};

  sim::State no_burnet = route_state();
  no_burnet.hand.erase(
      std::remove(no_burnet.hand.begin(), no_burnet.hand.end(),
                  sim::Card::ProfessorBurnet),
      no_burnet.hand.end());
  std::mt19937_64 rng_no_burnet{415701};
  sim::Engine no_burnet_engine =
      make_engine(scenario, rng_no_burnet, std::move(no_burnet));
  expect(!sim::EngineTestAccess::preserves_burnet_finish(no_burnet_engine),
         "A missing Professor Burnet must not create the route exception.");
  expect(!sim::EngineTestAccess::paid_tapu_available(no_burnet_engine),
         "Tate & Liza should keep priority when no Burnet finish exists.");

  sim::State no_payload = route_state();
  no_payload.deck = {sim::Card::Grass, sim::Card::Fire};
  std::mt19937_64 rng_no_payload{415702};
  sim::Engine no_payload_engine =
      make_engine(scenario, rng_no_payload, std::move(no_payload));
  expect(!sim::EngineTestAccess::preserves_burnet_finish(no_payload_engine),
         "A known deck without a permitted Dragon must reject the Burnet finish.");
  expect(!sim::EngineTestAccess::paid_tapu_available(no_payload_engine),
         "Tate & Liza should keep priority when Burnet cannot complete payload.");
}

void test_route_exception_respects_action_and_free_retreat_boundaries() {
  const sim::Scenario scenario{"issue-4157-boundaries",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};

  sim::State no_energy = route_state();
  no_energy.active->grass = 0;
  std::mt19937_64 rng_no_energy{415703};
  sim::Engine no_energy_engine =
      make_engine(scenario, rng_no_energy, std::move(no_energy));
  expect(!sim::EngineTestAccess::preserves_burnet_finish(no_energy_engine) &&
             !sim::EngineTestAccess::paid_tapu_available(no_energy_engine),
         "Paid Tapu Retreat must require its banked Basic Energy payment.");

  sim::State spent_retreat = route_state();
  spent_retreat.retreat_used = true;
  std::mt19937_64 rng_spent_retreat{415704};
  sim::Engine spent_retreat_engine =
      make_engine(scenario, rng_spent_retreat, std::move(spent_retreat));
  expect(!sim::EngineTestAccess::preserves_burnet_finish(spent_retreat_engine) &&
             !sim::EngineTestAccess::paid_tapu_available(spent_retreat_engine),
         "The once-per-turn Retreat action must remain enforced.");

  sim::State spent_supporter = route_state();
  spent_supporter.supporter_used = true;
  std::mt19937_64 rng_spent_supporter{415705};
  sim::Engine spent_supporter_engine =
      make_engine(scenario, rng_spent_supporter, std::move(spent_supporter));
  expect(!sim::EngineTestAccess::preserves_burnet_finish(spent_supporter_engine),
         "A spent Supporter action must reject the Burnet route exception.");

  const sim::Scenario supporter_lock{"issue-4157-supporter-lock",
                                     sim::DciProfile::MatchupFlexJit,
                                     sim::LockMode::FullSupporter, true, 5};
  std::mt19937_64 rng_supporter_lock{415706};
  sim::Engine supporter_lock_engine =
      make_engine(supporter_lock, rng_supporter_lock, route_state());
  expect(!sim::EngineTestAccess::preserves_burnet_finish(supporter_lock_engine),
         "Supporter lock must reject the Burnet route exception.");

  sim::State free_retreat = route_state();
  free_retreat.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 2, 0, 0});
  std::mt19937_64 rng_free{415707};
  sim::Engine free_engine = make_engine(scenario, rng_free, std::move(free_retreat));
  expect(!sim::EngineTestAccess::paid_tapu_available(free_engine),
         "A live zero-cost Latias retreat must supersede paid Tapu Retreat.");
}

}  // namespace

int main() {
  test_paid_tapu_preserves_same_turn_burnet_finish();
  test_tate_keeps_priority_without_live_burnet_finish();
  test_route_exception_respects_action_and_free_retreat_boundaries();
  return 0;
}