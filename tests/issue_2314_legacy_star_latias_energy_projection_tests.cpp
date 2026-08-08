#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

// Current-main reclaim validation: https://github.com/FlareZ123/pokemon-sims/issues/2314
namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static bool use_legacy_star(Engine& engine) {
    return engine.use_legacy_star();
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

std::vector<sim::Card> resilient_targets() {
  return {
      sim::Card::RegidragoV, sim::Card::RegidragoVstar,
      sim::Card::Dragapult, sim::Card::MegaDragonite,
      sim::Card::DialgaGX, sim::Card::GoodraVstar,
      sim::Card::TapuLeleGX, sim::Card::Oricorio,
      sim::Card::Appletun, sim::Card::Dipplin};
}

sim::State base_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 2, 1, 1, sim::Tool::None}};
  state.hand = {
      sim::Card::MegaDragonite, sim::Card::BrilliantBlender,
      sim::Card::ProfessorBurnet, sim::Card::PathToPeak,
      sim::Card::Fire, sim::Card::MysteriousTreasure};
  state.discard = {sim::Card::EarthenVessel, sim::Card::EarthenVessel};
  state.deck = resilient_targets();
  state.deck.insert(state.deck.end(), 5, sim::Card::Grass);
  state.deck.push_back(sim::Card::LatiasEx);
  return state;
}

sim::State reveal_case(const bool mill_latias, const bool mill_all_grass) {
  sim::State state = base_state();
  std::vector<sim::Card> top_seven;

  if (mill_latias) {
    const auto latias = std::find(state.deck.begin(), state.deck.end(), sim::Card::LatiasEx);
    state.deck.erase(latias);
    top_seven.push_back(sim::Card::LatiasEx);
  }
  if (mill_all_grass) {
    for (int i = 0; i < 5; ++i) {
      const auto grass = std::find(state.deck.begin(), state.deck.end(), sim::Card::Grass);
      state.deck.erase(grass);
      top_seven.push_back(sim::Card::Grass);
    }
  }
  while (top_seven.size() < 7U) {
    const auto filler = std::find_if(
        state.deck.begin(), state.deck.end(), [](const sim::Card card) {
          return card != sim::Card::LatiasEx && card != sim::Card::Grass;
        });
    if (filler == state.deck.end()) throw std::runtime_error("fixture lacks top-seven filler");
    top_seven.push_back(*filler);
    state.deck.erase(filler);
  }

  state.deck.insert(state.deck.end(), top_seven.begin(), top_seven.end());
  return state;
}

sim::State run_projection(sim::State state, const std::uint64_t seed) {
  const sim::Scenario scenario{"issue-2314", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng, &trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(sim::EngineTestAccess::use_legacy_star(engine),
         "Legacy Star rejected a deterministic Latias-plus-Energy projection.");
  return sim::EngineTestAccess::state(engine);
}

void test_four_post_reveal_branches() {
  // These four fixtures enumerate the complete two-axis reveal partition. Legacy Star
  // may recover any two cards; Mysterious Treasure searches Latias while it remains
  // in deck, Earthen Vessel searches Grass while Grass remains, and Skyliner gives
  // the Basic Active no Retreat Cost after Latias is Benched:
  // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Mega Dragonite ex payload: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Dialga-GX Basic Active: https://api.pokemontcg.io/v2/cards/sm5-100
  // Official Ability, Item, search, discard, attachment, Bench, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 / earliest-route / strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed seed-15 bug: https://github.com/FlareZ123/pokemon-sims/issues/2314
  const sim::State neither_milled = run_projection(reveal_case(false, false), 231401);
  expect(std::count(neither_milled.hand.begin(), neither_milled.hand.end(),
                    sim::Card::EarthenVessel) == 1,
         "Neither-milled branch did not recover Earthen Vessel.");

  const sim::State latias_milled = run_projection(reveal_case(true, false), 231402);
  expect(std::count(latias_milled.hand.begin(), latias_milled.hand.end(),
                    sim::Card::LatiasEx) == 1 &&
         std::count(latias_milled.hand.begin(), latias_milled.hand.end(),
                    sim::Card::EarthenVessel) == 1,
         "Latias-milled branch did not recover Latias plus Vessel.");

  const sim::State grass_milled = run_projection(reveal_case(false, true), 231403);
  expect(std::count(grass_milled.hand.begin(), grass_milled.hand.end(),
                    sim::Card::Grass) == 1,
         "All-Grass-milled branch did not recover Grass.");

  const sim::State both_milled = run_projection(reveal_case(true, true), 231404);
  expect(std::count(both_milled.hand.begin(), both_milled.hand.end(),
                    sim::Card::LatiasEx) == 1 &&
         std::count(both_milled.hand.begin(), both_milled.hand.end(),
                    sim::Card::Grass) == 1,
         "Latias-plus-Grass-milled branch did not recover both cards.");
}

void test_spent_retreat_rejects_projection() {
  sim::State state = reveal_case(false, false);
  state.retreat_used = true;
  const sim::Scenario scenario{"issue-2314-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng(231405);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng, &trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::use_legacy_star(engine),
         "Spent retreat still admitted the issue-2314 promotion projection.");
}
}  // namespace

int main() {
  try {
    test_four_post_reveal_branches();
    test_spent_retreat_rejects_projection();
    std::cout << "Issue 2314 Legacy Star Latias/Energy projection tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
