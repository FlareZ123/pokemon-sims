#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State payload_only_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::Serena,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::RegidragoV,
                sim::Card::Dragapult, sim::Card::DialgaGX,
                sim::Card::Fire, sim::Card::Grass};
  return state;
}

void test_k1_prefers_burnet_for_equal_turn_payload_completion() {
  const sim::Scenario scenario{"issue-2408-k1", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{2408};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, payload_only_state(), true, true);

  // K1 proves the deck-resident Dragon payloads. Burnet and Serena both finish the
  // sole strict-JIT payload axis this turn, so the lower resource-cost route keeps
  // Serena's draw/gust mode and the held Dragon available for later turns.
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter/search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::ProfessorBurnet),
         "K1 equal-turn payload completion failed to prefer Burnet.");
  expect(contains(result.hand, sim::Card::Serena),
         "K1 Burnet route failed to preserve Serena.");
  expect(contains(result.hand, sim::Card::MegaDragonite),
         "K1 Burnet route failed to preserve held Dragon payload.");
}

void test_k0_keeps_observable_serena_route() {
  const sim::Scenario scenario{"issue-2408-k0", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{2409};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, payload_only_state(), false, false);

  // K0 cannot infer that Burnet has a deck-resident Dragon target. Serena's held
  // Dragon route is observable and deterministic, so the K1 optimization must stay off.
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::Serena),
         "K0 incorrectly assumed a Burnet deck payload.");
  expect(contains(result.discard, sim::Card::MegaDragonite),
         "K0 Serena route failed to use the observable held payload.");
}
}  // namespace

int main() {
  try {
    test_k1_prefers_burnet_for_equal_turn_payload_completion();
    test_k0_keeps_observable_serena_route();
    std::cout << "Issue 2408 Burnet-before-Serena tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
