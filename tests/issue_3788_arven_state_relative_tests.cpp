#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool value) { engine.deck_seen_ = value; }
  static bool arven_redundant_payload_route_available(const Engine& engine) {
    return engine.issue_1605_arven_crobat_route_available();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::DeckRecipe route_recipe() {
  return {
      {sim::Card::RegidragoV, 4},
      {sim::Card::QuickBall, 1},
      {sim::Card::MysteriousTreasure, 1},
      {sim::Card::Arven, 1},
      {sim::Card::MegaDragonite, 2},
      {sim::Card::GoodraVstar, 1},
  };
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.hand = {
      sim::Card::Arven,
      sim::Card::MegaDragonite,
      sim::Card::MegaDragonite,
      sim::Card::GoodraVstar,
  };
  state.deck = {sim::Card::RegidragoV, sim::Card::QuickBall};
  return state;
}

bool route_available(const int turn, const bool going_first,
                     sim::State state,
                     const sim::LockMode locks = sim::LockMode::None,
                     const bool deck_seen = false) {
  const sim::Scenario scenario{"issue-3788", sim::DciProfile::StrictJit,
                               locks, going_first, 4};
  std::mt19937_64 rng(static_cast<std::uint64_t>(378800 + turn * 10 + going_first));
  sim::Engine engine(scenario, route_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, deck_seen);
  return sim::EngineTestAccess::arven_redundant_payload_route_available(engine);
}

void test_state_relative_positive_witnesses() {
  // Arven may be played once during a legal Supporter turn. Quick Ball and
  // Mysterious Treasure each discard one hand card before searching Regidrago V;
  // the dynamic DCI witness spends only a duplicated Dragon while preserving a
  // copy of that identity plus a second distinct Dragon identity:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Advanced Supporter and Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Generic predecessor: https://github.com/FlareZ123/pokemon-sims/issues/3467
  // Confirmed residual bug: https://github.com/FlareZ123/pokemon-sims/issues/3788
  expect(route_available(1, false, route_state(1)),
         "T1 going-second Arven witness must remain live.");
  expect(route_available(2, false, route_state(2)),
         "Equivalent T2 going-second state must admit the Arven connector.");
  expect(route_available(2, true, route_state(2)),
         "Equivalent T2 going-first state must admit Arven once Supporters are legal.");
}

void test_supporter_and_item_legality_controls() {
  // The player going first cannot play a Supporter on that player's first turn:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  expect(!route_available(1, true, route_state(1)),
         "T1 going-first must remain blocked by Supporter legality.");

  sim::State supporter_used = route_state(2);
  supporter_used.supporter_used = true;
  expect(!route_available(2, true, std::move(supporter_used)),
         "A used Supporter slot must block Arven admission.");

  // Quick Ball and Mysterious Treasure are Items, so FullItem lock must block
  // the searched one-discard continuation:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  expect(!route_available(2, true, route_state(2), sim::LockMode::FullItem),
         "Item lock must block the one-discard search route.");
}

void test_board_knowledge_and_dci_controls() {
  sim::State full_bench = route_state(2);
  full_bench.bench.assign(5, sim::Pokemon{sim::Card::TapuLeleGX, 1});
  expect(!route_available(2, true, std::move(full_bench)),
         "A full Bench must block the missing-Regidrago setup axis.");

  sim::State missing_target = route_state(2);
  missing_target.deck = {sim::Card::QuickBall};
  // K1 knowledge must reject a Regidrago V search after inspection proves that
  // no Regidrago V remains in the deck:
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  expect(!route_available(2, true, std::move(missing_target),
                          sim::LockMode::None, true),
         "A known deck with no Regidrago V target must block the route.");

  sim::State insufficient_payload = route_state(2);
  insufficient_payload.hand = {
      sim::Card::Arven,
      sim::Card::MegaDragonite,
      sim::Card::MegaDragonite,
  };
  // Duplicate-payload DCI preserves a second distinct modeled Dragon identity;
  // two copies of only one identity do not satisfy that survivor contract:
  // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Generic predecessor: https://github.com/FlareZ123/pokemon-sims/issues/3467
  // Confirmed residual bug: https://github.com/FlareZ123/pokemon-sims/issues/3788
  expect(!route_available(2, true, std::move(insufficient_payload)),
         "Duplicate payload fuel without a distinct survivor must remain protected.");
}

}  // namespace

int main() {
  test_state_relative_positive_witnesses();
  test_supporter_and_item_legality_controls();
  test_board_knowledge_and_dci_controls();
  return 0;
}
