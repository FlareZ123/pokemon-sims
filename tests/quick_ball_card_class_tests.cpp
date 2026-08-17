#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace sim {

// The unified-test generator requires one access block per standalone regression
// source. These card-module tests intentionally exercise the public card/rules seam
// and therefore do not need privileged Engine access.
struct EngineTestAccess {};

}  // namespace sim

namespace {

using sim::Card;

struct FakeQuickBallState {
  std::vector<Card> hand;
  std::vector<Card> deck;
  std::vector<Card> discard;
  bool search_started = false;
  bool shuffled = false;
  bool selector_saw_search_started = false;
  bool selector_saw_source_in_discard = false;
  bool selector_saw_cost_in_discard = false;
  std::vector<std::string> events;
};

int hand_count(const void* opaque, const Card card) {
  const auto& state = *static_cast<const FakeQuickBallState*>(opaque);
  return static_cast<int>(std::count(state.hand.begin(), state.hand.end(), card));
}

bool move_hand_to_discard(void* opaque, const Card card) {
  auto& state = *static_cast<FakeQuickBallState*>(opaque);
  const auto it = std::find(state.hand.begin(), state.hand.end(), card);
  if (it == state.hand.end()) return false;
  state.discard.push_back(*it);
  state.hand.erase(it);
  state.events.emplace_back("source");
  return true;
}

bool discard_from_hand(void* opaque, const Card card,
                       const std::string_view reason,
                       const std::string_view rules_reference) {
  auto& state = *static_cast<FakeQuickBallState*>(opaque);
  if (reason != "Quick Ball cost" || rules_reference != "R-QB-01") {
    throw std::runtime_error("Quick Ball changed its cost trace contract.");
  }
  const auto it = std::find(state.hand.begin(), state.hand.end(), card);
  if (it == state.hand.end()) return false;
  state.discard.push_back(*it);
  state.hand.erase(it);
  state.events.emplace_back("cost");
  return true;
}

bool search_deck_to_hand(void* opaque, const Card card) {
  auto& state = *static_cast<FakeQuickBallState*>(opaque);
  if (!state.search_started) {
    throw std::runtime_error("Quick Ball searched before establishing K1.");
  }
  state.events.emplace_back("target");
  const auto it = std::find(state.deck.begin(), state.deck.end(), card);
  if (it == state.deck.end()) return false;
  state.hand.push_back(*it);
  state.deck.erase(it);
  return true;
}

void shuffle_deck(void* opaque) {
  auto& state = *static_cast<FakeQuickBallState*>(opaque);
  state.shuffled = true;
  state.events.emplace_back("shuffle");
}

bool is_basic_pokemon(const void*, const Card card) {
  return sim::is_basic(card);
}

void begin_deck_search(void* opaque, const std::string_view reason) {
  auto& state = *static_cast<FakeQuickBallState*>(opaque);
  if (reason != "Quick Ball") {
    throw std::runtime_error("Quick Ball changed its deck-search reason.");
  }
  state.search_started = true;
  state.events.emplace_back("search");
}

sim::rules::CardContext make_context(FakeQuickBallState& state) {
  return sim::rules::CardContext{&state, &hand_count, &move_hand_to_discard,
                                 &discard_from_hand, &search_deck_to_hand,
                                 &shuffle_deck, &is_basic_pokemon,
                                 &begin_deck_search};
}

std::optional<Card> choose_regidrago_after_inspection(void* opaque) {
  auto& state = *static_cast<FakeQuickBallState*>(opaque);
  state.selector_saw_search_started = state.search_started;
  state.selector_saw_source_in_discard =
      std::find(state.discard.begin(), state.discard.end(), Card::QuickBall) !=
      state.discard.end();
  state.selector_saw_cost_in_discard =
      std::find(state.discard.begin(), state.discard.end(), Card::Grass) !=
      state.discard.end();
  return state.search_started ? std::optional<Card>{Card::RegidragoV}
                              : std::nullopt;
}

std::optional<Card> choose_non_basic(void*) {
  return Card::Crispin;
}

std::optional<Card> choose_nothing(void*) {
  return std::nullopt;
}

void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

void test_registry_metadata_and_item_classification() {
  const auto* definition = sim::cards::find_definition(Card::QuickBall);
  require(definition != nullptr, "Quick Ball must be explicitly registered.");
  require(definition->id == Card::QuickBall,
          "Quick Ball registry id must remain stable.");
  require(definition->canonical_id == "swsh1-179",
          "Quick Ball canonical print changed.");
  require(definition->name == "Quick Ball", "Quick Ball name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Quick Ball must remain a Trainer.");
  require(definition->trainer_kind == sim::cards::TrainerKind::Item,
          "Quick Ball must remain an Item.");
  require(sim::is_item(Card::QuickBall),
          "Legacy compatibility classification must source Quick Ball as an Item.");
  require(sim::name(Card::QuickBall) == "Quick Ball",
          "Legacy compatibility name must source the registered definition.");
}

void test_resolution_preserves_cost_k1_search_shuffle_source_order() {
  FakeQuickBallState state{
      .hand = {Card::QuickBall, Card::Grass},
      .deck = {Card::RegidragoV, Card::Fire},
  };
  auto context = make_context(state);
  sim::cards::QuickBall::Action action{
      .discard = Card::Grass,
      .search_context = &state,
      .choose_search_target = &choose_regidrago_after_inspection,
  };

  const auto resolution = sim::cards::QuickBall::resolve(context, action);
  require(resolution.played, "A payable Quick Ball action must resolve.");
  require(state.selector_saw_search_started,
          "Engine target policy must run only after deck-search knowledge begins.");

  // Quick Ball's separate printed cost is already discarded while its search is
  // resolving, while B-01 keeps the resolving Item itself out of discard until the
  // Item has been used.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Item procedure B-01: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed lifecycle defect: https://github.com/FlareZ123/pokemon-sims/issues/4288
  require(state.selector_saw_cost_in_discard,
          "Quick Ball's mandatory other-card cost must be discarded before target selection.");
  require(!state.selector_saw_source_in_discard,
          "The resolving Quick Ball must stay out of discard during target selection.");

  require(resolution.search_target == Card::RegidragoV,
          "Post-inspection selector must choose Regidrago V.");
  require(resolution.found_target,
          "Selected Regidrago V must move from deck to hand.");
  require(state.shuffled, "Quick Ball must shuffle after its deck search.");
  require(state.events == std::vector<std::string>{"cost", "search", "target",
                                                   "shuffle", "source"},
          "Quick Ball must pay its cost, resolve search and shuffle, then discard the used Item.");
  require(std::count(state.discard.begin(), state.discard.end(), Card::QuickBall) == 1,
          "Played Quick Ball must enter discard exactly once after resolution.");
  require(std::count(state.discard.begin(), state.discard.end(), Card::Grass) == 1,
          "Quick Ball must discard the selected other-card cost exactly once.");
  require(std::count(state.hand.begin(), state.hand.end(), Card::RegidragoV) == 1,
          "Quick Ball search target must arrive in hand.");
}

void test_second_quick_ball_is_legal_other_card_cost() {
  FakeQuickBallState state{
      .hand = {Card::QuickBall, Card::QuickBall},
      .deck = {Card::Fire},
  };
  auto context = make_context(state);
  sim::cards::QuickBall::Action action{
      .discard = Card::QuickBall,
      .search_context = &state,
      .choose_search_target = &choose_nothing,
  };

  const auto resolution = sim::cards::QuickBall::resolve(context, action);
  require(resolution.played,
          "A second Quick Ball must be legal as the printed other-card cost.");
  require(state.hand.empty(), "Both Quick Ball copies must leave hand.");
  require(std::count(state.discard.begin(), state.discard.end(), Card::QuickBall) == 2,
          "Source and cost Quick Ball copies must both enter discard.");
  require(state.search_started && state.shuffled,
          "A legal empty Quick Ball search must still inspect and shuffle.");
}

void test_single_quick_ball_cannot_discard_itself() {
  FakeQuickBallState state{
      .hand = {Card::QuickBall},
      .deck = {Card::RegidragoV},
  };
  auto context = make_context(state);
  sim::cards::QuickBall::Action action{
      .discard = Card::QuickBall,
      .search_context = &state,
      .choose_search_target = &choose_regidrago_after_inspection,
  };

  const auto resolution = sim::cards::QuickBall::resolve(context, action);
  require(!resolution.played,
          "The played Quick Ball cannot pay its own other-card discard cost.");
  require(state.hand == std::vector<Card>{Card::QuickBall},
          "Rejected Quick Ball must preserve hand state.");
  require(state.discard.empty() && !state.search_started && !state.shuffled,
          "Rejected Quick Ball must not mutate later resolution state.");
}

void test_non_basic_strategy_target_is_never_searched() {
  FakeQuickBallState state{
      .hand = {Card::QuickBall, Card::Grass},
      .deck = {Card::Crispin, Card::RegidragoV},
  };
  auto context = make_context(state);
  sim::cards::QuickBall::Action action{
      .discard = Card::Grass,
      .search_context = &state,
      .choose_search_target = &choose_non_basic,
  };

  const auto resolution = sim::cards::QuickBall::resolve(context, action);
  require(resolution.played,
          "Quick Ball may complete as a legal failed search after paying its cost.");
  require(!resolution.search_target && !resolution.found_target,
          "Card code must reject a non-Basic strategy target intrinsically.");
  require(std::find(state.deck.begin(), state.deck.end(), Card::Crispin) !=
              state.deck.end(),
          "Quick Ball must never move a non-Basic target from deck.");
  require(state.shuffled, "A failed Quick Ball search must still shuffle.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_item_classification();
    test_resolution_preserves_cost_k1_search_shuffle_source_order();
    test_second_quick_ball_is_legal_other_card_cost();
    test_single_quick_ball_cannot_discard_itself();
    test_non_basic_strategy_target_is_never_searched();
    std::cout << "Quick Ball card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
