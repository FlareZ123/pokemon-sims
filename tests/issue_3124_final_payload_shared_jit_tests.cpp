#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void set_knowledge(Engine& engine, const bool deck_seen) {
    engine.deck_seen_ = deck_seen;
  }
  static bool protects_final_payload(const Engine& engine) {
    return engine.issue_2323_protect_final_t1_payload();
  }
  static std::optional<Card> choose_discard(const Engine& engine,
                                            const bool permit_payload,
                                            const bool flex_fodder) {
    return engine.choose_discard(permit_payload, flex_fodder);
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

struct Fixture {
  Fixture(const sim::DciProfile dci,
          const bool going_first = false,
          const sim::LockMode locks = sim::LockMode::None)
      : scenario{"issue-3124/exact", dci, locks, going_first, 5},
        recipe{sim::baseline_recipe()},
        rng{3124},
        engine{scenario, recipe, rng} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

sim::State protected_payload_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(
      sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None, 1});
  state.hand = {sim::Card::Dragapult, sim::Card::EarthenVessel,
                sim::Card::Fire};
  state.discard = {sim::Card::MysteriousTreasure,
                   sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire};
  return state;
}

bool protects(const sim::DciProfile dci, sim::State state,
              const bool deck_seen = true,
              const bool going_first = false,
              const sim::LockMode locks = sim::LockMode::None) {
  Fixture fixture{dci, going_first, locks};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(fixture.engine, deck_seen);
  return sim::EngineTestAccess::protects_final_payload(fixture.engine);
}

std::optional<sim::Card> discard_candidate(
    const sim::DciProfile dci, sim::State state,
    const bool permit_payload, const bool flex_fodder) {
  Fixture fixture{dci};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(fixture.engine, true);
  return sim::EngineTestAccess::choose_discard(
      fixture.engine, permit_payload, flex_fodder);
}

void test_same_turn_jit_profiles_share_final_payload_udp() {
  const sim::State state = protected_payload_state();

  // The first T1 Dragon discard started the Regidrago V evolution timer but cannot
  // satisfy a later ready-turn Apex Dragon payload event. StrictJit and
  // MatchupFlexJit share that same-turn requirement, so the final held Dragon is
  // UDP in both profiles until the readiness window opens.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Item/search/evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-turn JIT and dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Original route / upstream shared-JIT fix / confirmed downstream bug: https://github.com/FlareZ123/pokemon-sims/issues/2323 https://github.com/FlareZ123/pokemon-sims/issues/3029 https://github.com/FlareZ123/pokemon-sims/issues/3124
  expect(protects(sim::DciProfile::StrictJit, state),
         "StrictJit lost the established final-payload UDP guard.");
  expect(protects(sim::DciProfile::MatchupFlexJit, state),
         "MatchupFlexJit still bypassed the shared same-turn-JIT payload guard.");
  expect(!protects(sim::DciProfile::NoDiscardControl, state),
         "The same-turn-JIT guard leaked into NoDiscardControl.");
}

void test_actual_discard_filter_blocks_final_payload_in_both_jit_profiles() {
  const sim::State state = protected_payload_state();

  // The wrapper must reject the payload candidate itself, rather than merely make
  // the helper symmetric. A Trainer with a discard cost can remain unplayed, so
  // consuming the final ready-turn Dragon here is dominated.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // DCI / decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3124
  expect(!discard_candidate(sim::DciProfile::StrictJit, state, true, false)
              .has_value(),
         "StrictJit allowed the final protected Dragon as a discard cost.");
  expect(!discard_candidate(sim::DciProfile::MatchupFlexJit, state, true, false)
              .has_value(),
         "MatchupFlexJit allowed the final protected Dragon as a discard cost.");
}

void test_guard_requires_the_exact_public_udp_state() {
  sim::State no_prior_payload = protected_payload_state();
  no_prior_payload.discarded_this_turn.clear();
  expect(!protects(sim::DciProfile::MatchupFlexJit,
                   std::move(no_prior_payload)),
         "Final-payload guard opened without a prior same-turn payload spend.");

  sim::State two_payloads = protected_payload_state();
  two_payloads.hand.push_back(sim::Card::Appletun);
  expect(!protects(sim::DciProfile::MatchupFlexJit, std::move(two_payloads)),
         "Final-payload guard protected a Dragon while another payload remained.");

  sim::State old_timer = protected_payload_state();
  old_timer.bench.front().entered_turn = 0;
  expect(!protects(sim::DciProfile::MatchupFlexJit, std::move(old_timer)),
         "Final-payload guard ignored that Regidrago V entered on a prior turn.");

  expect(!protects(sim::DciProfile::MatchupFlexJit,
                   protected_payload_state(), false),
         "Final-payload guard bypassed its K1 knowledge boundary.");
  expect(!protects(sim::DciProfile::MatchupFlexJit,
                   protected_payload_state(), true, true),
         "Final-payload guard leaked into the going-first T1 route.");
  expect(!protects(sim::DciProfile::MatchupFlexJit,
                   protected_payload_state(), true, false,
                   sim::LockMode::FullRuleBoxAbility),
         "Issue #3124 changed the existing lock-scope boundary.");

  sim::State later_turn = protected_payload_state();
  later_turn.turn = 2;
  expect(!protects(sim::DciProfile::MatchupFlexJit, std::move(later_turn)),
         "T1-specific final-payload guard leaked into a later readiness turn.");

  // These controls preserve the original #2323 scope while changing only the
  // profile identity bug fixed here.
  // K1 and JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Advanced turn/evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed scope: https://github.com/FlareZ123/pokemon-sims/issues/3124
}

void test_safer_non_payload_dci_keeps_precedence() {
  sim::State state = protected_payload_state();
  state.hand.push_back(sim::Card::Grant);
  const auto candidate = discard_candidate(
      sim::DciProfile::MatchupFlexJit, std::move(state), false, true);

  // Grant has no setup-axis value in these recipes and the established DCI selector
  // chooses it before any protected Dragon. Generalizing the payload UDP guard must
  // leave that safer ordinary cost untouched.
  // Grant: https://api.pokemontcg.io/v2/cards/swsh10-144
  // Dynamic DCI / resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3124
  expect(candidate.has_value() && *candidate == sim::Card::Grant,
         "Shared-JIT payload protection displaced a safer non-payload DCI cost.");
}

}  // namespace

int main() {
  try {
    test_same_turn_jit_profiles_share_final_payload_udp();
    test_actual_discard_filter_blocks_final_payload_in_both_jit_profiles();
    test_guard_requires_the_exact_public_udp_state();
    test_safer_non_payload_dci_keeps_precedence();
    std::cout << "Issue 3124 final-payload shared-JIT tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
