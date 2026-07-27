#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play(Engine& engine) { return engine.play_battle_vip_pass(); }
  static bool deck_seen(const Engine& engine) { return engine.deck_seen_; }
  static bool free_retreat(const Engine& engine) { return engine.can_free_retreat_with_latias(); }
  static std::optional<Card> choose_discard(Engine& engine) {
    return engine.choose_discard(false, true);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool in_play(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(),
                     [card](const sim::Pokemon& pokemon) {
                       return pokemon.card == card;
                     });
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng{1647};
  sim::TraceLog trace{true, {}};
  sim::Engine engine;

  Fixture(sim::LockMode lock = sim::LockMode::None,
          sim::DeckRecipe selected = sim::baseline_recipe())
      : scenario{"issue-1647", sim::DciProfile::StrictJit, lock, true, 4},
        recipe(std::move(selected)),
        engine(scenario, recipe, rng, &trace) {}
};

sim::State first_turn_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.hand = {sim::Card::BattleVipPass};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::CrobatV,
                sim::Card::Oricorio, sim::Card::LatiasEx,
                sim::Card::RegidragoV};
  return state;
}

void test_first_turn_searches_two_basics_without_entry_abilities() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, first_turn_state());
  expect(sim::EngineTestAccess::play(fixture.engine),
         "Battle VIP Pass did not resolve on turn one");
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
  // Tapu Lele-GX, Oricorio, and Crobat V entry conditions:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  // Latias ex static Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Core Item, search, Bench, and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/1647
  expect(in_play(state, sim::Card::RegidragoV),
         "Battle VIP Pass omitted Regidrago V");
  expect(in_play(state, sim::Card::LatiasEx),
         "Battle VIP Pass omitted the live Latias ex positioning target");
  expect(state.bench.size() == 2U,
         "Battle VIP Pass did not respect the up-to-two target limit");
  expect(std::count(state.discard.begin(), state.discard.end(),
                    sim::Card::BattleVipPass) == 1,
         "Battle VIP Pass did not enter discard after resolution");
  expect(sim::EngineTestAccess::deck_seen(fixture.engine),
         "Battle VIP Pass did not establish K1");
  expect(!state.dark_asset_used && !state.supporter_used,
         "direct deck placement incorrectly triggered a played-from-hand Ability");
  expect(sim::EngineTestAccess::free_retreat(fixture.engine),
         "Latias ex Skyliner was not live after direct Bench placement");
}

void test_first_turn_and_item_lock_gates() {
  Fixture late;
  sim::State state = first_turn_state();
  state.turn = 2;
  sim::EngineTestAccess::set_state(late.engine, state);
  expect(!sim::EngineTestAccess::play(late.engine),
         "Battle VIP Pass was playable after the first turn");

  Fixture locked(sim::LockMode::FullItem);
  sim::EngineTestAccess::set_state(locked.engine, first_turn_state());
  expect(!sim::EngineTestAccess::play(locked.engine),
         "Battle VIP Pass ignored Item lock");
}

void test_bench_capacity_and_pineco_priority() {
  Fixture one_slot;
  sim::State state = first_turn_state();
  state.bench = {{sim::Card::TapuLeleGX, 0}, {sim::Card::Oricorio, 0},
                 {sim::Card::CrobatV, 0}, {sim::Card::DialgaGX, 0}};
  sim::EngineTestAccess::set_state(one_slot.engine, state);
  expect(sim::EngineTestAccess::play(one_slot.engine),
         "Battle VIP Pass did not use the final legal Bench slot");
  expect(sim::EngineTestAccess::state(one_slot.engine).bench.size() == 5U,
         "Battle VIP Pass exceeded the five-Pokémon Bench limit");

  sim::DeckRecipe pineco = sim::pineco_recipe();
  pineco.push_back({sim::Card::BattleVipPass, 1});
  Fixture line(sim::LockMode::None, std::move(pineco));
  sim::State pineco_state = first_turn_state();
  pineco_state.deck.push_back(sim::Card::Pineco);
  sim::EngineTestAccess::set_state(line.engine, pineco_state);
  expect(sim::EngineTestAccess::play(line.engine),
         "Battle VIP Pass did not resolve for the Pineco line");
  const sim::State& after = sim::EngineTestAccess::state(line.engine);
  expect(in_play(after, sim::Card::RegidragoV) &&
             in_play(after, sim::Card::Pineco),
         "Battle VIP Pass did not prioritize Regidrago V plus Pineco");
}

void test_no_advancing_target_is_held_and_late_copy_is_dci() {
  Fixture no_target;
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::BattleVipPass};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::CrobatV,
                sim::Card::Oricorio, sim::Card::LatiasEx};
  sim::EngineTestAccess::set_state(no_target.engine, state);
  expect(!sim::EngineTestAccess::play(no_target.engine),
         "Battle VIP Pass was spent without a setup-advancing Basic");

  state.turn = 2;
  sim::EngineTestAccess::set_state(no_target.engine, state);
  const auto discard = sim::EngineTestAccess::choose_discard(no_target.engine);
  expect(discard && *discard == sim::Card::BattleVipPass,
         "post-turn-one Battle VIP Pass was not stable dead-card DCI");
}
}  // namespace

int main() {
  test_first_turn_searches_two_basics_without_entry_abilities();
  test_first_turn_and_item_lock_gates();
  test_bench_capacity_and_pineco_priority();
  test_no_advancing_target_is_held_and_late_copy_is_dci();
}
