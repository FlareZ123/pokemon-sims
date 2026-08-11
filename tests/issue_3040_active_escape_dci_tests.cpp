#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static std::optional<Card> choose_discard(
      Engine& engine, const std::optional<Card> excluded_from_cost) {
    return engine.choose_discard(false, true, true, excluded_from_cost, false);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State unique_escape_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0}};
  state.hand = {
      sim::Card::EarthenVessel,
      sim::Card::ProfessorTuro,
      sim::Card::MegaDragonite,
      sim::Card::Dragapult,
      sim::Card::Fire,
  };
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::MysteriousTreasure,
      sim::Card::Grass,
      sim::Card::Fire,
  };
  state.prizes = {
      sim::Card::LatiasEx,
      sim::Card::Arven,
      sim::Card::FieldBlower,
      sim::Card::Guzma,
      sim::Card::Klara,
      sim::Card::PathToPeak,
  };
  return state;
}

std::optional<sim::Card> choose_vessel_cost(sim::State state,
                                             const bool deck_seen = true) {
  const sim::Scenario scenario{
      "issue-3040", sim::DciProfile::MatchupFlexJit,
      sim::LockMode::None, false, 5};
  std::mt19937_64 rng{3040};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen, false);
  return sim::EngineTestAccess::choose_discard(
      engine, sim::Card::EarthenVessel);
}

void test_unique_turo_escape_uses_redundant_payload_cost() {
  const auto cost = choose_vessel_cost(unique_escape_state());
  // K1 proves Latias ex is unavailable, the zero-Energy Dialga-GX has printed
  // Retreat Cost 3, and Professor Turo's Scenario is the only held connector that
  // clears the Active position. One of two held payloads is therefore higher DCI
  // while another remains for the actual same-ready-turn JIT event.
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K1/JIT/DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
  expect(cost.has_value() && sim::is_payload(*cost),
         "Unique Active escape did not redirect Vessel to a redundant payload.");
  expect(*cost != sim::Card::ProfessorTuro,
         "Unique Active escape still discarded Professor Turo's Scenario.");
}

void test_unique_turo_escape_refuses_last_payload() {
  sim::State state = unique_escape_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Dragapult));
  const auto cost = choose_vessel_cost(std::move(state));
  // The sole payload remains protected because spending it early would erase the
  // later same-ready-turn JIT outlet. With no safe alternate cost, Vessel can stay
  // unplayed rather than discard the unique Active escape.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
  expect(!cost.has_value(),
         "Unique Active escape spent Turo or the sole protected payload.");
}

void test_equal_active_connector_releases_turo() {
  sim::State state = unique_escape_state();
  state.hand.push_back(sim::Card::TateLiza);
  const auto cost = choose_vessel_cost(std::move(state));
  // Tate & Liza provides a separate legal switch mode, so Turo is no longer the
  // unique observable Active-position connector and returns to ordinary flex DCI.
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Official one-Supporter-per-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
  expect(cost.has_value() && *cost == sim::Card::ProfessorTuro,
         "Turo stayed protected despite another held Active connector.");
}

void test_k0_does_not_infer_prized_latias() {
  const auto cost = choose_vessel_cost(unique_escape_state(), false);
  // K0 cannot use the hidden Prize identity to assign Turo special protection.
  // Knowledge-state specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
  expect(cost.has_value() && *cost == sim::Card::ProfessorTuro,
         "K0 inferred hidden Prize information while ranking Turo.");
}

void test_seed_871_preserves_escape_and_reaches_ready_state() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Issue-3040 trace fixture is unavailable.");

  std::mt19937_64 rng{871};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();

  // This CI witness establishes K1 on T1 with Latias ex Prized. T2 must preserve
  // Professor Turo's Scenario while a redundant Dragon can fund Earthen Vessel,
  // then clear the zero-Energy Active so the Regidrago line can finish legally.
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // CI witness: https://github.com/FlareZ123/pokemon-sims/actions/runs/31480655726
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
  expect(outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 4 &&
             !outcome.setup_failed,
         "Seed 871 still strands the powered Regidrago after the DCI fix.");
  expect(!trace_contains(trace,
                         "Professor Turo's Scenario (Earthen Vessel"),
         "Seed 871 still spends Professor Turo as the Vessel discard cost.");
  expect(trace_contains(trace, "READY |"),
         "Seed 871 did not emit a legal ready-state trace.");
}
}  // namespace

int main() {
  try {
    test_unique_turo_escape_uses_redundant_payload_cost();
    test_unique_turo_escape_refuses_last_payload();
    test_equal_active_connector_releases_turo();
    test_k0_does_not_infer_prized_latias();
    test_seed_871_preserves_escape_and_reaches_ready_state();
    std::cout << "Issue 3040 Active-escape DCI tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
