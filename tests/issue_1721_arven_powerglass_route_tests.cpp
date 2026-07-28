#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void establish_k1(Engine& engine) { engine.deck_seen_ = true; }
  static void reset_k0(Engine& engine) { engine.deck_seen_ = false; }
  static bool route(const Engine& engine) {
    return engine.issue_1721_arven_powerglass_route_available();
  }
  static long double probability(const Engine& engine) {
    return engine.issue_1721_t2_payload_probability_after_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void install_exact_k1_state(sim::Engine& engine) {
  auto& state = sim::EngineTestAccess::state(engine);
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Arven, sim::Card::RegidragoVstar,
                sim::Card::QuickBall, sim::Card::FieldBlower,
                sim::Card::GoodraVstar, sim::Card::MegaDragonite};
  state.deck = {
      sim::Card::EarthenVessel, sim::Card::Powerglass, sim::Card::Oricorio,
      sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::QuickBall, sim::Card::QuickBall,
      sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure,
      sim::Card::BrilliantBlender, sim::Card::ProfessorBurnet,
      sim::Card::Serena,
      sim::Card::Dragapult, sim::Card::Dragapult,
      sim::Card::MegaDragonite, sim::Card::DialgaGX};
  state.deck.insert(state.deck.end(), 27, sim::Card::Lusamine);
  sim::EngineTestAccess::establish_k1(engine);
}

template <typename Mutate>
bool route_with(const sim::LockMode locks, const bool going_first,
                const bool establish_k1, Mutate mutate) {
  sim::Scenario scenario{"issue-1721-unit", sim::DciProfile::StrictJit,
                         locks, going_first, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1721};
  sim::Engine engine(scenario, recipe, rng);
  install_exact_k1_state(engine);
  if (!establish_k1) sim::EngineTestAccess::reset_k0(engine);
  mutate(sim::EngineTestAccess::state(engine));
  return sim::EngineTestAccess::route(engine);
}

void test_probability_contract() {
  sim::Scenario scenario{"issue-1721-probability", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1721};
  sim::Engine engine(scenario, recipe, rng);
  install_exact_k1_state(engine);

  // The corrected proof conditions separately on a direct outlet, a Dragon
  // payload, and an irrelevant T2 draw:
  // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Repository future-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Corrected proof and confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1721
  constexpr long double expected = 700597.0L / 740962.0L;
  expect(std::fabs(sim::EngineTestAccess::probability(engine) - expected) <
             1.0e-15L,
         "The conditional 94.552352% route proof changed.");
  expect(sim::EngineTestAccess::route(engine),
         "The exact K1 Arven-Powerglass route was rejected.");
}

void test_negative_controls() {
  const auto no_change = [](sim::State&) {};
  expect(!route_with(sim::LockMode::None, false, false, no_change),
         "The route used K0 deck composition as K1 knowledge.");
  expect(!route_with(sim::LockMode::FullItem, false, true, no_change),
         "The route ignored Item lock.");
  expect(!route_with(sim::LockMode::FullSupporter, false, true, no_change),
         "The route ignored Supporter lock.");
  expect(!route_with(sim::LockMode::FullRuleBoxAbility, false, true, no_change),
         "The route ignored Rule Box Ability lock.");
  expect(!route_with(sim::LockMode::None, true, true, no_change),
         "The route ran while going first.");
  expect(!route_with(sim::LockMode::None, false, true, [](sim::State& state) {
           state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                                      sim::Card::RegidragoVstar));
         }), "The route invented a held Regidrago VSTAR.");
  expect(!route_with(sim::LockMode::None, false, true, [](sim::State& state) {
           state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                                      sim::Card::QuickBall));
         }), "The route invented a held Quick Ball.");
  expect(!route_with(sim::LockMode::None, false, true, [](sim::State& state) {
           state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                                      sim::Card::EarthenVessel));
         }), "The route invented Earthen Vessel.");
  expect(!route_with(sim::LockMode::None, false, true, [](sim::State& state) {
           state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                                      sim::Card::Powerglass));
         }), "The route invented Powerglass.");
  expect(!route_with(sim::LockMode::None, false, true, [](sim::State& state) {
           state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                                      sim::Card::Oricorio));
         }), "The route invented Oricorio.");
  expect(!route_with(sim::LockMode::None, false, true, [](sim::State& state) {
           state.active->tool = sim::Tool::ForestSealStone;
         }), "The route ignored an occupied Tool slot.");
  expect(!route_with(sim::LockMode::None, false, true, [](sim::State& state) {
           state.bench.insert(state.bench.end(), 5,
                              sim::Pokemon{sim::Card::CrobatV, 0, 0, 0,
                                           sim::Tool::None});
         }), "The route ignored a full Bench.");
  expect(!route_with(sim::LockMode::None, false, true, [](sim::State& state) {
           const auto grass = std::find(state.deck.begin(), state.deck.end(),
                                        sim::Card::Grass);
           state.deck.erase(grass);
         }), "The route ignored insufficient Grass Energy.");
  expect(!route_with(sim::LockMode::None, false, true, [](sim::State& state) {
           state.stadium = sim::Stadium::ChaoticSwell;
         }), "The route spent Field Blower while a Stadium target remained.");
  expect(!route_with(sim::LockMode::None, false, true, [](sim::State& state) {
           const auto outlet = std::find(state.deck.begin(), state.deck.end(),
                                         sim::Card::Serena);
           state.deck.erase(outlet);
           state.deck.push_back(sim::Card::Lusamine);
         }), "The route ignored a below-threshold T2 payload probability.");
}

void test_seed_31415_reaches_t2() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck =
      sim::crobat_modeling_deck_by_id("crobat2-erika-channeler");
  expect(scenario.has_value() && deck != nullptr,
         "Issue-1721 seed fixture unavailable.");
  std::mt19937_64 rng{31415};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Full legal line and direct sources:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official action order: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1721
  expect(outcome.first_ready_turn == 2,
         "Seed 31415 did not improve from T3 to T2 readiness.");
  expect(has(trace, "Arven searched Earthen Vessel and Powerglass") &&
             has(trace, "Discarded setup-dead Field Blower") &&
             has(trace, "searched Oricorio") &&
             has(trace, "Searched two Grass Energy") &&
             has(trace, "Held Celestial Roar") &&
             has(trace, "T1 | POWERGLASS") && has(trace, "T2 | READY"),
         "Seed 31415 omitted a required route action.");
}
}  // namespace

int main() {
  test_probability_contract();
  test_negative_controls();
  test_seed_31415_reaches_t2();
}
