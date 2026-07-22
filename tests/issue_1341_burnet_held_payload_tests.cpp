#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State burnet_state(const bool include_held_payload) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet,
                sim::Card::MysteriousTreasure, sim::Card::Grass};
  if (include_held_payload) state.hand.push_back(sim::Card::MegaDragonite);
  state.deck = {sim::Card::Dragapult, sim::Card::DialgaGX,
                sim::Card::TapuLeleGX, sim::Card::RegidragoV};
  return state;
}

void test_held_payload_item_preserves_burnet() {
  const sim::Scenario scenario{"issue-1341-held-outlet",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1341};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, burnet_state(true));

  // Mysterious Treasure can discard the public Mega Dragonite ex and complete the
  // payload axis. Burnet must preserve the one Supporter play and deck payloads:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1341
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(!result.supporter_used,
         "The direct held-payload Item route must preserve the Supporter play.");
  expect(contains(result.hand, sim::Card::ProfessorBurnet),
         "Professor Burnet must remain in hand.");
  expect(contains(result.deck, sim::Card::Dragapult) &&
             contains(result.deck, sim::Card::DialgaGX),
         "Burnet must not remove deck payloads when the direct Item route is live.");
}

void test_payable_ultra_ball_preserves_burnet() {
  const sim::Scenario scenario{"issue-1341-ultra-outlet",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1342};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::UltraBall,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball can discard the held Dragon plus a distinct card and still search a
  // legal Pokémon. That direct Item route preserves Professor Burnet:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/issues/1341
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(!result.supporter_used,
         "A payable held-payload Ultra Ball must preserve the Supporter play.");
  expect(contains(result.hand, sim::Card::ProfessorBurnet),
         "Professor Burnet must remain held for the Ultra Ball route.");
}

void test_serena_uses_held_payload_instead_of_burnet() {
  const sim::Scenario scenario{"issue-1341-serena-outlet",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1343};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::Serena,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::RegidragoV,
                sim::Card::Dragapult, sim::Card::DialgaGX,
                sim::Card::Fire, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Serena can use its mandatory discard on the held Dragon and complete strict JIT.
  // Burnet must remain out of the discard pile while Serena uses the Supporter slot:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/888
  // https://github.com/FlareZ123/pokemon-sims/issues/1341
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.supporter_used, "Serena must use the Supporter play.");
  expect(contains(result.discard, sim::Card::Serena),
         "Serena must be the Supporter placed in discard.");
  expect(contains(result.discard, sim::Card::MegaDragonite),
         "Serena must discard the held Dragon payload.");
  expect(!contains(result.discard, sim::Card::ProfessorBurnet),
         "Professor Burnet must not be consumed before Serena.");
}

void test_no_held_payload_preserves_burnet_route() {
  const sim::Scenario scenario{"issue-1341-no-held-payload",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1344};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, burnet_state(false));

  // Without a public payload, Burnet remains the deterministic deck-to-discard route:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://github.com/FlareZ123/pokemon-sims/issues/1341
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.supporter_used,
         "Professor Burnet must remain live when no payload is held.");
  expect(contains(result.discard, sim::Card::ProfessorBurnet),
         "The positive control must play Professor Burnet.");
}

void test_item_lock_preserves_burnet_route() {
  const sim::Scenario scenario{"issue-1341-item-lock",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  std::mt19937_64 rng{1345};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, burnet_state(true));

  // Item lock blocks Mysterious Treasure while Burnet remains legal. The Supporter
  // must still provide the only live current-turn payload route:
  // https://api.pokemontcg.io/v2/cards/me2pt5-16
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/issues/1341
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.supporter_used,
         "Item lock must not suppress the legal Burnet payload route.");
  expect(contains(result.discard, sim::Card::ProfessorBurnet),
         "Professor Burnet must be played under Item lock.");
}

void test_known_dead_quick_ball_does_not_suppress_burnet() {
  const sim::Scenario scenario{"issue-1341-dead-outlet",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1346};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::QuickBall,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::Dragapult, sim::Card::GoodraVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A K1 Quick Ball with no Basic target cannot be played merely to discard a
  // payload. The target-dead Item must not suppress Burnet:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/1341
  sim::EngineTestAccess::choose_supporter(engine);
  expect(sim::EngineTestAccess::state(engine).supporter_used,
         "A known-dead Quick Ball must not suppress Professor Burnet.");
}

void test_unpayable_ultra_ball_does_not_suppress_burnet() {
  const sim::Scenario scenario{"issue-1341-unpayable-ultra",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1347};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::UltraBall,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::RegidragoV,
                sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball requires two other cards. With only the held Dragon available, it is
  // unpayable and cannot replace Professor Burnet as the payload route:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/issues/1341
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.supporter_used,
         "An unpayable Ultra Ball must not suppress Professor Burnet.");
  expect(contains(result.discard, sim::Card::ProfessorBurnet),
         "Burnet must remain the live route when Ultra Ball lacks its second cost.");
}

}  // namespace

int main() {
  try {
    test_held_payload_item_preserves_burnet();
    test_payable_ultra_ball_preserves_burnet();
    test_serena_uses_held_payload_instead_of_burnet();
    test_no_held_payload_preserves_burnet_route();
    test_item_lock_preserves_burnet_route();
    test_known_dead_quick_ball_does_not_suppress_burnet();
    test_unpayable_ultra_ball_does_not_suppress_burnet();
    std::cout << "Issue 1341 Burnet held-payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
