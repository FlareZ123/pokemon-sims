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

  static const State& state(const Engine& engine) { return engine.state_; }

  static std::optional<Card> choose_discard(
      Engine& engine, const std::optional<Card> excluded_from_cost) {
    return engine.choose_discard(false, true, true, excluded_from_cost, false);
  }

  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
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

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
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

sim::State staging_state() {
  sim::State state = unique_escape_state();
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1}};
  state.hand = {sim::Card::ProfessorTuro, sim::Card::Dragapult};
  state.discard = {sim::Card::EarthenVessel, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
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

sim::State choose_staging_supporter(sim::State state,
                                    const bool deck_seen = true) {
  const sim::Scenario scenario{
      "issue-3040-staging", sim::DciProfile::MatchupFlexJit,
      sim::LockMode::None, false, 5};
  std::mt19937_64 rng{3041};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen, false);
  sim::EngineTestAccess::choose_supporter(engine);
  return sim::EngineTestAccess::state(engine);
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

void test_k1_turo_stages_powered_prior_turn_regidrago() {
  const sim::State after = choose_staging_supporter(staging_state());
  // The public K1 state proves the blocking Basic cannot retreat for free and a
  // Regidrago VSTAR remains searchable. Turo may return that zero-resource Active
  // and promote the prior-turn GGF Regidrago V without spending the future VSTAR
  // search or the Dragon reserved for the later same-turn payload event.
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Dialga-GX / Retreat Cost 3 witness: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, Active replacement, Retreat, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1/JIT/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
  expect(after.active && after.active->card == sim::Card::RegidragoV &&
             after.active->grass == 2 && after.active->fire == 1,
         "Turo did not promote the powered prior-turn Regidrago V.");
  expect(after.bench.empty() && contains(after.hand, sim::Card::DialgaGX) &&
             contains(after.discard, sim::Card::ProfessorTuro) &&
             after.supporter_used,
         "Turo staging did not preserve the expected public zones and Supporter use.");
}

void test_k0_holds_turo_staging_route() {
  const sim::State after = choose_staging_supporter(staging_state(), false);
  // Without K1, the selector cannot infer Latias Prize status or the exact VSTAR
  // search target from hidden zones.
  // Knowledge-state specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
  expect(after.active && after.active->card == sim::Card::DialgaGX &&
             contains(after.hand, sim::Card::ProfessorTuro) &&
             !after.supporter_used,
         "K0 used hidden-zone knowledge to stage Turo early.");
}

void test_k1_holds_turo_when_vstar_is_not_searchable() {
  sim::State state = staging_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::RegidragoVstar),
                   state.deck.end());
  const sim::State after = choose_staging_supporter(std::move(state));
  // K1 must refuse the staging action once the exact VSTAR continuation is absent
  // from the known deck.
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Knowledge-state specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
  expect(after.active && after.active->card == sim::Card::DialgaGX &&
             contains(after.hand, sim::Card::ProfessorTuro) &&
             !after.supporter_used,
         "Turo staging ignored the K1-known missing VSTAR continuation.");
}

void test_turo_staging_preserves_attached_active_resources() {
  sim::State state = staging_state();
  state.active->grass = 1;
  const sim::State after = choose_staging_supporter(std::move(state));
  // The scoped staging route is limited to a zero-resource blocking Active. A Basic
  // Energy attached to that Pokémon is observable discrete value and must not be
  // burned merely to advance position.
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Dynamic DCI/resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
  expect(after.active && after.active->card == sim::Card::DialgaGX &&
             after.active->grass == 1 && contains(after.hand, sim::Card::ProfessorTuro) &&
             !after.supporter_used,
         "Turo staging discarded observable Active resources.");
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
  // then use Turo to stage the powered Regidrago V before the later VSTAR search.
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
  expect(trace_contains(trace, "promoted the powered prior-turn Regidrago V"),
         "Seed 871 did not execute the K1 Turo staging route.");
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
    test_k1_turo_stages_powered_prior_turn_regidrago();
    test_k0_holds_turo_staging_route();
    test_k1_holds_turo_when_vstar_is_not_searchable();
    test_turo_staging_preserves_attached_active_resources();
    test_seed_871_preserves_escape_and_reaches_ready_state();
    std::cout << "Issue 3040 Active-escape DCI tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
