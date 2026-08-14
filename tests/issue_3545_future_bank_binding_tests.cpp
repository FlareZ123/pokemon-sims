#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = false;
  }
  static bool future_bank_supporter_is_used(
      const Engine& engine, const std::vector<Card>& targets,
      const Card current_supporter, const Card future_supporter) {
    return engine.issue_3545_future_bank_supporter_is_used(
        targets, current_supporter, future_supporter);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario() {
  return sim::Scenario{"issue-3545-future-bank-binding",
                       sim::DciProfile::StrictJit,
                       sim::LockMode::TurnTwoItem, false, 2};
}

sim::State base_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0};
  state.hand = {sim::Card::BattleCompressor, sim::Card::VsSeeker,
                sim::Card::VsSeeker, sim::Card::RegidragoVstar,
                sim::Card::Grass};
  state.deck = {sim::Card::Crispin, sim::Card::ProfessorBurnet,
                sim::Card::Arven, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite,
                sim::Card::MysteriousTreasure, sim::Card::QuickBall,
                sim::Card::EarthenVessel};
  return state;
}

void test_future_bank_requires_that_exact_supporter_action() {
  // The two-VS bank may discard a distinct second Supporter only when that exact
  // recovered card is the next-turn Supporter action. A generic setup improvement
  // from another held Supporter cannot justify spending Battle Compressor's slot.
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Persistent Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Connector domination: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  std::mt19937_64 positive_rng{3545101};
  sim::Engine positive{scenario(), sim::baseline_recipe(), positive_rng};
  sim::EngineTestAccess::set_state(positive, base_state());
  const std::vector<sim::Card> positive_targets{
      sim::Card::Crispin, sim::Card::ProfessorBurnet};
  expect(sim::EngineTestAccess::future_bank_supporter_is_used(
             positive, positive_targets, sim::Card::Crispin,
             sim::Card::ProfessorBurnet),
         "A live Crispin -> Burnet two-VS bank was not credited.");

  std::mt19937_64 negative_rng{3545102};
  sim::Engine negative{scenario(), sim::baseline_recipe(), negative_rng};
  sim::State negative_state = base_state();
  negative_state.hand.push_back(sim::Card::ProfessorBurnet);
  sim::EngineTestAccess::set_state(negative, std::move(negative_state));
  const std::vector<sim::Card> negative_targets{
      sim::Card::Crispin, sim::Card::Arven};
  expect(!sim::EngineTestAccess::future_bank_supporter_is_used(
             negative, negative_targets, sim::Card::Crispin,
             sim::Card::Arven),
         "A staged Arven received bank credit even though held Burnet is the next-turn Supporter.");
}
}  // namespace

int main() {
  try {
    test_future_bank_requires_that_exact_supporter_action();
    std::cout << "issue 3545 future-bank binding tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
