#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) {
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool compression_route(const Engine& engine) {
    return engine.late_steven_has_known_t3_compression_route();
  }
  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven();
  }
  static bool payload_outlet(Engine& engine) {
    return engine.play_known_steven_t3_payload_outlet();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void begin_turn(Engine& engine, const int turn) { engine.begin_turn(turn); }
  static bool ready(const Engine& engine) {
    const State& state = engine.state_;
    return state.turn >= 2 && engine.active_is_vstar() &&
        state.active->grass >= 2 && state.active->fire >= 1 && engine.payload_ready();
  }
  static std::string state_line(const Engine& engine) {
    return engine.state_line();
  }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

const sim::Pokemon* find_benched(const sim::State& state, const sim::Card card) {
  const auto it = std::find_if(state.bench.begin(), state.bench.end(),
                               [card](const sim::Pokemon& pokemon) {
                                 return pokemon.card == card;
                               });
  return it == state.bench.end() ? nullptr : &*it;
}

sim::State exact_route_state(const int bank_turn = 2) {
  sim::State state;
  state.turn = bank_turn;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, bank_turn - 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, bank_turn - 1, 1, 0, sim::Tool::None}};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::Fire, sim::Card::TateLiza,
                sim::Card::Grass, sim::Card::QuickBall, sim::Card::StevensResolve};
  state.deck = {sim::Card::LatiasEx, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite, sim::Card::Dipplin,
                sim::Card::Dragapult, sim::Card::GoodraVstar,
                sim::Card::Grass, sim::Card::Fire, sim::Card::Channeler};
  state.prizes = {sim::Card::QuickBall, sim::Card::ForestSealStone,
                  sim::Card::PathToPeak, sim::Card::Crispin,
                  sim::Card::Grass, sim::Card::RegidragoV};
  return state;
}

sim::State finish_state_without_bank(const int finish_turn = 3) {
  sim::State state;
  state.turn = finish_turn;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, finish_turn - 2, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, finish_turn - 1, 1, 1, sim::Tool::None},
      sim::Pokemon{sim::Card::LatiasEx, finish_turn, 0, 0, sim::Tool::None}};
  state.hand = {sim::Card::Grass, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite, sim::Card::TateLiza};
  state.deck = {sim::Card::Dragapult, sim::Card::GoodraVstar, sim::Card::Fire};
  state.prizes = {sim::Card::QuickBall, sim::Card::ForestSealStone,
                  sim::Card::PathToPeak, sim::Card::Crispin,
                  sim::Card::Grass, sim::Card::RegidragoV};
  return state;
}

struct EngineFixture {
  sim::Scenario scenario{"issue-869-late-steven-relative", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit EngineFixture(const std::uint64_t seed)
      : rng(seed), engine(scenario, recipe, rng) {}
};

// Engine stores references to DeckRecipe and RNG. Keep those owners alive for the
// complete fixture lifetime: https://github.com/FlareZ123/pokemon-sims/issues/869

void verify_bank_finish_pair(const int bank_turn, const std::uint64_t seed) {
  EngineFixture fixture(seed);
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::state(engine) = exact_route_state(bank_turn);
  sim::EngineTestAccess::set_deck_seen(engine);

  // The legal packet is relative to current state. Steven searches up to three cards
  // and ends the bank turn; evolution age, manual attachment, Item use, and retreat
  // are governed by current procedure rather than an absolute T2/T3 coordinate:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/869
  // https://github.com/FlareZ123/pokemon-sims/issues/2424
  // https://github.com/FlareZ123/pokemon-sims/issues/3601
  if (!sim::EngineTestAccess::compression_route(engine) ||
      !sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("confirmed late-Steven route was not admitted");
  }

  sim::EngineTestAccess::run_turn(engine);
  sim::State& after_steven = sim::EngineTestAccess::state(engine);
  const sim::Pokemon* prepared = find_benched(after_steven, sim::Card::RegidragoVstar);
  if (!after_steven.turn_ended || !after_steven.supporter_used || prepared == nullptr ||
      prepared->grass != 1 || prepared->fire != 1 ||
      !contains(after_steven.hand, sim::Card::Grass) ||
      !contains(after_steven.hand, sim::Card::LatiasEx) ||
      !contains(after_steven.hand, sim::Card::MysteriousTreasure) ||
      !contains(after_steven.hand, sim::Card::MegaDragonite) ||
      !contains(after_steven.discard, sim::Card::StevensResolve)) {
    throw std::runtime_error(
        "Steven did not preserve the exact relative route: " +
        sim::EngineTestAccess::state_line(engine));
  }

  sim::EngineTestAccess::begin_turn(engine, bank_turn + 1);
  if (!sim::EngineTestAccess::state(engine).turn_ended) {
    sim::EngineTestAccess::run_turn(engine);
  }
  const sim::State& ready = sim::EngineTestAccess::state(engine);
  if (!sim::EngineTestAccess::ready(engine) || !ready.active ||
      ready.active->card != sim::Card::RegidragoVstar ||
      ready.active->grass != 2 || ready.active->fire != 1 ||
      !contains(ready.discarded_this_turn, sim::Card::MegaDragonite) ||
      find_benched(ready, sim::Card::LatiasEx) == nullptr) {
    throw std::runtime_error(
        "confirmed Steven route did not reach readiness on the following turn: " +
        sim::EngineTestAccess::state_line(engine));
  }
}

void original_turn_two_pair_still_works() {
  verify_bank_finish_pair(2, 869);
}

void later_equivalent_turn_three_pair_works() {
  verify_bank_finish_pair(3, 3601);
}

void missing_target_blocks_route() {
  EngineFixture fixture(8691);
  sim::Engine& engine = fixture.engine;
  sim::State state = exact_route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::LatiasEx),
                   state.deck.end());
  state.prizes.push_back(sim::Card::LatiasEx);
  sim::EngineTestAccess::state(engine) = std::move(state);
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine) ||
      sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("Steven claimed a route with prized Latias ex");
  }
}

void missing_attachment_resource_blocks_route() {
  EngineFixture fixture(36011);
  sim::Engine& engine = fixture.engine;
  sim::State state = exact_route_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::Fire),
                   state.hand.end());
  sim::EngineTestAccess::state(engine) = std::move(state);
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine)) {
    throw std::runtime_error("Steven claimed a route without the current Fire attachment");
  }
}

void full_bench_blocks_route() {
  EngineFixture fixture(8692);
  sim::Engine& engine = fixture.engine;
  sim::State state = exact_route_state();
  while (state.bench.size() < 5U) {
    state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  }
  sim::EngineTestAccess::state(engine) = std::move(state);
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine) ||
      sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("Steven claimed a Latias route without Bench space");
  }
}

void item_lock_blocks_next_turn_treasure() {
  const sim::Scenario scenario{"issue-869-item-lock-control", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{8693};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::state(engine) = exact_route_state();
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine) ||
      sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("Steven claimed an Item-dependent route under Item lock");
  }
}

void supporter_lock_blocks_bank() {
  const sim::Scenario scenario{"issue-3601-supporter-lock", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::FullSupporter, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{36012};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::state(engine) = exact_route_state();
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine)) {
    throw std::runtime_error("Steven claimed a bank under Supporter lock");
  }
}

void ability_lock_blocks_latias_route() {
  const sim::Scenario scenario{"issue-3601-ability-lock", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::FullRuleBoxAbility, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{36013};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::state(engine) = exact_route_state();
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine)) {
    throw std::runtime_error("Steven claimed a Skyliner-dependent route under Ability lock");
  }
}

void used_retreat_blocks_route() {
  EngineFixture fixture(36014);
  sim::Engine& engine = fixture.engine;
  sim::State state = exact_route_state();
  state.retreat_used = true;
  sim::EngineTestAccess::state(engine) = std::move(state);
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine)) {
    throw std::runtime_error("Steven claimed the route after retreat was already used");
  }
}

void k0_blocks_known_route() {
  EngineFixture fixture(36015);
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::state(engine) = exact_route_state();
  if (sim::EngineTestAccess::compression_route(engine)) {
    throw std::runtime_error("Steven claimed the K1-only route from K0");
  }
}

void evolution_age_blocks_route() {
  EngineFixture fixture(36016);
  sim::Engine& engine = fixture.engine;
  sim::State state = exact_route_state(3);
  state.bench.front().entered_turn = 3;
  sim::EngineTestAccess::state(engine) = std::move(state);
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine)) {
    throw std::runtime_error("Steven claimed a route through a just-benched Regidrago V");
  }
}

void exhausted_horizon_blocks_bank() {
  const sim::Scenario scenario{"issue-3601-horizon", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 2};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{36017};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::state(engine) = exact_route_state(2);
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine)) {
    throw std::runtime_error("Steven claimed a bank without a modeled following turn");
  }
}

void no_bank_provenance_blocks_finish() {
  EngineFixture fixture(36018);
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::state(engine) = finish_state_without_bank();
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::payload_outlet(engine)) {
    throw std::runtime_error("Steven finish outlet fired without bank provenance");
  }
}

void stale_bank_provenance_blocks_finish() {
  EngineFixture fixture(36019);
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::state(engine) = exact_route_state(2);
  sim::EngineTestAccess::set_deck_seen(engine);
  if (!sim::EngineTestAccess::compression_route(engine)) {
    throw std::runtime_error("stale-provenance fixture failed to admit the bank");
  }
  sim::EngineTestAccess::run_turn(engine);
  sim::EngineTestAccess::begin_turn(engine, 4);
  if (sim::EngineTestAccess::payload_outlet(engine)) {
    throw std::runtime_error("Steven finish outlet accepted stale two-turn-old provenance");
  }
}

}  // namespace

int main() {
  try {
    original_turn_two_pair_still_works();
    later_equivalent_turn_three_pair_works();
    missing_target_blocks_route();
    missing_attachment_resource_blocks_route();
    full_bench_blocks_route();
    item_lock_blocks_next_turn_treasure();
    supporter_lock_blocks_bank();
    ability_lock_blocks_latias_route();
    used_retreat_blocks_route();
    k0_blocks_known_route();
    evolution_age_blocks_route();
    exhausted_horizon_blocks_bank();
    no_bank_provenance_blocks_finish();
    stale_bank_provenance_blocks_finish();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
