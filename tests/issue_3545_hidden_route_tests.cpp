#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static std::optional<Card> compressor_supporter(const Engine& engine) {
    return engine.issue_3545_compressor_supporter_target();
  }
  static std::vector<Card> base_targets(const Engine& engine) {
    return engine.issue_3545_battle_compressor_targets();
  }
  static std::vector<Card> hidden_targets(const Engine& engine) {
    return engine.issue_3545_hidden_battle_compressor_targets();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::DciProfile dci = sim::DciProfile::StrictJit,
                       const sim::LockMode locks = sim::LockMode::None,
                       const int max_turn = 4) {
  return sim::Scenario{"issue-3545-hidden", dci, locks, false, max_turn};
}

sim::State stochastic_supporter_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0});
  state.hand = {sim::Card::BattleCompressor, sim::Card::VsSeeker,
                sim::Card::Powerglass, sim::Card::ProfessorTuro,
                sim::Card::MegaDragonite, sim::Card::Fire};
  state.deck = {
      sim::Card::TateLiza, sim::Card::Arven, sim::Card::Crispin,
      sim::Card::StevensResolve, sim::Card::RegidragoVstar,
      sim::Card::RegidragoVstar, sim::Card::MysteriousTreasure,
      sim::Card::MysteriousTreasure, sim::Card::QuickBall,
      sim::Card::EarthenVessel, sim::Card::EvolutionIncense,
      sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
      sim::Card::Fire, sim::Card::Fire, sim::Card::Dragapult,
      sim::Card::GoodraVstar, sim::Card::PathToPeak,
      sim::Card::ChaoticSwell, sim::Card::Guzma,
      sim::Card::FieldBlower};
  return state;
}

void test_bc_vs_supporter_choice_is_rng_invariant() {
  // Battle Compressor establishes K1 by legally inspecting the deck. The decision
  // may use the resulting public card identities, while the future shuffled order
  // and a later Tate & Liza / Serena draw remain unknown. Holding the complete public
  // state fixed must therefore hold the BC->VS Supporter target fixed as well.
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  std::optional<sim::Card> expected;
  bool initialized = false;
  for (std::uint64_t seed = 1; seed <= 64; ++seed) {
    std::mt19937_64 rng{seed};
    sim::Engine engine{scenario(), sim::baseline_recipe(), rng};
    sim::EngineTestAccess::set_state(engine, stochastic_supporter_state(), false);
    const auto target = sim::EngineTestAccess::compressor_supporter(engine);
    if (!initialized) {
      expected = target;
      initialized = true;
    } else {
      expect(target == expected,
             "BC->VS Supporter selection depended on a future shuffled draw.");
    }
  }
}

void test_two_vs_bank_does_not_spend_redundant_supporter_slot() {
  // Once the first recovered Supporter is played it returns to discard. A second VS
  // Seeker can bank that same Supporter before a persistent next-turn Item lock, so
  // BC should discard another Supporter only when that different future card gives a
  // strictly better public continuation.
  // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // Turn-2 Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  std::mt19937_64 rng{3545002};
  sim::Engine engine{scenario(sim::DciProfile::StrictJit,
                              sim::LockMode::TurnTwoItem, 3),
                     sim::baseline_recipe(), rng};
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 1};
  state.hand = {sim::Card::BattleCompressor, sim::Card::VsSeeker,
                sim::Card::VsSeeker, sim::Card::Grass};
  state.deck = {sim::Card::Crispin, sim::Card::Arven,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite,
                sim::Card::MysteriousTreasure, sim::Card::QuickBall};
  sim::EngineTestAccess::set_state(engine, std::move(state), false);

  const auto base = sim::EngineTestAccess::base_targets(engine);
  const auto hidden = sim::EngineTestAccess::hidden_targets(engine);
  const auto supporter_count = [](const std::vector<sim::Card>& cards) {
    return static_cast<int>(std::count_if(cards.begin(), cards.end(),
        [](const sim::Card card) { return sim::is_supporter(card); }));
  };
  expect(supporter_count(hidden) <= supporter_count(base),
         "Hidden two-VS bank added an unnecessary Supporter discard.");
  expect(supporter_count(hidden) <= 1,
         "BC spent two slots on Supporters even though the second VS can re-bank the first.");
}
}  // namespace

int main() {
  try {
    test_bc_vs_supporter_choice_is_rng_invariant();
    test_two_vs_bank_does_not_spend_redundant_supporter_slot();
    std::cout << "issue 3545 hidden-route tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
