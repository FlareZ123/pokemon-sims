#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_klara(Engine& engine) { return engine.play_klara_recovery(); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static bool ready(const Engine& engine) {
    return engine.active_is_vstar() && engine.state_.active->grass >= 2 &&
           engine.state_.active->fire >= 1 && engine.payload_ready();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::DeckRecipe crobat_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::CrobatV, 1);
  return recipe;
}

sim::State exact_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Klara, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass};
  state.discard = {sim::Card::CrobatV};
  // Vector back is the next draw in this simulator. The policy may use only the
  // public K1 composition, while this exact outcome witness places Blender on top:
  // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
  // Crobat V and Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR and Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Bench, Ability, Supporter, Item, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Public-composition boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Crobat connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#crobat-v-draw-connector-policy
  // Prior narrower fix: https://github.com/FlareZ123/pokemon-sims/issues/1965
  // Confirmed primary-connector bug: https://github.com/FlareZ123/pokemon-sims/issues/1978
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::GoodraVstar, sim::Card::DialgaGX,
                sim::Card::BrilliantBlender};
  state.vstar_power_used = true;
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const sim::LockMode lock, const std::uint64_t seed,
          sim::State state, const bool deck_seen = true,
          const bool prizes_revealed = false)
      : scenario{"issue-1978", sim::DciProfile::StrictJit, lock, false, 4},
        rng(seed),
        engine(scenario, crobat_recipe(), rng) {
    sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen,
                                     prizes_revealed);
  }
};

void expect_klara_rejected(Fixture& fixture, const char* message) {
  expect(!sim::EngineTestAccess::play_klara(fixture.engine), message);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.discard, sim::Card::CrobatV) &&
             contains(after.hand, sim::Card::Klara),
         "Rejected Klara mutated the Crobat or Supporter zones.");
}

void test_primary_crobat_connector_reaches_exact_ready_turn() {
  Fixture fixture(sim::LockMode::None, 1978, exact_state());
  sim::EngineTestAccess::run_turn(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(sim::EngineTestAccess::ready(fixture.engine),
         "Primary Klara-Crobat-Blender route did not reach T3 readiness.");
  expect(after.dark_asset_used &&
             std::any_of(after.bench.begin(), after.bench.end(),
                         [](const sim::Pokemon& pokemon) {
                           return pokemon.card == sim::Card::CrobatV;
                         }),
         "Klara did not recover and Bench the primary Crobat connector.");
  expect(contains(after.discard, sim::Card::BrilliantBlender) &&
             std::any_of(after.discarded_this_turn.begin(),
                         after.discarded_this_turn.end(), sim::is_payload),
         "Dark Asset did not expose the exact Blender payload completion.");
}

void test_identical_post_klara_state_completes_downstream_route() {
  sim::State state = exact_state();
  state.hand.erase(state.hand.begin());
  state.hand.push_back(sim::Card::CrobatV);
  state.discard = {sim::Card::Klara};
  state.supporter_used = true;
  Fixture fixture(sim::LockMode::None, 1979, std::move(state));
  sim::EngineTestAccess::run_turn(fixture.engine);
  expect(sim::EngineTestAccess::ready(fixture.engine),
         "The legal post-Klara Crobat state did not complete downstream.");
}

void test_prize_inspection_k1_admits_public_composition() {
  Fixture fixture(sim::LockMode::None, 1980, exact_state(), false, true);
  sim::EngineTestAccess::run_turn(fixture.engine);
  expect(sim::EngineTestAccess::ready(fixture.engine),
         "Prize-inspection K1 did not admit the primary Crobat route.");
}

void test_true_k0_rejects_hidden_composition() {
  Fixture fixture(sim::LockMode::None, 1981, exact_state(), false, false);
  expect_klara_rejected(fixture,
      "True K0 must not use hidden deck composition to justify Crobat.");
}

void test_used_dark_asset_rejects_primary_crobat() {
  sim::State state = exact_state();
  state.dark_asset_used = true;
  Fixture fixture(sim::LockMode::None, 1982, std::move(state));
  expect_klara_rejected(fixture,
      "Used Dark Asset must reject the primary Crobat route.");
}

void test_rule_box_lock_rejects_primary_crobat() {
  Fixture fixture(sim::LockMode::FullRuleBoxAbility, 1983, exact_state());
  expect_klara_rejected(fixture,
      "Rule Box Ability lock must reject the primary Crobat route.");
}

void test_item_lock_rejects_blender_only_value() {
  Fixture fixture(sim::LockMode::FullItem, 1984, exact_state());
  expect_klara_rejected(fixture,
      "Item lock must reject a Blender-only Crobat draw route.");
}

void test_full_bench_rejects_primary_crobat() {
  sim::State state = exact_state();
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1},
      sim::Pokemon{sim::Card::RegidragoV, 1},
      sim::Pokemon{sim::Card::Oricorio, 1},
      sim::Pokemon{sim::Card::TapuLeleGX, 1},
      sim::Pokemon{sim::Card::LatiasEx, 1},
  };
  Fixture fixture(sim::LockMode::None, 1985, std::move(state));
  expect_klara_rejected(fixture,
      "A full Bench must reject the primary Crobat route.");
}

void test_empty_deck_rejects_primary_crobat() {
  sim::State state = exact_state();
  state.deck.clear();
  Fixture fixture(sim::LockMode::None, 1986, std::move(state));
  expect_klara_rejected(fixture,
      "An empty deck must reject the primary Crobat route.");
}

void test_six_card_post_bench_hand_rejects_primary_crobat() {
  sim::State state = exact_state();
  state.hand.push_back(sim::Card::Grass);
  Fixture fixture(sim::LockMode::None, 1987, std::move(state));
  expect_klara_rejected(fixture,
      "A six-card post-Bench hand must reject zero-card Dark Asset.");
}

void test_absent_crobat_rejects_primary_route() {
  sim::State state = exact_state();
  state.discard.clear();
  Fixture fixture(sim::LockMode::None, 1988, std::move(state));
  expect(!sim::EngineTestAccess::play_klara(fixture.engine),
         "Absent Crobat must reject the primary recovery route.");
}

void test_public_deck_without_axis_value_rejects_crobat() {
  sim::State state = exact_state();
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::GoodraVstar, sim::Card::DialgaGX,
                sim::Card::Appletun};
  Fixture fixture(sim::LockMode::None, 1989, std::move(state));
  expect_klara_rejected(fixture,
      "Public payloads without a legal discard outlet must not justify Crobat.");
}
}  // namespace

int main() {
  try {
    test_primary_crobat_connector_reaches_exact_ready_turn();
    test_identical_post_klara_state_completes_downstream_route();
    test_prize_inspection_k1_admits_public_composition();
    test_true_k0_rejects_hidden_composition();
    test_used_dark_asset_rejects_primary_crobat();
    test_rule_box_lock_rejects_primary_crobat();
    test_item_lock_rejects_blender_only_value();
    test_full_bench_rejects_primary_crobat();
    test_empty_deck_rejects_primary_crobat();
    test_six_card_post_bench_hand_rejects_primary_crobat();
    test_absent_crobat_rejects_primary_route();
    test_public_deck_without_axis_value_rejects_crobat();
    std::cout << "Issue 1978 Klara primary Crobat tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
