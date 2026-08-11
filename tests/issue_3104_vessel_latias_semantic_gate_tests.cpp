#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }

  static bool play_current_latias_finish(Engine& engine) {
    return engine.play_issue_1672_vessel_payload_finish();
  }

  static bool play_canonical_latias_finish(Engine& engine) {
    return engine.play_issue_1672_vessel_payload_finish_issue2437_original();
  }

  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& zone, const sim::Card card) {
  return std::find(zone.begin(), zone.end(), card) != zone.end();
}

sim::State canonical_state() {
  sim::State state;
  state.turn = 3;
  state.supporter_used = true;
  state.vstar_power_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                                     sim::Tool::ForestSealStone, 1});
  state.hand = {sim::Card::EarthenVessel, sim::Card::LatiasEx,
                sim::Card::RegidragoVstar, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::QuickBall};
  state.prizes = {sim::Card::Gladion, sim::Card::Crispin,
                  sim::Card::MysteriousTreasure, sim::Card::ProfessorBurnet,
                  sim::Card::FieldBlower, sim::Card::Powerglass};
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{3104};
  sim::Engine engine;

  Fixture(const sim::DciProfile dci, const sim::LockMode locks,
          const int max_turn = 5)
      : scenario{"issue-3104", dci, locks, false, max_turn},
        engine{scenario, recipe, rng} {}
};

void require_route_resolves(const sim::DciProfile dci,
                            const sim::LockMode locks,
                            const bool path_lock_removed,
                            const bool current_dde_layer) {
  Fixture fixture{dci, locks};
  sim::State state = canonical_state();
  state.path_lock_removed = path_lock_removed;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  const bool played = current_dde_layer
      ? sim::EngineTestAccess::play_current_latias_finish(fixture.engine)
      : sim::EngineTestAccess::play_canonical_latias_finish(fixture.engine);
  require(played, "Legal Vessel-Latias route was rejected by a scenario-coordinate gate.");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  // Earthen Vessel discards one other card and searches Basic Energy. Here the
  // held Dragon is DCI 1 because its mandatory discard establishes the same-turn
  // Apex payload, while searched Grass is the unused manual attachment target:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dynamic DCI / same-ready-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3104
  require(contains(after.discard, sim::Card::EarthenVessel),
          "Vessel did not resolve on the admitted #3104 route.");
  require(contains(after.discard, sim::Card::MegaDragonite),
          "Vessel did not use the held Dragon as the current-turn payload cost.");
  require(contains(after.hand, sim::Card::Grass),
          "Vessel did not search the completing Basic Energy.");
}

void test_strict_jit_earlier_turn_is_legal() {
  // StrictJit and MatchupFlexJit share the same same-ready-turn payload contract.
  // No card in the route has a final-turn-only timing clause, so a legal T3 route
  // in a five-turn diagnostic horizon must resolve on T3:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earliest deterministic route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3104
  require_route_resolves(sim::DciProfile::StrictJit, sim::LockMode::None,
                         false, false);
  require_route_resolves(sim::DciProfile::StrictJit, sim::LockMode::None,
                         false, true);
}

void test_recovered_rule_box_ability_state_is_legal() {
  // Path suppresses Rule Box Pokemon Abilities only while its effect is active.
  // Once the modeled lock-removal state is true, Skyliner is available again and
  // scenario identity cannot keep suppressing this deterministic route:
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Advanced Ability/Stadium procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3104
  require_route_resolves(sim::DciProfile::StrictJit,
                         sim::LockMode::FullRuleBoxAbility, true, false);
  require_route_resolves(sim::DciProfile::StrictJit,
                         sim::LockMode::FullRuleBoxAbility, true, true);
}

void test_live_lock_still_blocks_route() {
  Fixture fixture{sim::DciProfile::StrictJit,
                  sim::LockMode::FullRuleBoxAbility};
  sim::EngineTestAccess::set_state(fixture.engine, canonical_state());

  // Skyliner is required to promote the prepared Regidrago without spending the
  // already-reserved Retreat action. A live Rule Box Ability lock therefore makes
  // the advertised completion unavailable:
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3104
  require(!sim::EngineTestAccess::play_current_latias_finish(fixture.engine),
          "Live Rule Box Ability lock incorrectly admitted the Latias route.");
}

void test_item_lock_and_k0_still_block_route() {
  {
    Fixture fixture{sim::DciProfile::StrictJit,
                    sim::LockMode::TurnTwoItem};
    sim::EngineTestAccess::set_state(fixture.engine, canonical_state());
    require(!sim::EngineTestAccess::play_current_latias_finish(fixture.engine),
            "Live Item lock incorrectly admitted Earthen Vessel.");
  }
  {
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None};
    sim::EngineTestAccess::set_state(fixture.engine, canonical_state(), false);
    require(!sim::EngineTestAccess::play_current_latias_finish(fixture.engine),
            "K0 state incorrectly used hidden deck knowledge for the Vessel route.");
  }

  // Item effects cannot be played through Item lock, and exact deck membership is
  // unavailable before a legal inspection establishes K1:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Advanced Item/search procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3104
}

void test_physical_route_guards_remain() {
  {
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None};
    sim::State state = canonical_state();
    state.manual_energy_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    require(!sim::EngineTestAccess::play_current_latias_finish(fixture.engine),
            "Spent manual attachment incorrectly admitted the Latias finish.");
  }
  {
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None};
    sim::State state = canonical_state();
    state.retreat_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    require(!sim::EngineTestAccess::play_current_latias_finish(fixture.engine),
            "Spent Retreat incorrectly admitted the Latias finish.");
  }
  {
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None};
    sim::State state = canonical_state();
    state.bench.front().entered_turn = state.turn;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    require(!sim::EngineTestAccess::play_current_latias_finish(fixture.engine),
            "Same-turn Regidrago V incorrectly admitted VSTAR evolution.");
  }
  {
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None};
    sim::State state = canonical_state();
    while (state.bench.size() < 5) {
      state.bench.push_back(sim::Pokemon{sim::Card::Pineco, 1});
    }
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    require(!sim::EngineTestAccess::play_current_latias_finish(fixture.engine),
            "Full Bench incorrectly advertised a Latias promotion route.");
  }

  // Evolution requires a prior-turn Basic, Latias must have Bench space, and each
  // turn permits one manual attachment and one Retreat action:
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Advanced evolution, Bench, attachment, and Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3104
}

}  // namespace

int main() {
  test_strict_jit_earlier_turn_is_legal();
  test_recovered_rule_box_ability_state_is_legal();
  test_live_lock_still_blocks_route();
  test_item_lock_and_k0_still_block_route();
  test_physical_route_guards_remain();
}
