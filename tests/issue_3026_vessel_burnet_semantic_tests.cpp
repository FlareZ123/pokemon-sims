#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void mark_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool route_visible(const Engine& engine) {
    return engine.issue_1646_vessel_burnet_finish_visible();
  }
  static bool play_vessel(Engine& engine) {
    return engine.play_earthen_vessel(false);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

struct Fixture {
  Fixture(const sim::DciProfile dci, const sim::LockMode lock,
          const bool going_first = true, const int max_turn = 5)
      : scenario{"issue-3026/exact", dci, lock, going_first, max_turn},
        recipe{sim::baseline_recipe()},
        rng{3026},
        trace{true, {}},
        engine{scenario, recipe, rng, &trace} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;
};

sim::State finish_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {
      sim::Card::ProfessorBurnet,
      sim::Card::EarthenVessel,
      sim::Card::QuickBall,
  };
  state.deck = {
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Dragapult,
      sim::Card::RegidragoV,
  };
  state.prizes = {
      sim::Card::ForestSealStone,
      sim::Card::FieldBlower,
      sim::Card::Oricorio,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::QuickBall,
  };
  return state;
}

void install_state(Fixture& fixture, const int turn) {
  sim::EngineTestAccess::set_state(fixture.engine, finish_state(turn));
  sim::EngineTestAccess::mark_deck_seen(fixture.engine);
}

void test_matchup_flex_rulebox_route_is_visible_and_resolves() {
  Fixture fixture{sim::DciProfile::MatchupFlexJit,
                  sim::LockMode::FullRuleBoxAbility};
  install_state(fixture, 4);

  // Earthen Vessel is an Item and Professor Burnet is a Supporter, so a Rule Box
  // Ability lock does not make either Trainer action illegal. MatchupFlexJit and
  // StrictJit share the repository's same-ready-turn payload requirement:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced legality procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3026
  expect(sim::EngineTestAccess::route_visible(fixture.engine),
         "MatchupFlexJit Rule Box Ability lock hid the legal Vessel-Burnet route.");
  expect(sim::EngineTestAccess::play_vessel(fixture.engine),
         "The semantic MatchupFlexJit Vessel-Burnet route did not resolve.");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(count(after.hand, sim::Card::Grass) == 1,
         "Earthen Vessel did not search the final Grass Energy.");
  expect(count(after.discard, sim::Card::QuickBall) == 1 &&
             count(after.discard, sim::Card::ProfessorBurnet) == 1 &&
             count(after.discard, sim::Card::Dragapult) == 1,
         "The semantic route did not preserve the proven Quick Ball/Burnet payload sequence.");
}

void test_strict_t2_route_is_visible() {
  Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None};
  install_state(fixture, 2);

  // The turn number is not a printed Earthen Vessel, Burnet, or Apex Dragon
  // legality condition. With a prior-turn VSTAR, K1, unused Supporter and manual
  // attachment, one missing Basic Energy, and a live deck payload, the same route
  // is legal on T2:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3026
  expect(sim::EngineTestAccess::route_visible(fixture.engine),
         "The legal T2 Vessel-Burnet route was suppressed by the historical turn witness.");
}

void test_semantic_negative_controls() {
  {
    Fixture fixture{sim::DciProfile::NoDiscardControl, sim::LockMode::None};
    install_state(fixture, 3);
    // NoDiscardControl intentionally lacks the same-turn payload obligation:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/3026
    expect(!sim::EngineTestAccess::route_visible(fixture.engine),
           "NoDiscardControl incorrectly entered the JIT-specific Burnet route.");
  }

  {
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::FullItem};
    install_state(fixture, 3);
    // Earthen Vessel is an Item and cannot be used through the repository's
    // current full Item lock model:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
    // https://github.com/FlareZ123/pokemon-sims/issues/3026
    expect(!sim::EngineTestAccess::route_visible(fixture.engine),
           "Full Item lock incorrectly admitted the Vessel-Burnet route.");
  }

  {
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::FullSupporter};
    install_state(fixture, 3);
    // Professor Burnet is a Supporter, so Supporter lock blocks the route:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // https://github.com/FlareZ123/pokemon-sims/issues/3026
    expect(!sim::EngineTestAccess::route_visible(fixture.engine),
           "Supporter lock incorrectly admitted the Vessel-Burnet route.");
  }

  {
    Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None, true, 3};
    install_state(fixture, 4);
    // The repository scenario horizon remains a model boundary:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
    // https://github.com/FlareZ123/pokemon-sims/issues/3026
    expect(!sim::EngineTestAccess::route_visible(fixture.engine),
           "The Vessel-Burnet route ignored the configured simulation horizon.");
  }
}

}  // namespace

int main() {
  try {
    test_matchup_flex_rulebox_route_is_visible_and_resolves();
    test_strict_t2_route_is_visible();
    test_semantic_negative_controls();
    std::cout << "Issue 3026 Vessel-Burnet semantic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
