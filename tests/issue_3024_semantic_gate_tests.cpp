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
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = true;
  }

  static bool attach_manual(Engine& engine) {
    return engine.attach_manual();
  }

  static const State& state(const Engine& engine) {
    return engine.state_;
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State issue_3024_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0};
  state.active->entered_turn = 1;
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::DialgaGX,
      sim::Card::Serena,
      sim::Card::BrilliantBlender,
      sim::Card::QuickBall,
      sim::Card::Fire,
      sim::Card::Grass,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::RegidragoV,
      sim::Card::Channeler,
  };
  state.prizes = {
      sim::Card::Arven,
      sim::Card::FieldBlower,
      sim::Card::Guzma,
      sim::Card::Klara,
      sim::Card::Lusamine,
      sim::Card::PathToPeak,
  };
  state.discard = {sim::Card::EarthenVessel, sim::Card::MysteriousTreasure};
  return state;
}

struct Outcome {
  bool ready{};
  bool payload_this_turn{};
  bool evolved{};
  bool used_supporter{};
  bool used_manual_attachment{};
};

Outcome run_control(const sim::DciProfile dci, const sim::LockMode locks,
                    const bool going_first, const int turn,
                    const std::uint64_t seed) {
  const sim::Scenario scenario{"issue-3024", dci, locks, going_first, 5};
  std::mt19937_64 rng{seed};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::EngineTestAccess::set_state(engine, issue_3024_state(turn));
  expect(sim::EngineTestAccess::attach_manual(engine),
         "Issue-3024 fixture took no legal action");

  const sim::State& state = sim::EngineTestAccess::state(engine);
  const bool evolved = state.active &&
      state.active->card == sim::Card::RegidragoVstar;
  const bool ready = evolved && state.active->grass >= 2 &&
      state.active->fire >= 1;
  const bool payload_this_turn =
      std::find(state.discarded_this_turn.begin(),
                state.discarded_this_turn.end(), sim::Card::DialgaGX) !=
      state.discarded_this_turn.end();
  return Outcome{ready, payload_this_turn, evolved, state.supporter_used,
                 state.manual_energy_used};
}

bool complete(const Outcome& outcome) {
  return outcome.ready && outcome.payload_this_turn && outcome.evolved &&
      outcome.used_supporter && outcome.used_manual_attachment;
}

void test_semantic_equivalents_complete() {
  // StrictJit and MatchupFlexJit both require the Dragon payload on the actual
  // ready turn. Seat and absolute turn do not alter the printed Quick Ball,
  // Wonder Tag, Crispin, evolution-age, or Energy-attachment prerequisites:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official evolution, Item, Ability, Supporter, search, discard, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // DCI/JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3024
  expect(complete(run_control(sim::DciProfile::MatchupFlexJit,
                              sim::LockMode::None, true, 2, 302401)),
         "MatchupFlexJit rejected the same current-turn JIT route");
  expect(complete(run_control(sim::DciProfile::StrictJit,
                              sim::LockMode::None, false, 2, 302402)),
         "Going second rejected an otherwise legal continuation");
  expect(complete(run_control(sim::DciProfile::StrictJit,
                              sim::LockMode::None, true, 3, 302403)),
         "A later legal turn rejected the same observable continuation");
}

void test_semantic_locks_still_block() {
  // Quick Ball remains illegal under Item lock, while Wonder Tag remains
  // unavailable under the modeled Rule Box Ability lock. These are semantic
  // blockers and must survive removal of the historical LockMode::None gate:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Official Item and Ability procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-model
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3024
  const Outcome item_locked = run_control(
      sim::DciProfile::StrictJit, sim::LockMode::Turn2Item, true, 2, 302404);
  expect(!item_locked.ready || !item_locked.payload_this_turn,
         "Item lock illegally allowed the Quick Ball continuation");

  const Outcome ability_locked = run_control(
      sim::DciProfile::StrictJit, sim::LockMode::RuleBoxAbility,
      true, 2, 302405);
  expect(!ability_locked.ready || !ability_locked.used_supporter,
         "Rule Box Ability lock illegally allowed Wonder Tag continuation");
}
}  // namespace

int main() {
  try {
    test_semantic_equivalents_complete();
    test_semantic_locks_still_block();
    std::cout << "Issue 3024 semantic gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
