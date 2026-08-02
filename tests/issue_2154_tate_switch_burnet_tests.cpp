#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstddef>
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
  static bool tate_burnet_route(const Engine& engine) {
    return engine.tate_switch_then_next_turn_burnet_route();
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static void advance_without_future_draw(Engine& engine) {
    ++engine.state_.turn;
    engine.state_.supporter_used = false;
    engine.state_.manual_energy_used = false;
    engine.state_.retreat_used = false;
    engine.state_.stadium_used = false;
    engine.state_.dark_asset_used = false;
    engine.state_.turn_ended = false;
    engine.state_.discarded_this_turn.clear();
  }
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
  state.hand = {sim::Card::TateLiza, sim::Card::ProfessorBurnet};
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

void test_exact_switch_then_burnet_route() {
  const sim::Scenario scenario{"issue-2154-exact",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{215400};
  sim::Engine engine = make_engine(scenario, rng, route_state());

  // Tate & Liza legally promotes the prepared GGF Regidrago VSTAR on T3.
  // Professor Burnet remains for T4 and discards a Dragon during that ready turn:
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // One-Supporter-per-turn and switching procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest deterministic route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2154
  expect(sim::EngineTestAccess::tate_burnet_route(engine),
         "The exact Tate switch then Burnet route was not recognized.");
  sim::EngineTestAccess::choose_supporter(engine);
  expect(engine.state().active &&
             engine.state().active->card == sim::Card::RegidragoVstar,
         "Tate & Liza did not promote the prepared Regidrago VSTAR.");
  expect(std::find(engine.state().hand.begin(), engine.state().hand.end(),
                   sim::Card::ProfessorBurnet) != engine.state().hand.end(),
         "Tate draw mode removed the deterministic Burnet continuation.");

  sim::EngineTestAccess::advance_without_future_draw(engine);
  sim::EngineTestAccess::choose_supporter(engine);
  expect(engine.state().supporter_used,
         "Professor Burnet did not consume the next turn's Supporter action.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Professor Burnet did not establish the current-turn Dragon payload.");
}

void test_route_boundaries() {
  const sim::Scenario strict{"issue-2154-controls", sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 5};
  const auto rejected = [&](sim::State state, const sim::Scenario& scenario,
                            const std::uint64_t seed, const char* message,
                            const bool known = true) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state), known);
    expect(!sim::EngineTestAccess::tate_burnet_route(engine), message);
  };

  sim::State no_burnet = route_state();
  no_burnet.hand.erase(std::remove(no_burnet.hand.begin(), no_burnet.hand.end(),
                                   sim::Card::ProfessorBurnet),
                       no_burnet.hand.end());
  rejected(std::move(no_burnet), strict, 215401,
           "The route must require Professor Burnet in hand.");

  sim::State no_payload = route_state();
  no_payload.deck = {sim::Card::Grass, sim::Card::Fire};
  rejected(std::move(no_payload), strict, 215402,
           "A known deck without a Dragon payload must reject the route.");

  sim::State incomplete_energy = route_state();
  incomplete_energy.bench.front().grass = 1;
  rejected(std::move(incomplete_energy), strict, 215403,
           "An incompletely powered switch target must reject the route.");

  sim::State no_switch_target = route_state();
  no_switch_target.bench.front().card = sim::Card::RegidragoV;
  rejected(std::move(no_switch_target), strict, 215404,
           "A Bench without Regidrago VSTAR must reject the route.");

  const sim::Scenario supporter_lock{"issue-2154-supporter-lock",
                                     sim::DciProfile::StrictJit,
                                     sim::LockMode::FullSupporter, false, 5};
  rejected(route_state(), supporter_lock, 215405,
           "Supporter lock must reject Tate and Burnet.");

  const sim::Scenario expired{"issue-2154-horizon", sim::DciProfile::StrictJit,
                              sim::LockMode::None, false, 3};
  rejected(route_state(), expired, 215406,
           "The route must not start after the configured horizon.");

  rejected(route_state(), strict, 215407,
           "K0 must not infer the payload availability needed by Burnet.", false);
}

void test_seed_150_reaches_turn_four_in_order() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing matchup-flex-jit/go-first scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{150};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const std::size_t tate = trace_index(
      trace, 3, "PLAY SUPPORTER", "Tate & Liza switch mode");
  const std::size_t burnet = trace_index(
      trace, 4, "PLAY SUPPORTER", "Searched and discarded");
  const std::size_t ready = trace_index(
      trace, 4, "READY", "Active Regidrago VSTAR has GGF");

  // The fixed-seed regression uses only the K1-visible T3 route described by the
  // confirmed report and reaches readiness through sequential Supporter turns:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/2154
  expect(outcome.first_ready_turn == 4,
         "Seed 150 did not reach matchup-flex readiness on T4.");
  expect(tate < burnet && burnet < ready,
         "Seed 150 did not preserve Tate switch then Burnet ordering.");
}

void test_item_lock_seed_392_reaches_turn_four_in_order() {
  const auto scenario =
      sim::scenario_by_label("strict-jit-turn2-item-lock/go-second");
  if (!scenario) {
    throw std::runtime_error("Missing strict-jit-turn2-item-lock/go-second scenario");
  }

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{392};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const std::size_t tate = trace_index(
      trace, 3, "PLAY SUPPORTER", "Tate & Liza switch mode");
  const std::size_t burnet = trace_index(
      trace, 4, "PLAY SUPPORTER", "Searched and discarded");
  const std::size_t ready = trace_index(
      trace, 4, "READY", "Active Regidrago VSTAR has GGF");

  // Item lock does not prohibit either Supporter. The same visible two-turn route
  // therefore remains legal in the scheduled-lock scenario:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // https://github.com/FlareZ123/pokemon-sims/issues/2154
  expect(outcome.first_ready_turn == 4,
         "Item-lock seed 392 did not reach strict-JIT readiness on T4.");
  expect(tate < burnet && burnet < ready,
         "Item-lock seed 392 did not preserve Tate switch then Burnet ordering.");
}

}  // namespace

int main() {
  test_exact_switch_then_burnet_route();
  test_route_boundaries();
  test_seed_150_reaches_turn_four_in_order();
  test_item_lock_seed_392_reaches_turn_four_in_order();
  return 0;
}
