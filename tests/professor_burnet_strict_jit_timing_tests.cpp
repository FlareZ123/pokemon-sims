#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_professor_burnet(Engine& engine) {
    return engine.play_professor_burnet();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

int count(const std::vector<sim::Card>& zone, const sim::Card card) {
  return static_cast<int>(std::count(zone.begin(), zone.end(), card));
}

struct Fixture {
  explicit Fixture(const sim::Scenario& selected_scenario)
      : scenario(selected_scenario), recipe(sim::baseline_recipe()), rng(719),
        engine(scenario, recipe, rng) {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

sim::State base_state() {
  sim::State state;
  state.turn = 2;
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult};
  return state;
}

void test_rejects_burnet_after_manual_attachment_window_closes() {
  Fixture fixture({"burnet-manual-window-closed", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = base_state();
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0};
  state.hand.push_back(sim::Card::Fire);
  state.manual_energy_used = true;
  const sim::State before = state;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // One manual Energy attachment is allowed each turn. Burnet only discards deck
  // cards, so a strict-JIT payload cannot complete Apex Dragon after that attachment
  // window is already spent while Fire remains missing:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  expect(!sim::EngineTestAccess::play_professor_burnet(fixture.engine),
         "Burnet should be held when strict JIT cannot finish the Energy axis");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.hand == before.hand, "Burnet and held Fire should remain in hand");
  expect(after.deck == before.deck, "the payload deck should remain unchanged");
  expect(after.discard.empty(), "no payload or Supporter should enter discard");
  expect(!after.supporter_used, "the Supporter slot should remain available");
}

void test_rejects_burnet_when_no_needed_energy_is_held() {
  Fixture fixture({"burnet-no-held-energy", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = base_state();
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1};
  state.manual_energy_used = false;
  const sim::State before = state;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Before the Supporter decision, the active turn procedure has already attempted
  // Item searches, Basic-entry Abilities, and the manual attachment. If GGF is still
  // incomplete, Burnet cannot create an Energy route because it only discards deck cards:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c_latias_bench_override.inc#L72-L108
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  expect(!sim::EngineTestAccess::play_professor_burnet(fixture.engine),
         "Burnet should be held when no deterministic same-turn Energy route remains");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.hand == before.hand, "Burnet should remain held");
  expect(after.deck == before.deck, "the payload deck should remain unchanged");
  expect(after.discard.empty(), "no payload or Supporter should enter discard");
  expect(!after.supporter_used, "the Supporter slot should remain available");
}

void test_rejects_ci_item_lock_trace_state() {
  Fixture fixture({"burnet-ci-item-lock-state", sim::DciProfile::StrictJit,
                   sim::LockMode::TurnTwoItem, false, 4});
  sim::State state = base_state();
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 2, 1, 1}};
  state.manual_energy_used = false;
  const sim::State before = state;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // CI seed 17 reached this structure: Oricorio Active, a GF Regidrago VSTAR on the
  // Bench, no needed Energy in hand, and Burnet available. Burnet cannot finish the
  // Energy or Active-position axis, so its strict-JIT payload would expire:
  // https://github.com/FlareZ123/pokemon-sims/actions/runs/29477535274
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  expect(!sim::EngineTestAccess::play_professor_burnet(fixture.engine),
         "Burnet should be held in the CI-derived incomplete Item-lock state");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.hand == before.hand, "Burnet should remain held in the CI state");
  expect(after.deck == before.deck, "the CI-state payload deck should remain unchanged");
  expect(after.discard.empty(), "the CI state should not discard a payload");
  expect(!after.supporter_used, "the CI state should preserve the Supporter slot");
}

void test_allows_burnet_when_energy_is_already_complete() {
  Fixture fixture({"burnet-energy-complete", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = base_state();
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // With Active GGF already complete, Professor Burnet immediately establishes the
  // current-turn Dragon payload required by Apex Dragon:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  expect(sim::EngineTestAccess::play_professor_burnet(fixture.engine),
         "Burnet should remain live when every non-payload axis is complete");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.supporter_used, "Burnet should consume the Supporter play");
  expect(count(after.discard, sim::Card::ProfessorBurnet) == 1,
         "Burnet should enter discard");
  expect(count(after.discard, sim::Card::MegaDragonite) == 1,
         "the first modeled payload should enter discard");
  expect(count(after.discard, sim::Card::Dragapult) == 1,
         "the second modeled payload should enter discard");
}

void test_rejects_burnet_when_promotion_requires_tate() {
  Fixture fixture({"burnet-blocks-tate-promotion", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = base_state();
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1}};
  state.hand.push_back(sim::Card::TateLiza);
  const sim::State before = state;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Burnet and Tate & Liza compete for the one Supporter play. Without an existing
  // Latias ex retreat route, Burnet would strand the ready VSTAR on the Bench:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  expect(!sim::EngineTestAccess::play_professor_burnet(fixture.engine),
         "Burnet should not consume the required Tate promotion slot");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.hand == before.hand, "both competing Supporters should remain held");
  expect(after.deck == before.deck, "the payload deck should remain unchanged");
  expect(!after.supporter_used, "Tate's Supporter slot should remain open");
}

void test_allows_burnet_with_existing_latias_promotion() {
  Fixture fixture({"burnet-latias-promotion", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = base_state();
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1}};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Latias ex gives the Basic Active no Retreat Cost, so Burnet may establish the
  // payload while the later free retreat preserves same-turn VSTAR promotion:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::play_professor_burnet(fixture.engine),
         "Burnet should remain live with an existing Latias promotion route");
}

void test_allows_burnet_before_active_regi_evolves() {
  Fixture fixture({"burnet-active-regi-evolution", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = base_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1}};
  state.hand.push_back(sim::Card::RegidragoVstar);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // A prior-turn Active Regidrago V may evolve from the held VSTAR later in the same
  // turn, so the existing Benched VSTAR does not make Burnet's active-position route dead:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  expect(sim::EngineTestAccess::play_professor_burnet(fixture.engine),
         "Burnet should remain live before a legal Active Regidrago evolution");
}

void test_no_discard_control_keeps_payload_banking() {
  Fixture fixture({"burnet-no-discard-control", sim::DciProfile::NoDiscardControl,
                   sim::LockMode::None, false, 4});
  sim::State state = base_state();
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0}};
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // No-discard-control explicitly permits payload banking before the complete ready
  // turn, so the strict-JIT timing guard must not change that profile:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  expect(sim::EngineTestAccess::play_professor_burnet(fixture.engine),
         "no-discard-control should retain Burnet payload banking");
}

}  // namespace

int main() {
  try {
    test_rejects_burnet_after_manual_attachment_window_closes();
    test_rejects_burnet_when_no_needed_energy_is_held();
    test_rejects_ci_item_lock_trace_state();
    test_allows_burnet_when_energy_is_already_complete();
    test_rejects_burnet_when_promotion_requires_tate();
    test_allows_burnet_with_existing_latias_promotion();
    test_allows_burnet_before_active_regi_evolves();
    test_no_discard_control_keeps_payload_banking();
    std::cout << "Professor Burnet strict-JIT timing tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Professor Burnet strict-JIT timing test failed: "
              << error.what() << '\n';
    return 1;
  }
}
