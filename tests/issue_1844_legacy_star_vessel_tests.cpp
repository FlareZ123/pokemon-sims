#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool future_vessel_route_available(Engine& engine) {
    return engine.issue_1844_future_vessel_route_available_before_legacy();
  }
  static bool use_legacy_star(Engine& engine) {
    return engine.use_legacy_star_issue_1417();
  }
  static bool play_earthen_vessel(Engine& engine) {
    return engine.play_earthen_vessel(true);
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool payload_ready(Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

std::vector<sim::Card> delayed_vessel_deck() {
  // Legacy Star removes cards from the vector back. Grass remains in the inspected
  // deck, while one Dragon payload, Earthen Vessel, and five inert cards enter
  // discard. This leaves Vessel a legal K1 search route for the following turn:
  // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
  return {sim::Card::Grass, sim::Card::ChaoticSwell,
          sim::Card::PathToPeak, sim::Card::ErikasInvitation,
          sim::Card::Guzma, sim::Card::Channeler,
          sim::Card::EarthenVessel, sim::Card::GoodraVstar};
}

std::vector<sim::Card> exhausted_grass_deck() {
  // The sole Grass is inside the seven cards discarded by Legacy Star, so the
  // post-discard K1 deck has no legal Earthen Vessel Energy target:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Trainer no-effect ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
  return {sim::Card::ChaoticSwell, sim::Card::PathToPeak,
          sim::Card::ErikasInvitation, sim::Card::Guzma,
          sim::Card::Channeler, sim::Card::EarthenVessel,
          sim::Card::Grass, sim::Card::GoodraVstar};
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode locks = sim::LockMode::None)
      : scenario{"issue-1844", sim::DciProfile::StrictJit, locks, true, 4},
        recipe(sim::baseline_recipe()), rng(1844), engine(scenario, recipe, rng) {}
};

void seed_delayed_vessel_state(sim::Engine& engine) {
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.manual_energy_used = true;
  state.hand = {sim::Card::MegaDragonite};
  state.deck = delayed_vessel_deck();
  sim::EngineTestAccess::set_deck_seen(engine);
}

void test_recovers_holds_and_uses_vessel_on_t3() {
  Fixture fixture;
  sim::Engine& engine = fixture.engine;
  seed_delayed_vessel_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);

  // Legacy Star may recover Earthen Vessel after its top-seven discard. The prior
  // manual attachment is already spent, so Vessel stays held until T3. It then
  // discards Mega Dragonite ex as the current-turn Dragon payload, searches the
  // K1-proven Grass, and the T3 manual attachment completes Active GGF:
  // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Official Item, discard-cost, search, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Strict-JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
  if (!sim::EngineTestAccess::future_vessel_route_available(engine) ||
      !sim::EngineTestAccess::use_legacy_star(engine) ||
      !contains(state.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error(
        "Legacy Star should recover the deterministic next-turn Earthen Vessel route.");
  }
  if (sim::EngineTestAccess::play_earthen_vessel(engine)) {
    throw std::runtime_error(
        "The recovered Vessel must remain held during the Legacy Star turn.");
  }

  state.turn = 3;
  state.manual_energy_used = false;
  state.discarded_this_turn.clear();
  if (!sim::EngineTestAccess::play_earthen_vessel(engine) ||
      !contains(state.discarded_this_turn, sim::Card::MegaDragonite) ||
      !contains(state.hand, sim::Card::Grass) ||
      !sim::EngineTestAccess::attach_manual(engine) || !state.active ||
      state.active->grass != 2 || state.active->fire != 1 ||
      !sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error(
        "The held Vessel route must discard the T3 Dragon, search Grass, and complete GGF.");
  }
}

void test_rejects_route_under_item_lock() {
  Fixture fixture(sim::LockMode::FullItem);
  sim::Engine& engine = fixture.engine;
  seed_delayed_vessel_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);

  // Full Item lock makes a recovered Earthen Vessel unusable:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Repository lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-model
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
  if (!sim::EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The item-lock control should still resolve Legacy Star.");
  }
  if (contains(state.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error("Legacy Star must not recover an Item-locked future Vessel.");
  }
}

void test_rejects_route_without_held_payload() {
  Fixture fixture;
  sim::Engine& engine = fixture.engine;
  seed_delayed_vessel_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.hand.clear();

  // Earthen Vessel requires another hand card, and strict JIT specifically requires
  // that cost to be a permitted Dragon during the ready turn:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
  if (!sim::EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The missing-payload control should still resolve Legacy Star.");
  }
  if (contains(state.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error("Legacy Star must not recover Vessel without a held Dragon cost.");
  }
}

void test_rejects_route_when_two_attachments_are_missing() {
  Fixture fixture;
  sim::Engine& engine = fixture.engine;
  seed_delayed_vessel_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1,
                              sim::Tool::None};

  // One future manual attachment cannot pay two missing Grass requirements:
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
  if (!sim::EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The two-attachment control should still resolve Legacy Star.");
  }
  if (contains(state.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error(
        "Legacy Star must not recover a one-turn Vessel line when two attachments remain.");
  }
}

void test_rejects_route_when_legacy_star_exhausts_energy_target() {
  Fixture fixture;
  sim::Engine& engine = fixture.engine;
  seed_delayed_vessel_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.deck = exhausted_grass_deck();

  // K1 target legality is checked again after Legacy Star discards the top seven.
  // When no Grass remains, Vessel cannot promise the next-turn attachment:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
  if (!sim::EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The exhausted-target control should still resolve Legacy Star.");
  }
  if (contains(state.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error(
        "Legacy Star must not recover Vessel after its exact Energy target is exhausted.");
  }
}

void test_rejects_route_with_unresolved_active_position() {
  Fixture fixture;
  sim::Engine& engine = fixture.engine;
  seed_delayed_vessel_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None}};

  // The setup contract requires the powered Regidrago VSTAR to be Active. A future
  // Vessel attachment alone does not solve an unpaid Active-position axis:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earliest ready-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
  if (!sim::EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The Active-position control should still resolve Legacy Star.");
  }
  if (contains(state.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error(
        "Legacy Star must not recover Vessel while the Active-position axis is unresolved.");
  }
}

void test_exact_seed_314159_reaches_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("The registered seed-314159 fixture is unavailable.");
  }

  std::mt19937_64 rng{314159};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The exact K1 route must recover Vessel on T2, hold it, discard a held Dragon
  // with Vessel on T3, attach the searched Grass, and reach strict-JIT readiness:
  // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
  if (outcome.first_ready_turn != 3) {
    throw std::runtime_error("The corrected seed-314159 route did not reach T3.");
  }
  const auto contains_line = [&trace](const std::string& needle) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&needle](const std::string& line) {
                         return line.find(needle) != std::string::npos;
                       });
  };
  if (!contains_line(
          "Recovered Earthen Vessel for the next-turn strict-JIT Dragon payload") ||
      !contains_line("T3 | Earthen Vessel") ||
      !(contains_line("Dragapult ex (Earthen Vessel cost)") ||
        contains_line("Mega Dragonite ex (Earthen Vessel cost)")) ||
      !contains_line("T3 | READY")) {
    throw std::runtime_error(
        "The exact seed-314159 trace omitted the delayed Vessel continuation.");
  }
}

}  // namespace

int main() {
  try {
    test_recovers_holds_and_uses_vessel_on_t3();
    test_rejects_route_under_item_lock();
    test_rejects_route_without_held_payload();
    test_rejects_route_when_two_attachments_are_missing();
    test_rejects_route_when_legacy_star_exhausts_energy_target();
    test_rejects_route_with_unresolved_active_position();
    test_exact_seed_314159_reaches_t3();
    std::cout << "issue 1844 delayed Legacy Star Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
