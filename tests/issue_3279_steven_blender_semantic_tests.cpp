#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool available(const Engine& engine) {
    return engine.issue_1798_steven_blender_route_available();
  }
  static bool start(Engine& engine) {
    return engine.start_issue_1798_steven_blender_route();
  }
  static bool finish_available(const Engine& engine) {
    return engine.issue_1798_blender_finish_available();
  }
  static bool finish(Engine& engine) {
    return engine.complete_issue_1798_steven_blender_route();
  }
  static State& state(Engine& engine) { return engine.state_; }
  static void set_finish_turn(Engine& engine, const int turn) {
    engine.issue_1798_blender_turn_ = turn;
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::DciProfile dci = sim::DciProfile::StrictJit,
                       const sim::LockMode lock = sim::LockMode::None,
                       const bool going_first = true,
                       const int max_turn = 5) {
  return sim::Scenario{"issue-3279", dci, lock, going_first, max_turn};
}

sim::State bank_state(const int turn = 3) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Fire, sim::Card::StevensResolve,
                sim::Card::BrilliantBlender, sim::Card::Dragapult,
                sim::Card::Powerglass};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::MegaDragonite, sim::Card::QuickBall,
                sim::Card::ErikasInvitation};
  state.prizes = {sim::Card::Grass, sim::Card::GoodraVstar,
                  sim::Card::MysteriousTreasure, sim::Card::ProfessorBurnet,
                  sim::Card::DialgaGX, sim::Card::FieldBlower};
  return state;
}

sim::State finish_state(const int turn = 4) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 2, 1, 2,
                              sim::Tool::None};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::BrilliantBlender, sim::Card::Dragapult};
  state.deck = {sim::Card::MegaDragonite, sim::Card::QuickBall};
  state.prizes = {sim::Card::Grass, sim::Card::GoodraVstar,
                  sim::Card::MysteriousTreasure, sim::Card::ProfessorBurnet,
                  sim::Card::DialgaGX, sim::Card::FieldBlower};
  return state;
}

bool available_for(const sim::Scenario& selected, sim::State state,
                   const bool k1 = true) {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3279);
  sim::Engine engine(selected, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), k1);
  return sim::EngineTestAccess::available(engine);
}

void semantic_bank_and_finish_parity() {
  // These actions are Steven's Resolve now, then evolution, manual attachment, and
  // Brilliant Blender on the recorded next turn. Seat and Rule Box Ability lock do
  // not change their legality when the current Supporter and projected Item windows
  // are open. Both same-ready-turn JIT profiles use the same Dragon timing contract.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // JIT / lock / priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original / cross-profile / semantic bugs: https://github.com/FlareZ123/pokemon-sims/issues/1798 https://github.com/FlareZ123/pokemon-sims/issues/2764 https://github.com/FlareZ123/pokemon-sims/issues/3279
  expect(available_for(scenario(sim::DciProfile::StrictJit,
                                sim::LockMode::None, false, 4),
                       bank_state(3)),
         "Going-second public state hid the legal bank route");
  expect(available_for(scenario(sim::DciProfile::MatchupFlexJit,
                                sim::LockMode::FullRuleBoxAbility, true, 5),
                       bank_state(4)),
         "Later MatchupFlex Rule Box-lock state hid the legal bank route");

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3279);
  const sim::Scenario selected = scenario(sim::DciProfile::MatchupFlexJit,
                                          sim::LockMode::FullRuleBoxAbility,
                                          false, 5);
  sim::Engine engine(selected, recipe, rng);
  sim::EngineTestAccess::set_state(engine, bank_state(4));
  expect(sim::EngineTestAccess::start(engine),
         "Relative later bank route did not start");
  sim::State& after = sim::EngineTestAccess::state(engine);
  after.turn = 5;
  after.turn_ended = false;
  after.supporter_used = false;
  after.manual_energy_used = false;
  after.discarded_this_turn.clear();
  expect(sim::EngineTestAccess::finish_available(engine),
         "Recorded Rule Box-lock finish became unavailable");
  expect(sim::EngineTestAccess::finish(engine),
         "Recorded later Blender route did not finish");
}

void semantic_negative_controls() {
  // Brilliant Blender is an Item, so persistent/full Item locks on the projected
  // finish turn block the bank. Steven remains subject to current Supporter lock.
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3279
  expect(!available_for(scenario(sim::DciProfile::StrictJit,
                                 sim::LockMode::TurnTwoItem, true, 5),
                        bank_state(3)),
         "Persistent TurnTwoItem lock admitted projected Blender");
  expect(!available_for(scenario(sim::DciProfile::StrictJit,
                                 sim::LockMode::FullItem, true, 5),
                        bank_state(3)),
         "Full Item lock admitted projected Blender");
  expect(!available_for(scenario(sim::DciProfile::StrictJit,
                                 sim::LockMode::FullSupporter, true, 5),
                        bank_state(3)),
         "Supporter lock admitted Steven");
  expect(!available_for(scenario(sim::DciProfile::NoDiscardControl,
                                 sim::LockMode::None, true, 5),
                        bank_state(3)),
         "NoDiscardControl entered the same-ready-turn JIT packet");
  expect(!available_for(scenario(sim::DciProfile::StrictJit,
                                 sim::LockMode::None, true, 3),
                        bank_state(3)),
         "Route ignored exhausted next-turn horizon");
  expect(!available_for(scenario(), bank_state(3), false),
         "K0 admitted the K1-only route");

  sim::State state = bank_state(3);
  state.active->entered_turn = state.turn;
  expect(!available_for(scenario(), state),
         "Same-turn Regidrago V bypassed evolution-age control");
  state = bank_state(3);
  state.active->grass = 0;
  expect(!available_for(scenario(), state),
         "Wrong Energy board admitted the route");
  state = bank_state(3);
  state.manual_energy_used = true;
  expect(!available_for(scenario(), state),
         "Spent manual attachment admitted the route");

  for (const sim::Card missing : {sim::Card::Fire, sim::Card::StevensResolve,
                                  sim::Card::BrilliantBlender}) {
    state = bank_state(3);
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), missing));
    expect(!available_for(scenario(), state),
           "Missing held route resource admitted the bank");
  }
  for (const sim::Card missing : {sim::Card::RegidragoVstar, sim::Card::Grass,
                                  sim::Card::MegaDragonite}) {
    state = bank_state(3);
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(), missing));
    expect(!available_for(scenario(), state),
           "Missing K1 deck resource admitted the bank");
  }
  state = bank_state(3);
  state.hand.push_back(sim::Card::RegidragoVstar);
  expect(!available_for(scenario(), state),
         "Deferred package displaced a direct VSTAR connector");

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3279);
  sim::Engine finish_locked(scenario(sim::DciProfile::StrictJit,
                                     sim::LockMode::FullItem, true, 5),
                            recipe, rng);
  sim::EngineTestAccess::set_state(finish_locked, finish_state(4));
  sim::EngineTestAccess::set_finish_turn(finish_locked, 4);
  expect(!sim::EngineTestAccess::finish_available(finish_locked),
         "Current Item lock admitted the recorded Blender finish");
}

}  // namespace

int main() {
  semantic_bank_and_finish_parity();
  semantic_negative_controls();
  return 0;
}
