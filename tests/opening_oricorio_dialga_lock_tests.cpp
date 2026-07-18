#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_opening_active(Engine& engine) { engine.choose_opening_active(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State issue_hand() {
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::Powerglass,
                sim::Card::RegidragoVstar};
  return state;
}

const sim::Scenario& scenario_for(const sim::LockMode lock) {
  static const sim::Scenario none{"issue-674-none", sim::DciProfile::StrictJit,
                                  sim::LockMode::None, false, 4};
  static const sim::Scenario turn_two_item{
      "issue-674-turn-two-item", sim::DciProfile::StrictJit,
      sim::LockMode::TurnTwoItem, false, 4};
  static const sim::Scenario rule_box{
      "issue-674-rule-box", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, false, 4};
  static const sim::Scenario full_item{
      "issue-674-full-item", sim::DciProfile::StrictJit,
      sim::LockMode::FullItem, false, 4};
  static const sim::Scenario combined{
      "issue-674-combined", sim::DciProfile::StrictJit,
      sim::LockMode::FullCombined, false, 4};
  switch (lock) {
    case sim::LockMode::None: return none;
    case sim::LockMode::TurnTwoItem: return turn_two_item;
    case sim::LockMode::FullRuleBoxAbility: return rule_box;
    case sim::LockMode::FullItem: return full_item;
    case sim::LockMode::FullCombined: return combined;
  }
  throw std::runtime_error("Unsupported lock mode in issue 674 fixture.");
}

sim::Engine make_engine(const sim::LockMode lock, sim::State state,
                        std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario_for(lock), recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void expect_dialga_active(const sim::State& state, const char* message) {
  if (!state.active || state.active->card != sim::Card::DialgaGX ||
      !contains(state.hand, sim::Card::Oricorio) ||
      !contains(state.hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error(message);
  }
}

void expect_oricorio_active(const sim::State& state, const char* message) {
  if (!state.active || state.active->card != sim::Card::Oricorio ||
      !contains(state.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error(message);
  }
}

void test_four_approved_lock_modes() {
  const std::array<sim::LockMode, 4> locks = {
      sim::LockMode::TurnTwoItem,
      sim::LockMode::FullRuleBoxAbility,
      sim::LockMode::FullItem,
      sim::LockMode::FullCombined,
  };
  for (std::size_t index = 0; index < locks.size(); ++index) {
    std::mt19937_64 rng{67400 + index};
    sim::Engine engine = make_engine(locks[index], issue_hand(), rng);

    // Setup may choose Dialga-GX while Oricorio remains in hand. Vital Dance
    // triggers when Oricorio is later Benched, and Oricorio has no Rule Box:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://github.com/FlareZ123/pokemon-sims/issues/674
    sim::EngineTestAccess::choose_opening_active(engine);
    expect_dialga_active(sim::EngineTestAccess::state(engine),
                         "The approved lock graph should preserve Oricorio in hand.");
  }
}

void test_scope_controls() {
  {
    std::mt19937_64 rng{67410};
    sim::State state = issue_hand();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::RegidragoVstar),
                     state.hand.end());
    sim::Engine engine = make_engine(sim::LockMode::None, std::move(state), rng);
    sim::EngineTestAccess::choose_opening_active(engine);
    expect_oricorio_active(sim::EngineTestAccess::state(engine),
                           "The lock-conditioned fix must not create a no-lock route.");
  }
  {
    std::mt19937_64 rng{67411};
    sim::State state = issue_hand();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::MegaDragonite),
                     state.hand.end());
    sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility,
                                     std::move(state), rng);
    sim::EngineTestAccess::choose_opening_active(engine);
    expect_oricorio_active(sim::EngineTestAccess::state(engine),
                           "A unique Dialga-GX payload must remain protected.");
  }
  {
    std::mt19937_64 rng{67412};
    sim::State state;
    state.hand = {sim::Card::QuickBall, sim::Card::MegaDragonite,
                  sim::Card::DialgaGX, sim::Card::Oricorio,
                  sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
    sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility,
                                     std::move(state), rng);
    sim::EngineTestAccess::choose_opening_active(engine);
    expect_oricorio_active(sim::EngineTestAccess::state(engine),
                           "A complete GGF hand should preserve the Dialga payload.");
  }
  {
    std::mt19937_64 rng{67413};
    sim::State state = issue_hand();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::TeamYellsCheer),
                     state.hand.end());
    state.hand.push_back(sim::Card::Arven);
    sim::Engine engine = make_engine(sim::LockMode::FullItem,
                                     std::move(state), rng);
    sim::EngineTestAccess::choose_opening_active(engine);
    expect_oricorio_active(sim::EngineTestAccess::state(engine),
                           "Full Item lock requires the approved conditioned graph.");
  }
}
}  // namespace

int main() {
  test_four_approved_lock_modes();
  test_scope_controls();
  return 0;
}
