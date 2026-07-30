#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void set_deck_seen(Engine& engine, const bool seen) {
    engine.deck_seen_ = seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static Card fallback(Engine& engine) {
    return engine.fallback_mysterious_target_after_search_started();
  }
  static bool play(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
};
}

namespace {

sim::Engine make_engine(const std::uint64_t seed) {
  static const sim::Scenario scenario{
      "issue-1949", sim::DciProfile::StrictJit,
      sim::LockMode::None, false, 4};
  static std::mt19937_64 rng{seed};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::CrobatV, 1);
  return sim::Engine(scenario, recipe, rng);
}

void test_fallback_skips_darkness_crobat_for_legal_dragon() {
  sim::Engine engine = make_engine(1949);
  sim::State state;
  state.deck = {sim::Card::CrobatV, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Mysterious Treasure searches only Psychic or Dragon Pokémon. Crobat V is
  // Darkness, while Mega Dragonite ex is Dragon, so the exhaustive K1 fallback
  // must select Mega Dragonite ex:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/issues/1949
  if (sim::EngineTestAccess::fallback(engine) != sim::Card::MegaDragonite) {
    throw std::runtime_error(
        "Mysterious Treasure fallback selected an illegal Darkness target.");
  }
}

void test_every_modeled_fallback_target_is_typed_legal() {
  constexpr std::array legal_targets{
      sim::Card::RegidragoV, sim::Card::RegidragoVstar,
      sim::Card::LatiasEx, sim::Card::Oricorio, sim::Card::TapuLeleGX,
      sim::Card::MegaDragonite, sim::Card::Dragapult,
      sim::Card::GoodraVstar, sim::Card::DialgaGX,
      sim::Card::Appletun, sim::Card::Dipplin};

  // The exhaustive fallback itself is a legality boundary. Every modeled entry
  // must satisfy Mysterious Treasure's Psychic-or-Dragon restriction:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1949
  for (const sim::Card target : legal_targets) {
    sim::Engine engine = make_engine(1950 + static_cast<std::uint64_t>(target));
    sim::State state;
    state.deck = {target};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    if (!sim::is_dragon_or_psychic(target) ||
        sim::EngineTestAccess::fallback(engine) != target) {
      throw std::runtime_error(
          "Mysterious Treasure fallback contains an untyped or unreachable target.");
    }
  }
}

void test_known_crobat_only_deck_does_not_pay_the_item_cost() {
  sim::Engine engine = make_engine(2000);
  sim::State state;
  state.turn = 2;
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Grant};
  state.deck = {sim::Card::CrobatV};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // At K1, a known Crobat-V-only deck contains no legal Mysterious Treasure
  // target, so the Item cannot be played and its discard cost remains unpaid:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/1949
  if (sim::EngineTestAccess::play(engine)) {
    throw std::runtime_error(
        "Mysterious Treasure played with no legal known-deck target.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.hand.size() != 2U || after.deck.size() != 1U ||
      !after.discard.empty()) {
    throw std::runtime_error(
        "Targetless Mysterious Treasure mutated zones before resolution.");
  }
}

}

int main() {
  try {
    test_fallback_skips_darkness_crobat_for_legal_dragon();
    test_every_modeled_fallback_target_is_typed_legal();
    test_known_crobat_only_deck_does_not_pay_the_item_cost();
    std::cout << "Issue 1949 Mysterious Treasure legality tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
