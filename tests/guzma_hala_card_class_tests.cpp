#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace sim {

struct EngineTestAccess {};

}  // namespace sim

namespace {

using sim::Card;

struct FakeGuzmaHalaState {
  std::vector<Card> hand;
  std::vector<Card> deck;
  std::vector<Card> discard;
  bool search_started = false;
  bool selector_saw_search_started = false;
  bool shuffled = false;
  std::vector<std::string> events;
};

int hand_count(const void* opaque, const Card card) {
  const auto& state = *static_cast<const FakeGuzmaHalaState*>(opaque);
  return static_cast<int>(std::count(state.hand.begin(), state.hand.end(), card));
}

bool move_hand_to_discard(void* opaque, const Card card) {
  auto& state = *static_cast<FakeGuzmaHalaState*>(opaque);
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
  auto& state = *static_cast<FakeGuzmaHalaState*>(opaque);
  if (reason != "Guzma & Hala optional cost" ||
      rules_reference != "R-GUZMA-HALA-01") {
    throw std::runtime_error("Guzma & Hala changed its cost trace contract.");
  }
  const auto it = std::find(state.hand.begin(), state.hand.end(), card);
  if (it == state.hand.end()) return false;
  state.discard.push_back(*it);
  state.hand.erase(it);
  state.events.emplace_back("cost");
  return true;
}

bool search_deck_to_hand(void* opaque, const Card card) {
  auto& state = *static_cast<FakeGuzmaHalaState*>(opaque);
  if (!state.search_started) {
    throw std::runtime_error("Guzma & Hala searched before establishing K1.");
  }
  state.events.emplace_back("target");
  const auto it = std::find(state.deck.begin(), state.deck.end(), card);
  if (it == state.deck.end()) return false;
  state.hand.push_back(*it);
  state.deck.erase(it);
  return true;
}

void shuffle_deck(void* opaque) {
  auto& state = *static_cast<FakeGuzmaHalaState*>(opaque);
  state.shuffled = true;
  state.events.emplace_back("shuffle");
}

bool is_basic_pokemon(const void*, const Card card) {
  return sim::is_basic(card);
}

void begin_deck_search(void* opaque, const std::string_view reason) {
  auto& state = *static_cast<FakeGuzmaHalaState*>(opaque);
  if (reason != "Guzma & Hala") {
    throw std::runtime_error("Guzma & Hala changed its deck-search reason.");
  }
  state.search_started = true;
  state.events.emplace_back("search");
}

bool is_stadium(const void*, const Card card) {
  return sim::is_stadium(card);
}

bool is_pokemon_tool(const void*, const Card card) {
  return sim::is_tool(card);
}

bool is_special_energy(const void*, const Card card) {
  return sim::is_special_energy(card);
}

sim::rules::CardContext make_context(FakeGuzmaHalaState& state) {
  return sim::rules::CardContext{
      &state, &hand_count, &move_hand_to_discard, &discard_from_hand,
      &search_deck_to_hand, &shuffle_deck, &is_basic_pokemon,
      &begin_deck_search, &is_stadium, &is_pokemon_tool,
      &is_special_energy};
}

sim::cards::GuzmaHala::SearchTargets choose_full_bonus(void* opaque) {
  auto& state = *static_cast<FakeGuzmaHalaState*>(opaque);
  state.selector_saw_search_started = state.search_started;
  return {.stadium = Card::ChaoticSwell,
          .tool = Card::ForestSealStone,
          .special_energy = Card::DoubleDragonEnergy};
}

sim::cards::GuzmaHala::SearchTargets choose_invalid_categories(void*) {
  return {.stadium = Card::Grass,
          .tool = Card::Crispin,
          .special_energy = Card::Fire};
}

sim::cards::GuzmaHala::SearchTargets choose_nothing(void*) {
  return {};
}

void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

void test_registry_metadata_and_supporter_classification() {
  const auto* definition = sim::cards::find_definition(Card::GuzmaHala);
  require(definition != nullptr, "Guzma & Hala must be explicitly registered.");
  require(definition->canonical_id == "sm12-229",
          "Guzma & Hala canonical print changed.");
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Guzma & Hala must remain a Supporter.");
  require(sim::is_supporter(Card::GuzmaHala),
          "Compatibility classification must source registered Supporter metadata.");
  require(!sim::is_item(Card::GuzmaHala),
          "Guzma & Hala must never be classified as an Item.");
  require(sim::name(Card::GuzmaHala) == "Guzma & Hala",
          "Compatibility name must source the registered definition.");
}

void test_bonus_cost_precedes_one_k1_search_and_one_shuffle() {
  FakeGuzmaHalaState state{
      .hand = {Card::GuzmaHala, Card::Grass, Card::Fire},
      .deck = {Card::ChaoticSwell, Card::ForestSealStone,
               Card::DoubleDragonEnergy, Card::RegidragoV},
  };
  auto context = make_context(state);
  sim::cards::GuzmaHala::Action action{
      .use_bonus = true,
      .discards = {Card::Grass, Card::Fire},
      .search_context = &state,
      .choose_search_targets = &choose_full_bonus,
  };

  const auto resolution = sim::cards::GuzmaHala::resolve(context, action);
  require(resolution.played, "A payable Guzma & Hala bonus action must resolve.");
  require(state.selector_saw_search_started,
          "Target strategy must run only after legal deck inspection begins.");
  require(resolution.found_stadium && resolution.found_tool &&
              resolution.found_special_energy,
          "All three legal bonus categories must move to hand.");
  require(state.events == std::vector<std::string>{
              "source", "cost", "cost", "search", "target", "target",
              "target", "shuffle"},
          "Guzma & Hala cost/search/shuffle ordering changed.");
  require(state.shuffled, "Guzma & Hala must shuffle exactly after the combined search.");
}

void test_second_guzma_hala_can_be_an_other_card_cost() {
  FakeGuzmaHalaState state{
      .hand = {Card::GuzmaHala, Card::GuzmaHala, Card::Grass},
      .deck = {Card::ChaoticSwell},
  };
  auto context = make_context(state);
  sim::cards::GuzmaHala::Action action{
      .use_bonus = true,
      .discards = {Card::GuzmaHala, Card::Grass},
      .search_context = &state,
      .choose_search_targets = &choose_nothing,
  };

  const auto resolution = sim::cards::GuzmaHala::resolve(context, action);
  require(resolution.played,
          "A second physical Guzma & Hala must be legal as an other-card cost.");
  require(state.hand.empty(), "Source and both optional costs must leave hand.");
  require(std::count(state.discard.begin(), state.discard.end(), Card::GuzmaHala) == 2,
          "The source and cost copies must both enter discard.");
}

void test_played_copy_cannot_pay_its_own_bonus_cost() {
  FakeGuzmaHalaState state{
      .hand = {Card::GuzmaHala, Card::Grass},
      .deck = {Card::ChaoticSwell},
  };
  auto context = make_context(state);
  sim::cards::GuzmaHala::Action action{
      .use_bonus = true,
      .discards = {Card::GuzmaHala, Card::Grass},
      .search_context = &state,
      .choose_search_targets = &choose_nothing,
  };

  const auto resolution = sim::cards::GuzmaHala::resolve(context, action);
  require(!resolution.played,
          "The played Guzma & Hala cannot count as one of its two other-card costs.");
  require(state.hand == std::vector<Card>({Card::GuzmaHala, Card::Grass}),
          "Rejected bonus action must preserve hand state.");
  require(state.discard.empty() && !state.search_started && !state.shuffled,
          "Rejected bonus action must not begin resolution.");
}

void test_no_bonus_searches_only_stadium_category() {
  FakeGuzmaHalaState state{
      .hand = {Card::GuzmaHala},
      .deck = {Card::ChaoticSwell, Card::ForestSealStone,
               Card::DoubleDragonEnergy},
  };
  auto context = make_context(state);
  sim::cards::GuzmaHala::Action action{
      .use_bonus = false,
      .search_context = &state,
      .choose_search_targets = &choose_full_bonus,
  };

  const auto resolution = sim::cards::GuzmaHala::resolve(context, action);
  require(resolution.played && resolution.found_stadium,
          "Base Guzma & Hala effect must search its Stadium.");
  require(!resolution.found_tool && !resolution.found_special_energy,
          "Tool and Special Energy searches require the optional two-card cost.");
  require(std::find(state.deck.begin(), state.deck.end(), Card::ForestSealStone) !=
              state.deck.end() &&
              std::find(state.deck.begin(), state.deck.end(),
                        Card::DoubleDragonEnergy) != state.deck.end(),
          "No-bonus resolution must leave bonus-only categories in deck.");
}

void test_invalid_strategy_categories_are_filtered() {
  FakeGuzmaHalaState state{
      .hand = {Card::GuzmaHala, Card::Grass, Card::Fire},
      .deck = {Card::Grass, Card::Crispin, Card::Fire},
  };
  auto context = make_context(state);
  sim::cards::GuzmaHala::Action action{
      .use_bonus = true,
      .discards = {Card::Grass, Card::Fire},
      .search_context = &state,
      .choose_search_targets = &choose_invalid_categories,
  };

  const auto resolution = sim::cards::GuzmaHala::resolve(context, action);
  require(resolution.played,
          "A legal hidden-information search may resolve with no selected targets.");
  require(!resolution.search_targets.stadium && !resolution.search_targets.tool &&
              !resolution.search_targets.special_energy,
          "Card code must reject targets outside each printed category.");
  require(state.deck.size() == 3U && state.shuffled,
          "Invalid target categories must not move deck cards and must still shuffle.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    test_bonus_cost_precedes_one_k1_search_and_one_shuffle();
    test_second_guzma_hala_can_be_an_other_card_cost();
    test_played_copy_cannot_pay_its_own_bonus_cost();
    test_no_bonus_searches_only_stadium_category();
    test_invalid_strategy_categories_are_filtered();
    std::cout << "Guzma & Hala card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
