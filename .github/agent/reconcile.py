import os
from pathlib import Path


def atomic_write(path: Path, text: str) -> None:
    temporary = path.with_name(path.name + ".tmp-781")
    temporary.write_text(text, encoding="utf-8")
    os.replace(temporary, path)


source = Path("src/trace_engine_v2/part_012_override.inc")
text = source.read_text(encoding="utf-8")
if "complementary_quick_ball_for_latias" not in text:
    old = """    const bool can_pay_search_cost = is_legal_cost_available(can_play_payload_this_turn(), true);

    const auto item_candidates_for_current_knowledge = [this, can_pay_search_cost]() {
"""
    new = """    const bool can_pay_search_cost = is_legal_cost_available(can_play_payload_this_turn(), true);

    const auto held_treasure_then_quick_ball_is_payable = [this]() {
      if (item_locked() || hand_count(Card::MysteriousTreasure) == 0 ||
          hand_count(Card::QuickBall) > 0 || !might_be_unseen(Card::RegidragoVstar) ||
          !might_be_unseen(Card::QuickBall)) return false;
      const std::vector<Card> original_hand = state_.hand;
      const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
      const bool permit_payload = can_play_payload_this_turn();
      const auto treasure_cost = choose_discard(
          permit_payload, true, true, Card::MysteriousTreasure);
      if (!treasure_cost || !remove_one(state_.hand, Card::MysteriousTreasure) ||
          !remove_one(state_.hand, *treasure_cost)) {
        restore_hand();
        return false;
      }
      state_.hand.push_back(Card::RegidragoVstar);
      state_.hand.push_back(Card::QuickBall);
      const bool payable = choose_discard(
          permit_payload, true, true, Card::QuickBall).has_value();
      restore_hand();
      return payable;
    };
    const bool evolvable_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn;
        });
    const bool complementary_quick_ball_for_latias =
        need_vstar() && state_.active && state_.active->card != Card::RegidragoV &&
        state_.active->card != Card::RegidragoVstar && evolvable_benched_regi &&
        bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        hand_count(Card::LatiasEx) == 0 && !in_play(Card::LatiasEx) &&
        might_be_unseen(Card::LatiasEx) && held_treasure_then_quick_ball_is_payable();

    const auto item_candidates_for_current_knowledge =
        [this, can_pay_search_cost, complementary_quick_ball_for_latias]() {
"""
    if old not in text:
        raise SystemExit("Arven candidate anchor missing")
    text = text.replace(old, new, 1)
    old_candidates = """      if (known_prized_regi_target) add_item_candidate(Card::HisuianHeavyBall, true);
      if (need_vstar() && live_vstar_target) {
"""
    new_candidates = """      if (known_prized_regi_target) add_item_candidate(Card::HisuianHeavyBall, true);
      // A held Mysterious Treasure can cover Regidrago VSTAR. Prefer the
      // complementary Quick Ball when both printed discard costs remain payable
      // and Latias ex is required to promote the evolved Benched attacker:
      // https://api.pokemontcg.io/v2/cards/sv1-166
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/781
      if (complementary_quick_ball_for_latias) add_item_candidate(Card::QuickBall, true);
      if (need_vstar() && live_vstar_target) {
"""
    if old_candidates not in text:
        raise SystemExit("Arven VSTAR candidate anchor missing")
    text = text.replace(old_candidates, new_candidates, 1)
    atomic_write(source, text)

test = Path("tests/arven_complementary_latias_route_tests.cpp")
test_text = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State route_state(const int payload_count, const bool latias_in_deck) {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0},
                 sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::Arven, sim::Card::MysteriousTreasure,
                sim::Card::ProfessorBurnet, sim::Card::DialgaGX};
  if (payload_count > 1) state.hand.push_back(sim::Card::Dragapult);
  state.deck = {sim::Card::RegidragoVstar, sim::Card::QuickBall,
                sim::Card::EvolutionIncense, sim::Card::Grass,
                sim::Card::Fire, sim::Card::ForestSealStone};
  if (latias_in_deck) state.deck.push_back(sim::Card::LatiasEx);
  return state;
}

void chooses_complementary_quick_ball() {
  const sim::Scenario scenario{"issue-781-positive", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{781};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(2, true));
  // Treasure covers VSTAR; Quick Ball remains payable for Latias ex promotion:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/781
  if (!sim::EngineTestAccess::play_arven(engine)) throw std::runtime_error("Arven did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::QuickBall) ||
      !contains(after.hand, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("Arven duplicated Treasure instead of preserving the Latias connector.");
  }
}

void unpayable_second_cost_does_not_force_quick_ball() {
  const sim::Scenario scenario{"issue-781-unpayable", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{7811};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(1, true));
  sim::EngineTestAccess::play_arven(engine);
  if (contains(sim::EngineTestAccess::state(engine).hand, sim::Card::QuickBall)) {
    throw std::runtime_error("Arven claimed an unpayable sequential Item route.");
  }
}

void unavailable_latias_does_not_force_quick_ball() {
  const sim::Scenario scenario{"issue-781-no-latias", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{7812};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(2, false));
  sim::EngineTestAccess::play_arven(engine);
  if (contains(sim::EngineTestAccess::state(engine).hand, sim::Card::QuickBall)) {
    throw std::runtime_error("Arven searched Quick Ball without a Latias ex target.");
  }
}

void item_lock_blocks_complementary_item() {
  const sim::Scenario scenario{"issue-781-lock", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{7813};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(2, true));
  sim::EngineTestAccess::play_arven(engine);
  if (contains(sim::EngineTestAccess::state(engine).hand, sim::Card::QuickBall)) {
    throw std::runtime_error("Item lock failed to block Arven's Item target.");
  }
}
}  // namespace

int main() {
  chooses_complementary_quick_ball();
  unpayable_second_cost_does_not_force_quick_ball();
  unavailable_latias_does_not_force_quick_ball();
  item_lock_blocks_complementary_item();
  return 0;
}
'''
atomic_write(test, test_text)

cmake = Path("CMakeLists.txt")
cmake_text = cmake.read_text(encoding="utf-8")
if "regidrago_arven_complementary_latias_tests" not in cmake_text:
    cmake_text += """

add_executable(regidrago_arven_complementary_latias_tests tests/arven_complementary_latias_route_tests.cpp)
target_compile_options(regidrago_arven_complementary_latias_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
add_test(NAME regidrago_arven_complementary_latias COMMAND regidrago_arven_complementary_latias_tests)
"""
    atomic_write(cmake, cmake_text)

for path in [Path(".github/workflows/issue-781-arven-complementary-route.yml"), Path(__file__)]:
    if path.exists():
        path.unlink()
