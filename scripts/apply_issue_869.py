from pathlib import Path

ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/869"


def replace_once(path: str, old: str, new: str) -> None:
    file_path = Path(path)
    text = file_path.read_text(encoding="utf-8")
    if old not in text:
        raise SystemExit(f"Unable to find patch anchor in {path}")
    file_path.write_text(text.replace(old, new, 1), encoding="utf-8")


late_steven_path = "src/trace_engine_v2/part_010_late_steven_override.inc"
late_steven_anchor = "  bool late_steven_is_only_basic_connector() const {\n"
late_steven_helper = r'''  bool late_steven_has_known_t3_compression_route() const {
    if (state_.turn != 2 || scenario_.locks != LockMode::None || !supporter_allowed() ||
        hand_count(Card::StevensResolve) == 0 || hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::QuickBall) == 0 || !deck_seen_ || !state_.active ||
        state_.active->card != Card::TapuLeleGX || state_.retreat_used ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx) ||
        hand_count(Card::LatiasEx) > 0 || in_play(Card::LatiasEx) ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        deck_count_after_search_started(Card::MysteriousTreasure) == 0 ||
        deck_count_after_search_started(Card::MegaDragonite) == 0) {
      return false;
    }

    bool before_attachment = false;
    bool after_attachment = false;
    bool after_evolution = false;
    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.entered_turn >= state_.turn) continue;
      if (pokemon.card == Card::RegidragoV && pokemon.grass == 1 && pokemon.fire == 0 &&
          !state_.manual_energy_used && hand_count(Card::RegidragoVstar) > 0 &&
          hand_count(Card::Fire) > 0 && hand_count(Card::Grass) > 0) {
        before_attachment = true;
      }
      if (pokemon.card == Card::RegidragoV && pokemon.grass == 1 && pokemon.fire == 1 &&
          state_.manual_energy_used && hand_count(Card::RegidragoVstar) > 0 &&
          hand_count(Card::Grass) > 0) {
        after_attachment = true;
      }
      if (pokemon.card == Card::RegidragoVstar && pokemon.grass == 1 && pokemon.fire == 1 &&
          state_.manual_energy_used && hand_count(Card::Grass) > 0) {
        after_evolution = true;
      }
    }

    // Steven's Resolve can bank Latias ex, Mysterious Treasure, and Mega Dragonite ex
    // after K1 proves all three remain in deck. Evolving the prior-turn Regidrago V
    // and attaching Fire before Steven preserves the held Grass attachment for T3;
    // Treasure then discards the held Dragon payload, Latias grants free Retreat, and
    // the route reaches Active GGF Regidrago VSTAR one turn earlier than Tate draw mode:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    return before_attachment || after_attachment || after_evolution;
  }

'''
replace_once(late_steven_path, late_steven_anchor, late_steven_helper + late_steven_anchor)

attachment_path = "src/trace_engine_v2/part_tate_blender_attachment_override.inc"
attachment_anchor = "  bool attach_manual() {\n"
attachment_replacement = r'''  bool attach_manual() {
    if (late_steven_has_known_t3_compression_route() && !state_.manual_energy_used &&
        hand_count(Card::Fire) > 0) {
      for (Pokemon& pokemon : state_.bench) {
        if (pokemon.card != Card::RegidragoV || pokemon.entered_turn >= state_.turn ||
            pokemon.grass != 1 || pokemon.fire != 0) {
          continue;
        }
        remove_one(state_.hand, Card::Fire);
        ++pokemon.fire;
        state_.manual_energy_used = true;
        // Preserve Grass for the next turn of the exact Steven compression route:
        // https://api.pokemontcg.io/v2/cards/sm7-145
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://api.pokemontcg.io/v2/cards/sm6-113
        // https://github.com/FlareZ123/pokemon-sims/issues/869
        trace("ATTACH", "R-GAME-ENERGY; R-STEVEN-01; R-RVS-01; P-DCI-01",
              "Fire Energy manually to the prior-turn Regidrago V for the known T3 Steven route.");
        return true;
      }
    }
'''
replace_once(attachment_path, attachment_anchor, attachment_replacement)

steven_gate_path = "src/trace_engine_v2/part_010_steven_crispin_override.inc"
steven_gate_old = r'''  bool should_play_steven() const {
    return (should_play_steven_original() &&
            !steven_should_yield_to_crispin_mysterious_route()) ||
        late_steven_is_only_basic_connector();
  }
'''
steven_gate_new = r'''  bool should_play_steven() const {
    return (should_play_steven_original() &&
            !steven_should_yield_to_crispin_mysterious_route()) ||
        late_steven_is_only_basic_connector() ||
        late_steven_has_known_t3_compression_route();
  }
'''
replace_once(steven_gate_path, steven_gate_old, steven_gate_new)

turn_path = "src/trace_engine_v2/part_014c.inc"
turn_old = r'''    attach_manual();
    // Steven's Resolve ends the turn, so attach a live Powerglass before choosing
    // that Supporter: https://api.pokemontcg.io/v2/cards/sm7-145
    if (should_play_steven()) attach_powerglass();
'''
turn_new = r'''    attach_manual();
    // The confirmed K1 route must evolve the prior-turn Regidrago V before the
    // turn-ending Steven search so the remaining targets solve payload and promotion:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    if (late_steven_has_known_t3_compression_route()) evolve_best_regi();
    // Steven's Resolve ends the turn, so attach a live Powerglass before choosing
    // that Supporter: https://api.pokemontcg.io/v2/cards/sm7-145
    if (should_play_steven()) attach_powerglass();
'''
replace_once(turn_path, turn_old, turn_new)

steven_path = "src/trace_engine_v2/part_011.inc"
steven_start_old = r'''  bool play_steven() {
    if (!supporter_allowed() || hand_count(Card::StevensResolve) == 0 || !should_play_steven()) return false;
    remove_one(state_.hand, Card::StevensResolve);
'''
steven_start_new = r'''  bool play_steven() {
    if (!supporter_allowed() || hand_count(Card::StevensResolve) == 0 || !should_play_steven()) return false;
    const bool known_t3_compression = late_steven_has_known_t3_compression_route();
    remove_one(state_.hand, Card::StevensResolve);
'''
replace_once(steven_path, steven_start_old, steven_start_new)

wanted_anchor = "    std::vector<Card> wanted;\n"
wanted_replacement = r'''    std::vector<Card> wanted;
    if (known_t3_compression) {
      // These exact K1 targets complete the confirmed T3 route; generic fallbacks may
      // append later, but Steven resolves only the first three targets in this order:
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/me2pt5-152
      // https://github.com/FlareZ123/pokemon-sims/issues/869
      wanted.push_back(Card::LatiasEx);
      wanted.push_back(Card::MysteriousTreasure);
      wanted.push_back(Card::MegaDragonite);
    }
'''
replace_once(steven_path, wanted_anchor, wanted_replacement)

test_path = Path("tests/late_steven_t3_compression_tests.cpp")
test_path.write_text(r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void begin_turn(Engine& engine, const int turn) { engine.begin_turn(turn); }
  static bool ready(const Engine& engine) { return engine.ready(); }
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

sim::State exact_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None}};
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

sim::Engine make_engine(const std::uint64_t seed) {
  const sim::Scenario scenario{"issue-869-late-steven-t3", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{seed};
  return sim::Engine{scenario, recipe, rng};
}

void exact_route_reaches_turn_three() {
  sim::Engine engine = make_engine(869);
  sim::EngineTestAccess::state(engine) = exact_route_state();
  sim::EngineTestAccess::set_deck_seen(engine);

  // Steven searches any three cards and ends the turn. The prior-turn Regidrago V
  // may evolve first; Fire is attached now, Grass next turn, Mysterious Treasure
  // discards Mega Dragonite ex, and Latias ex frees the Basic Active's Retreat Cost:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/issues/869
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
    throw std::runtime_error("Steven did not preserve the exact T3 route");
  }

  sim::EngineTestAccess::begin_turn(engine, 3);
  if (!sim::EngineTestAccess::state(engine).turn_ended) {
    sim::EngineTestAccess::run_turn(engine);
  }
  const sim::State& ready = sim::EngineTestAccess::state(engine);
  if (!sim::EngineTestAccess::ready(engine) || !ready.active ||
      ready.active->card != sim::Card::RegidragoVstar ||
      ready.active->grass != 2 || ready.active->fire != 1 ||
      !contains(ready.discarded_this_turn, sim::Card::MegaDragonite) ||
      find_benched(ready, sim::Card::LatiasEx) == nullptr) {
    throw std::runtime_error("confirmed Steven route did not reach readiness on T3");
  }
}

void missing_target_blocks_route() {
  sim::Engine engine = make_engine(8691);
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

void full_bench_blocks_route() {
  sim::Engine engine = make_engine(8692);
  sim::State state = exact_route_state();
  while (state.bench.size() < 5U) {
    state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 2, 0, 0, sim::Tool::None});
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

}  // namespace

int main() {
  exact_route_reaches_turn_three();
  missing_target_blocks_route();
  full_bench_blocks_route();
  item_lock_blocks_next_turn_treasure();
  return 0;
}
''', encoding="utf-8")

cmake_path = "CMakeLists.txt"
cmake_anchor = "add_test(NAME regidrago_arven_complementary_latias COMMAND regidrago_arven_complementary_latias_tests)\n"
cmake_addition = r'''add_test(NAME regidrago_arven_complementary_latias COMMAND regidrago_arven_complementary_latias_tests)

# Deterministic K1 late-Steven T3 compression route:
# https://api.pokemontcg.io/v2/cards/sm7-145
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://api.pokemontcg.io/v2/cards/sm6-113
# https://api.pokemontcg.io/v2/cards/me2pt5-152
# https://api.pokemontcg.io/v2/cards/sv8-76
# https://github.com/FlareZ123/pokemon-sims/issues/869
add_executable(regidrago_late_steven_t3_compression_tests
  tests/late_steven_t3_compression_tests.cpp)
target_compile_options(regidrago_late_steven_t3_compression_tests PRIVATE
  -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
add_test(NAME regidrago_late_steven_t3_compression
  COMMAND regidrago_late_steven_t3_compression_tests)
'''
replace_once(cmake_path, cmake_anchor, cmake_addition)
