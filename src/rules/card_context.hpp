#pragma once

#include <string_view>

#include "../cards/card_id.hpp"

namespace sim::rules {

// Narrow rules-facing interface used by migrated card effects. The trace engine
// supplies these callbacks through its compatibility adapter. Card modules never
// receive the simulator's raw State or zone containers.
class CardContext final {
 public:
  using HandCountFn = int (*)(const void*, Card);
  using MoveHandToDiscardFn = bool (*)(void*, Card);
  using DiscardFromHandFn = bool (*)(void*, Card, std::string_view,
                                     std::string_view);
  using SearchDeckToHandFn = bool (*)(void*, Card);
  using ShuffleDeckFn = void (*)(void*);
  using IsBasicPokemonFn = bool (*)(const void*, Card);
  using BeginDeckSearchFn = void (*)(void*, std::string_view);
  using IsStadiumFn = bool (*)(const void*, Card);
  using IsPokemonToolFn = bool (*)(const void*, Card);
  using IsSpecialEnergyFn = bool (*)(const void*, Card);

  constexpr CardContext(void* opaque, HandCountFn hand_count_fn,
                        MoveHandToDiscardFn move_hand_to_discard_fn,
                        DiscardFromHandFn discard_from_hand_fn,
                        SearchDeckToHandFn search_deck_to_hand_fn,
                        ShuffleDeckFn shuffle_deck_fn,
                        IsBasicPokemonFn is_basic_pokemon_fn,
                        BeginDeckSearchFn begin_deck_search_fn,
                        IsStadiumFn is_stadium_fn = nullptr,
                        IsPokemonToolFn is_pokemon_tool_fn = nullptr,
                        IsSpecialEnergyFn is_special_energy_fn = nullptr)
      : opaque_(opaque),
        hand_count_(hand_count_fn),
        move_hand_to_discard_(move_hand_to_discard_fn),
        discard_from_hand_(discard_from_hand_fn),
        search_deck_to_hand_(search_deck_to_hand_fn),
        shuffle_deck_(shuffle_deck_fn),
        is_basic_pokemon_(is_basic_pokemon_fn),
        begin_deck_search_(begin_deck_search_fn),
        is_stadium_(is_stadium_fn),
        is_pokemon_tool_(is_pokemon_tool_fn),
        is_special_energy_(is_special_energy_fn) {}

  int hand_count(const Card card) const {
    return hand_count_(static_cast<const void*>(opaque_), card);
  }

  bool move_hand_to_discard(const Card card) {
    return move_hand_to_discard_(opaque_, card);
  }

  bool discard_from_hand(const Card card, const std::string_view reason,
                         const std::string_view rules_reference) {
    return discard_from_hand_(opaque_, card, reason, rules_reference);
  }

  bool search_deck_to_hand(const Card card) {
    return search_deck_to_hand_(opaque_, card);
  }

  void shuffle_deck() { shuffle_deck_(opaque_); }

  bool is_basic_pokemon(const Card card) const {
    return is_basic_pokemon_(static_cast<const void*>(opaque_), card);
  }

  void begin_deck_search(const std::string_view reason) {
    begin_deck_search_(opaque_, reason);
  }

  bool is_stadium(const Card card) const {
    return is_stadium_ != nullptr &&
           is_stadium_(static_cast<const void*>(opaque_), card);
  }

  bool is_pokemon_tool(const Card card) const {
    return is_pokemon_tool_ != nullptr &&
           is_pokemon_tool_(static_cast<const void*>(opaque_), card);
  }

  bool is_special_energy(const Card card) const {
    return is_special_energy_ != nullptr &&
           is_special_energy_(static_cast<const void*>(opaque_), card);
  }

 private:
  void* opaque_;
  HandCountFn hand_count_;
  MoveHandToDiscardFn move_hand_to_discard_;
  DiscardFromHandFn discard_from_hand_;
  SearchDeckToHandFn search_deck_to_hand_;
  ShuffleDeckFn shuffle_deck_;
  IsBasicPokemonFn is_basic_pokemon_;
  BeginDeckSearchFn begin_deck_search_;
  IsStadiumFn is_stadium_;
  IsPokemonToolFn is_pokemon_tool_;
  IsSpecialEnergyFn is_special_energy_;
};

}  // namespace sim::rules
