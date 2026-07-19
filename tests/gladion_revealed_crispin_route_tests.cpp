#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_blind_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = false;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

const sim::DeckRecipe& recipe() {
  static const sim::DeckRecipe value = sim::baseline_recipe();
  return value;
}

sim::State blind_energy_state() {
  sim::State state;
  state.turn = 1;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Grass, sim::Card::Dragapult,
                sim::Card::BrilliantBlender, sim::Card::Powerglass};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::MysteriousTreasure, sim::Card::ForestSealStone};
  state.prizes = {sim::Card::Crispin, sim::Card::RegidragoV, sim::Card::Grass,
                  sim::Card::TapuLeleGX, sim::Card::Serena, sim::Card::Channeler};
  return state;
}

void revealed_crispin_beats_redundant_basic() {
  const sim::Scenario scenario{"issue-971-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{97101};
  sim::Engine engine(scenario, recipe(), rng);
  sim::EngineTestAccess::set_blind_state(engine, blind_energy_state());

  // Gladion first reveals every Prize, then may exchange itself for the newly known
  // Crispin. With the prior-turn Regidrago V already solving the Basic axis and both
  // Basic Energy types still in deck, Crispin advances GGF while another Regidrago V
  // is redundant: https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/971
  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Blind Gladion did not resolve for the revealed Crispin route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Crispin) ||
      contains(after.hand, sim::Card::RegidragoV) ||
      !contains(after.prizes, sim::Card::RegidragoV)) {
    throw std::runtime_error("Gladion failed to prefer the revealed prized Crispin connector.");
  }
}

void direct_prized_vstar_stays_first_priority() {
  const sim::Scenario scenario{"issue-971-vstar-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{97102};
  sim::Engine engine(scenario, recipe(), rng);
  sim::State state = blind_energy_state();
  state.prizes[2] = sim::Card::RegidragoVstar;
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::RegidragoVstar));
  sim::EngineTestAccess::set_blind_state(engine, std::move(state));

  // A newly revealed VSTAR is the exact missing evolution axis and must outrank the
  // slower Crispin connector: https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/971
  if (!sim::EngineTestAccess::play_gladion(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("The revealed Crispin route displaced direct VSTAR recovery.");
  }
}

void dead_crispin_does_not_displace_redundant_fallback() {
  const sim::Scenario scenario{"issue-971-dead-crispin", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{97103};
  sim::Engine engine(scenario, recipe(), rng);
  sim::State state = blind_energy_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Fire),
                   state.deck.end());
  sim::EngineTestAccess::set_blind_state(engine, std::move(state));

  // Crispin cannot provide the required different Energy route after the legal Prize
  // reveal proves no Fire remains in deck: https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/971
  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion unexpectedly declined the mandatory blind exchange.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (contains(after.hand, sim::Card::Crispin) ||
      !contains(after.hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("Gladion selected Crispin after K1 proved the Fire route dead.");
  }
}

void missing_basic_remains_first_priority() {
  const sim::Scenario scenario{"issue-971-basic-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{97104};
  sim::Engine engine(scenario, recipe(), rng);
  sim::State state = blind_energy_state();
  state.active.reset();
  sim::EngineTestAccess::set_blind_state(engine, std::move(state));

  // A genuinely missing revealed Regidrago V remains the immediate axis:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/issues/971
  if (!sim::EngineTestAccess::play_gladion(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("The revealed Crispin route displaced direct Basic recovery.");
  }
}

void seed_28_full_trace_selects_crispin_and_reaches_turn_two() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-second scenario.");
  std::mt19937_64 rng{28};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe(), rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const bool selected_crispin = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("exchanged Gladion for Crispin") != std::string::npos;
      });
  // Exact full-trace regression: https://github.com/FlareZ123/pokemon-sims/issues/971
  // Trace contract: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
  if (!selected_crispin || outcome.first_ready_turn != 2) {
    throw std::runtime_error("Seed 28 did not take revealed Crispin to T2 readiness.");
  }
}

}  // namespace

int main() {
  revealed_crispin_beats_redundant_basic();
  direct_prized_vstar_stays_first_priority();
  dead_crispin_does_not_displace_redundant_fallback();
  missing_basic_remains_first_priority();
  seed_28_full_trace_selects_crispin_and_reaches_turn_two();
  return 0;
}
