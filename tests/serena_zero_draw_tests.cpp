#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool play_serena(Engine& engine, const bool allow_zero_draw_payload_completion = false) {
    return engine.play_serena(allow_zero_draw_payload_completion);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng) {
  return sim::Engine(scenario, sim::baseline_recipe(), rng);
}

void test_generic_fallback_rejects_zero_draw_payload_spend() {
  using namespace sim;
  const Scenario scenario{"issue-882-zero-draw", DciProfile::StrictJit,
                          LockMode::FullCombined, false, 4};
  std::mt19937_64 rng(88201);
  Engine engine = make_engine(scenario, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 1;
  state.active = Pokemon{Card::TapuLeleGX, 0, 0, 0, Tool::None};
  state.hand = {Card::Dipplin, Card::RegidragoVstar, Card::Grass,
                Card::MysteriousTreasure, Card::Serena, Card::Fire,
                Card::TeamYellsCheer};
  state.deck = {Card::RegidragoV, Card::MegaDragonite, Card::QuickBall};

  // Serena removes itself and at least one chosen card before drawing until five.
  // Seven cards therefore leave exactly five after the mandatory discard. Under
  // full Item and Rule Box Ability lock, discarding Dipplin cannot complete a
  // current-turn Regidrago route, so preserving both cards dominates a zero-draw play:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/882
  assert(!EngineTestAccess::play_serena(engine));
  assert(!state.supporter_used);
  assert(contains(state.hand, Card::Serena));
  assert(contains(state.hand, Card::Dipplin));
  assert(!contains(state.discard, Card::Serena));
  assert(!contains(state.discard, Card::Dipplin));
}

void test_generic_fallback_keeps_positive_draw_mode() {
  using namespace sim;
  const Scenario scenario{"issue-882-positive-draw", DciProfile::StrictJit,
                          LockMode::FullCombined, false, 4};
  std::mt19937_64 rng(88202);
  Engine engine = make_engine(scenario, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 1;
  state.active = Pokemon{Card::TapuLeleGX, 0, 0, 0, Tool::None};
  state.hand = {Card::Dipplin, Card::RegidragoVstar, Card::Grass,
                Card::Serena, Card::Fire, Card::TeamYellsCheer};
  state.deck = {Card::RegidragoV, Card::MegaDragonite, Card::QuickBall};

  // Six cards leave four after Serena and its mandatory discard, so the printed
  // draw-until-five effect gains one card and remains an admissible fallback:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://github.com/FlareZ123/pokemon-sims/issues/882
  assert(EngineTestAccess::play_serena(engine));
  assert(state.supporter_used);
  assert(state.hand.size() == 5U);
  assert(contains(state.discard, Card::Serena));
  assert(contains(state.discard, Card::Dipplin));
}

void test_zero_draw_is_kept_when_payload_completes_ready_state() {
  using namespace sim;
  const Scenario scenario{"issue-882-payload-completion", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  std::mt19937_64 rng(88203);
  Engine engine = make_engine(scenario, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::Serena, Card::Dipplin, Card::Grass, Card::Fire,
                Card::TeamYellsCheer, Card::Guzma, Card::Channeler};
  state.deck = {Card::QuickBall, Card::Arven};

  // The dedicated Serena branch may accept a zero-draw resolution when its mandatory
  // Dragon discard is the final strict-JIT setup axis on the current turn:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#discard-capability-index-dci
  // https://github.com/FlareZ123/pokemon-sims/issues/882
  assert(EngineTestAccess::play_serena(engine, true));
  assert(state.supporter_used);
  assert(contains(state.discard, Card::Serena));
  assert(contains(state.discard, Card::Dipplin));
  assert(contains(state.discarded_this_turn, Card::Dipplin));
}

}  // namespace

int main() {
  test_generic_fallback_rejects_zero_draw_payload_spend();
  test_generic_fallback_keeps_positive_draw_mode();
  test_zero_draw_is_kept_when_payload_completes_ready_state();
  return 0;
}
