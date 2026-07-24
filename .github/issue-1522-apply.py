import os
from pathlib import Path


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


source_path = Path("src/trace_engine_v2/part_turo_oricorio_override.inc")
source = source_path.read_text(encoding="utf-8")
start_marker = "  bool tate_draw_has_held_non_supporter_completion() const {"
end_marker = "\n  void choose_supporter() {"
if source.count(start_marker) != 1 or source.count(end_marker) != 1:
    raise SystemExit("Expected one Tate held-route function and one choose_supporter anchor")
start = source.index(start_marker)
end = source.index(end_marker, start)
replacement = r'''  std::vector<Card> tate_k0_public_projection_deck() const {
    std::vector<Card> projected_deck;
    for (const auto& [card, copies] : recipe_) {
      const int evolved_regidrago_bases =
          card == Card::RegidragoV ? in_play_count(Card::RegidragoVstar) : 0;
      const int evolved_pineco_bases =
          card == Card::Pineco ? in_play_count(Card::ForretressEx) : 0;
      const int public_known = hand_count(card) + count_of(state_.discard, card) +
          in_play_count(card) + evolved_regidrago_bases +
          evolved_pineco_bases + attached_public_count(card);
      for (int unseen = std::max(0, copies - public_known); unseen > 0; --unseen) {
        projected_deck.push_back(card);
      }
    }
    return projected_deck;
  }

  bool tate_draw_has_held_non_supporter_completion() const {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 ||
        !setup_axis_missing()) {
      return false;
    }

    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    if (deck_seen_ || prizes_revealed_) {
      projected.deck_seen_ = deck_seen_;
      projected.prizes_revealed_ = prizes_revealed_;
    } else {
      // Before a legal inspection, assign every card still possible from fixed copy
      // counts to one synthetic search zone. This keeps the held-route classifier
      // identical for equal public K0 states while retaining exact K1 behavior after
      // a real deck or Prize inspection:
      // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Supporter, Item, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // K0/K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
      // Future-card-oracle boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1522
      projected.state_.deck = tate_k0_public_projection_deck();
      projected.state_.prizes.clear();
      projected.deck_seen_ = false;
      projected.prizes_revealed_ = false;
    }
    while (remove_one(projected.state_.hand, Card::TateLiza)) {
    }
    projected.state_.supporter_used = true;
    projected.run_turn();

    // Legacy Star samples a shuffled private top seven. That result cannot prove
    // that the public held non-Supporter cards deterministically complete the turn:
    // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Future-card-oracle boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Prior fixes: https://github.com/FlareZ123/pokemon-sims/issues/1057
    // https://github.com/FlareZ123/pokemon-sims/issues/1130
    // Confirmed hidden-placement bug: https://github.com/FlareZ123/pokemon-sims/issues/1522
    if (projected.outcome_.used_legacy_star) return false;

    return projected.active_is_vstar() && projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 && projected.payload_ready();
  }
'''
atomic_write(source_path, source[:start] + replacement + source[end:])


test_path = Path("tests/issue_1522_tate_k0_hidden_vstar_tests.cpp")
test_content = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }

  static bool tate_route_completes(const Engine& engine) {
    return engine.tate_draw_has_held_non_supporter_completion();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::DeckRecipe issue_recipe() {
  return {
      {sim::Card::RegidragoV, 1},       {sim::Card::RegidragoVstar, 1},
      {sim::Card::MegaDragonite, 1},    {sim::Card::TateLiza, 1},
      {sim::Card::MysteriousTreasure, 1}, {sim::Card::Crispin, 1},
      {sim::Card::Serena, 1},           {sim::Card::Gladion, 1},
      {sim::Card::Arven, 1},            {sim::Card::QuickBall, 1},
      {sim::Card::EarthenVessel, 1},    {sim::Card::LatiasEx, 1},
      {sim::Card::TapuLeleGX, 1},       {sim::Card::Grass, 2},
      {sim::Card::Fire, 1},
  };
}

sim::State paired_state(const bool vstar_in_deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::TateLiza, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {vstar_in_deck ? sim::Card::RegidragoVstar
                              : sim::Card::Crispin,
                sim::Card::Serena, sim::Card::Gladion};
  state.prizes = {vstar_in_deck ? sim::Card::Crispin
                                : sim::Card::RegidragoVstar,
                  sim::Card::Arven, sim::Card::QuickBall,
                  sim::Card::EarthenVessel, sim::Card::LatiasEx,
                  sim::Card::TapuLeleGX};
  return state;
}

bool tate_route_completes(sim::State state, const bool known) {
  const sim::Scenario scenario{"issue-1522", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = issue_recipe();
  std::mt19937_64 rng{1522};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess::tate_route_completes(engine);
}

void test_k0_tate_decision_ignores_hidden_vstar_location() {
  // The public hand can legally pay Mysterious Treasure's one-card discard and a
  // Regidrago VSTAR remains possible from fixed copy counts. Before the search,
  // moving that hidden VSTAR between deck and Prizes cannot change the Tate choice:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core search, Supporter, discard, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K0/K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1522
  const bool deck_result = tate_route_completes(paired_state(true), false);
  const bool prize_result = tate_route_completes(paired_state(false), false);
  expect(deck_result == prize_result,
         "K0 Tate decision still depends on hidden VSTAR placement");
  expect(deck_result,
         "K0 public Mysterious Treasure completion was not recognized");
}

void test_k1_tate_decision_uses_legally_inspected_vstar_location() {
  // After a legal inspection establishes K1, the exact Mysterious Treasure target
  // availability may distinguish the two states:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 begins only during legal effect resolution: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug and required positive control: https://github.com/FlareZ123/pokemon-sims/issues/1522
  expect(tate_route_completes(paired_state(true), true),
         "K1 failed to preserve the observable deck VSTAR route");
  expect(!tate_route_completes(paired_state(false), true),
         "K1 treated a known prized VSTAR as a Treasure target");
}

}  // namespace

int main() {
  test_k0_tate_decision_ignores_hidden_vstar_location();
  test_k1_tate_decision_uses_legally_inspected_vstar_location();
  return 0;
}
'''
atomic_write(test_path, test_content)
