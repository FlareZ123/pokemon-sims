#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }
  static bool paid_oricorio_route(const Engine& engine) {
    return engine.issue_2158_paid_oricorio_retreat_burnet_available();
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
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
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1}};
  state.hand = {sim::Card::Grass, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool known = true,
                        sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return engine;
}

std::size_t trace_index(const sim::TraceLog& trace, const int turn,
                        const std::string& action,
                        const std::string& detail) {
  const std::string prefix = "T" + std::to_string(turn) + " | ";
  for (std::size_t index = 0; index < trace.lines.size(); ++index) {
    const std::string& line = trace.lines[index];
    if (line.starts_with(prefix) && line.find(action) != std::string::npos &&
        line.find(detail) != std::string::npos) {
      return index;
    }
  }
  return trace.lines.size();
}

void test_exact_paid_retreat_then_burnet_route() {
  const sim::Scenario scenario{"issue-2158-exact",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{215800};
  sim::Engine engine = make_engine(scenario, rng, route_state());

  // Oricorio has a one-Colorless Retreat Cost. Attaching the held Grass, paying
  // that Retreat Cost, and promoting the already-GGF Regidrago VSTAR preserves the
  // unused Supporter action for Professor Burnet's same-turn Dragon discard:
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official attachment, Retreat, Supporter, search, and discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2158
  expect(sim::EngineTestAccess::paid_oricorio_route(engine),
         "The exact paid Oricorio retreat route was not recognized.");
  expect(sim::EngineTestAccess::attach_manual(engine),
         "The paid Retreat attachment did not execute.");
  expect(engine.state().active &&
             engine.state().active->card == sim::Card::RegidragoVstar,
         "The GGF Regidrago VSTAR was not promoted.");
  expect(engine.state().manual_energy_used && engine.state().retreat_used,
         "The route did not consume the attachment and Retreat actions.");
  expect(std::find(engine.state().discard.begin(), engine.state().discard.end(),
                   sim::Card::Grass) != engine.state().discard.end(),
         "The attached Grass was not discarded as the Retreat payment.");

  sim::EngineTestAccess::choose_supporter(engine);
  expect(engine.state().supporter_used,
         "Professor Burnet did not consume the Supporter action.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Professor Burnet did not establish the current-turn Dragon payload.");
}

void test_route_boundaries() {
  const sim::Scenario flex{"issue-2158-controls",
                           sim::DciProfile::MatchupFlexJit,
                           sim::LockMode::None, true, 5};
  const auto rejected = [&](sim::State state, const sim::Scenario& scenario,
                            const std::uint64_t seed, const char* message,
                            const bool known = true) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state), known);
    expect(!sim::EngineTestAccess::paid_oricorio_route(engine), message);
  };

  sim::State no_burnet = route_state();
  no_burnet.hand.erase(
      std::remove(no_burnet.hand.begin(), no_burnet.hand.end(),
                  sim::Card::ProfessorBurnet),
      no_burnet.hand.end());
  rejected(std::move(no_burnet), flex, 215801,
           "The route must require Professor Burnet in hand.");

  sim::State no_payment = route_state();
  no_payment.hand.erase(
      std::remove(no_payment.hand.begin(), no_payment.hand.end(), sim::Card::Grass),
      no_payment.hand.end());
  rejected(std::move(no_payment), flex, 215802,
           "The route must require a Basic Energy Retreat payment.");

  sim::State incomplete_energy = route_state();
  incomplete_energy.bench.front().grass = 1;
  rejected(std::move(incomplete_energy), flex, 215803,
           "An incompletely powered promotion target must reject the route.");

  sim::State no_payload = route_state();
  no_payload.deck = {sim::Card::Grass, sim::Card::Fire};
  rejected(std::move(no_payload), flex, 215804,
           "A known deck without a Dragon payload must reject the route.");

  sim::State spent_attachment = route_state();
  spent_attachment.manual_energy_used = true;
  rejected(std::move(spent_attachment), flex, 215805,
           "A spent manual attachment must reject the route.");

  sim::State spent_retreat = route_state();
  spent_retreat.retreat_used = true;
  rejected(std::move(spent_retreat), flex, 215806,
           "A spent Retreat action must reject the route.");

  const sim::Scenario supporter_lock{"issue-2158-supporter-lock",
                                     sim::DciProfile::MatchupFlexJit,
                                     sim::LockMode::FullSupporter, true, 5};
  rejected(route_state(), supporter_lock, 215807,
           "Supporter lock must reject the Burnet route.");

  rejected(route_state(), flex, 215808,
           "K0 must not infer the Dragon payload needed by Burnet.", false);

  sim::State wrong_active = route_state();
  wrong_active.active->card = sim::Card::CrobatV;
  rejected(std::move(wrong_active), flex, 215809,
           "A different Active Pokemon must not use the Oricorio-specific route.");

  sim::State free_retreat = route_state();
  free_retreat.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 2, 0, 0});
  rejected(std::move(free_retreat), flex, 215810,
           "A live Skyliner route must supersede the paid Retreat route.");
}

void test_seed_150_reaches_turn_three_in_order() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  if (!scenario) {
    throw std::runtime_error("Missing matchup-flex-jit/go-first scenario");
  }

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{150};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const std::size_t attach = trace_index(
      trace, 3, "ATTACH", "immediate Retreat Cost");
  const std::size_t retreat = trace_index(
      trace, 3, "RETREAT", "before Professor Burnet");
  const std::size_t burnet = trace_index(
      trace, 3, "PLAY SUPPORTER", "Searched and discarded");
  const std::size_t ready = trace_index(
      trace, 3, "READY", "Active Regidrago VSTAR has GGF");

  // The fixed seed exposes every route prerequisite through legal K1 inspection.
  // The legal attachment, paid Retreat, Burnet, and ready-state sequence must all
  // occur on T3 without consulting a future draw:
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Observable-information and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2158
  expect(outcome.first_ready_turn == 3,
         "Seed 150 did not reach matchup-flex readiness on T3.");
  expect(attach < retreat && retreat < burnet && burnet < ready,
         "Seed 150 did not preserve attach, Retreat, Burnet, READY ordering.");
}

}  // namespace

int main() {
  test_exact_paid_retreat_then_burnet_route();
  test_route_boundaries();
  test_seed_150_reaches_turn_three_in_order();
  return 0;
}
