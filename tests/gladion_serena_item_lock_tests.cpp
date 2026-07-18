#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_known_prizes_do_not_preempt_serena_payload_under_item_lock() {
  // Serena's draw mode requires at least one discard; the payload is that cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // The player may play one Supporter per turn: https://www.pokemon.com/us/pokemon-tcg/rules
  sim::Scenario scenario{"gladion-serena-item-lock", sim::DciProfile::StrictJit, sim::LockMode::FullItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{59};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Serena, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Lusamine, sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin,
                  sim::Card::MawileGX, sim::Card::Guzma};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used, "Serena must use the one available Supporter play");
  expect(contains(after.hand, sim::Card::Gladion), "Gladion must be preserved when Serena directly completes payload timing");
  expect(contains(after.discard, sim::Card::Serena), "Serena must enter discard after being played");
  expect(contains(after.discard, sim::Card::MegaDragonite), "Serena must discard the modeled Dragon payload");
  expect(contains(after.discarded_this_turn, sim::Card::MegaDragonite), "the payload must be marked as discarded this turn");
  expect(sim::EngineTestAccess::payload_ready(engine), "the Serena discard must satisfy strict current-turn payload timing");
}

void test_live_crispin_preempts_dead_burnet_gladion_exchange() {
  sim::Scenario scenario{"issue-840-live-crispin", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{84001};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Crispin,
                sim::Card::ProfessorBurnet, sim::Card::RegidragoVstar};
  state.prizes = {sim::Card::Lusamine, sim::Card::Grass, sim::Card::Dipplin,
                  sim::Card::MawileGX, sim::Card::Guzma, sim::Card::Powerglass};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Crispin searches different Basic Energy types, attaches the missing Fire, and
  // preserves Professor Burnet as the later payload floor. Gladion cannot improve
  // this known Energy axis and cannot consume the single Supporter play first:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/840
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used, "Crispin must use the available Supporter play");
  expect(after.active.has_value(), "the Regidrago V target must remain Active");
  expect(after.active->grass == 2 && after.active->fire == 1,
         "Crispin must complete GGF before evolution and Legacy Star");
  expect(contains(after.discard, sim::Card::Crispin), "Crispin must enter discard after play");
  expect(contains(after.hand, sim::Card::Gladion), "Gladion must remain held when it advances no known axis");
  expect(contains(after.hand, sim::Card::ProfessorBurnet), "Professor Burnet must remain as the payload floor");
  expect(contains(after.hand, sim::Card::RegidragoVstar), "the held VSTAR route must remain intact");
}

void test_live_crispin_does_not_block_known_prized_vstar() {
  sim::Scenario scenario{"issue-840-prized-vstar", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{84002};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Crispin, sim::Card::ProfessorBurnet};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Dipplin,
                  sim::Card::MawileGX, sim::Card::Guzma, sim::Card::Powerglass};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // A known prized Regidrago VSTAR is the immediate missing evolution axis. Gladion
  // may recover it even while Crispin remains a live Energy route:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/840
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used, "Gladion must recover the known prized VSTAR");
  expect(contains(after.hand, sim::Card::RegidragoVstar), "the prized VSTAR must move to hand");
  expect(contains(after.prizes, sim::Card::Gladion), "Gladion must replace the recovered Prize card");
  expect(contains(after.hand, sim::Card::Crispin), "Crispin must remain held after the critical exchange");
  expect(contains(after.hand, sim::Card::ProfessorBurnet), "Professor Burnet must remain held after the exchange");
}

void test_dead_crispin_preserves_critical_prized_energy_gladion() {
  sim::Scenario scenario{"issue-840-prized-fire", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{84003};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Crispin,
                sim::Card::ProfessorBurnet, sim::Card::RegidragoVstar};
  state.prizes = {sim::Card::Fire, sim::Card::Lusamine, sim::Card::Dipplin,
                  sim::Card::MawileGX, sim::Card::Guzma, sim::Card::Powerglass};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Crispin cannot supply the missing Fire after K1 proves no Basic Energy remains.
  // Gladion must retain access to that known prized Energy axis:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/840
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used, "Gladion must recover the known prized Fire Energy");
  expect(contains(after.hand, sim::Card::Fire), "the prized Fire Energy must move to hand");
  expect(contains(after.prizes, sim::Card::Gladion), "Gladion must replace the recovered Fire Energy");
  expect(contains(after.hand, sim::Card::Crispin), "the dead Crispin must remain held");
  expect(contains(after.hand, sim::Card::ProfessorBurnet), "the hidden Burnet must be restored exactly");
}

}  // namespace

int main() {
  try {
    test_known_prizes_do_not_preempt_serena_payload_under_item_lock();
    test_live_crispin_preempts_dead_burnet_gladion_exchange();
    test_live_crispin_does_not_block_known_prized_vstar();
    test_dead_crispin_preserves_critical_prized_energy_gladion();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
