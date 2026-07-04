#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace sim {

enum class Card : std::uint8_t {
  RegidragoV,
  RegidragoVstar,
  Dragapult,
  MegaDragonite,
  DialgaGX,
  GoodraVstar,
  TapuLeleGX,
  LatiasEx,
  MawileGX,
  Oricorio,
  Dipplin,
  BrilliantBlender,
  MysteriousTreasure,
  QuickBall,
  UltraBall,
  EvolutionIncense,
  EarthenVessel,
  Arven,
  Crispin,
  ProfessorBurnet,
  Serena,
  TateLiza,
  StevensResolve,
  Guzma,
  Channeler,
  Gladion,
  Lusamine,
  TeamYellsCheer,
  RoseannesBackup,
  ProfessorTuro,
  ForestSealStone,
  Powerglass,
  FieldBlower,
  ChaoticSwell,
  PathToPeak,
  HisuianHeavyBall,
  Grass,
  Fire,
};

constexpr std::array<std::pair<Card, int>, 36> kDeckRecipe{{
    {Card::RegidragoV, 4},       {Card::RegidragoVstar, 3}, {Card::Dragapult, 2},
    {Card::MegaDragonite, 2},    {Card::DialgaGX, 1},       {Card::GoodraVstar, 1},
    {Card::TapuLeleGX, 2},       {Card::LatiasEx, 1},        {Card::MawileGX, 1},
    {Card::Oricorio, 1},         {Card::Dipplin, 1},         {Card::BrilliantBlender, 1},
    {Card::MysteriousTreasure, 4},{Card::QuickBall, 3},       {Card::EarthenVessel, 2},
    {Card::Arven, 2},            {Card::Crispin, 2},         {Card::ProfessorBurnet, 1},
    {Card::Serena, 1},           {Card::TateLiza, 1},        {Card::StevensResolve, 1},
    {Card::Guzma, 1},            {Card::Channeler, 1},       {Card::Gladion, 2},
    {Card::Lusamine, 1},         {Card::TeamYellsCheer, 1},  {Card::RoseannesBackup, 1},
    {Card::ProfessorTuro, 1},    {Card::ForestSealStone, 1}, {Card::Powerglass, 1},
    {Card::FieldBlower, 1},      {Card::ChaoticSwell, 1},    {Card::PathToPeak, 1},
    {Card::HisuianHeavyBall, 1}, {Card::Grass, 6},           {Card::Fire, 3},
}};

constexpr std::string_view name(Card c) {
  switch (c) {
    case Card::RegidragoV: return "Regidrago V";
    case Card::RegidragoVstar: return "Regidrago VSTAR";
    case Card::Dragapult: return "Dragapult ex";
    case Card::MegaDragonite: return "Mega Dragonite ex";
    case Card::DialgaGX: return "Dialga-GX";
    case Card::GoodraVstar: return "Hisuian Goodra VSTAR";
    case Card::TapuLeleGX: return "Tapu Lele-GX";
    case Card::LatiasEx: return "Latias ex";
    case Card::MawileGX: return "Mawile-GX";
    case Card::Oricorio: return "Oricorio GRI 55";
    case Card::Dipplin: return "Dipplin TWM 127";
    case Card::BrilliantBlender: return "Brilliant Blender";
    case Card::MysteriousTreasure: return "Mysterious Treasure";
    case Card::QuickBall: return "Quick Ball";
    case Card::UltraBall: return "Ultra Ball";
    case Card::EvolutionIncense: return "Evolution Incense";
    case Card::EarthenVessel: return "Earthen Vessel";
    case Card::Arven: return "Arven";
    case Card::Crispin: return "Crispin";
    case Card::ProfessorBurnet: return "Professor Burnet";
    case Card::Serena: return "Serena";
    case Card::TateLiza: return "Tate & Liza";
    case Card::StevensResolve: return "Steven's Resolve";
    case Card::Guzma: return "Guzma";
    case Card::Channeler: return "Channeler";
    case Card::Gladion: return "Gladion";
    case Card::Lusamine: return "Lusamine";
    case Card::TeamYellsCheer: return "Team Yell's Cheer";
    case Card::RoseannesBackup: return "Roseanne's Backup";
    case Card::ProfessorTuro: return "Professor Turo's Scenario";
    case Card::ForestSealStone: return "Forest Seal Stone";
    case Card::Powerglass: return "Powerglass";
    case Card::FieldBlower: return "Field Blower";
    case Card::ChaoticSwell: return "Chaotic Swell";
    case Card::PathToPeak: return "Path to the Peak";
    case Card::HisuianHeavyBall: return "Hisuian Heavy Ball";
    case Card::Grass: return "Grass Energy";
    case Card::Fire: return "Fire Energy";
  }
  return "Unknown";
}

constexpr bool is_basic(Card c) {
  switch (c) {
    case Card::RegidragoV:
    case Card::DialgaGX:
    case Card::TapuLeleGX:
    case Card::LatiasEx:
    case Card::MawileGX:
    case Card::Oricorio:
      return true;
    default:
      return false;
  }
}

constexpr bool is_dragon_or_psychic(Card c) {
  switch (c) {
    case Card::RegidragoV:
    case Card::RegidragoVstar:
    case Card::Dragapult:
    case Card::MegaDragonite:
    case Card::DialgaGX:
    case Card::GoodraVstar:
    case Card::TapuLeleGX:
    case Card::LatiasEx:
    case Card::Oricorio:
    case Card::Dipplin:
      return true;
    default:
      return false;
  }
}

constexpr bool is_supporter(Card c) {
  switch (c) {
    case Card::Arven:
    case Card::Crispin:
    case Card::ProfessorBurnet:
    case Card::Serena:
    case Card::TateLiza:
    case Card::StevensResolve:
    case Card::Guzma:
    case Card::Channeler:
    case Card::Gladion:
    case Card::Lusamine:
    case Card::TeamYellsCheer:
    case Card::RoseannesBackup:
    case Card::ProfessorTuro:
      return true;
    default:
      return false;
  }
}

constexpr bool is_payload_as(Card c) {
  return c == Card::Dragapult || c == Card::MegaDragonite || c == Card::DialgaGX ||
         c == Card::GoodraVstar;
}

constexpr bool is_payload_s(Card c) {
  return c == Card::Dragapult || c == Card::MegaDragonite;
}

constexpr bool is_energy(Card c) { return c == Card::Grass || c == Card::Fire; }

enum class DciProfile : std::uint8_t { StrictJit, MatchupFlexJit, NoDiscardControl };

enum class LockMode : std::uint8_t { None, TurnTwoItem, FullItem, FullRuleBoxAbility, FullCombined };

struct Scenario {
  std::string label;
  DciProfile dci;
  LockMode locks;
  bool going_first;
  int max_turn{4};
};

using DeckRecipe = std::vector<std::pair<Card, int>>;

struct Pokemon {
  Card card{};
  int entered_turn{0};
  int grass{0};
  int fire{0};
  bool fss_attached{false};
  bool powerglass_attached{false};
};

struct State {
  std::vector<Card> deck;
  std::vector<Card> hand;
  std::vector<Card> prizes;
  std::vector<Card> discard;
  std::vector<Card> discarded_this_turn;
  std::optional<Pokemon> active;
  std::vector<Pokemon> bench;
  bool supporter_used{false};
  bool manual_energy_used{false};
  bool vstar_power_used{false};
  bool turn_ended{false};
  int turn{0};
};

struct TrialOutcome {
  bool ready_by_2{false};
  bool ready_by_3{false};
  bool ready_by_4{false};
  int first_ready_turn{0};
  int mulligans{0};
  bool started_regi{false};
  bool used_fss{false};
  bool used_steven{false};
  bool used_blender{false};
};

class Engine {
 public:
  Engine(const Scenario& scenario, const DeckRecipe& recipe, std::mt19937_64& rng, TrialOutcome& outcome)
      : cfg_(scenario), recipe_(recipe), rng_(rng), out_(outcome) {}

  TrialOutcome run() {
    initialize();
    for (int turn = 1; turn <= cfg_.max_turn; ++turn) {
      state_.turn = turn;
      state_.supporter_used = false;
      state_.manual_energy_used = false;
      state_.turn_ended = false;
      state_.discarded_this_turn.clear();
      draw_one();
      play_turn();
      evaluate_ready();
      if (state_.turn_ended) continue;
    }
    return out_;
  }

 private:
  const Scenario& cfg_;
  const DeckRecipe& recipe_;
  std::mt19937_64& rng_;
  TrialOutcome& out_;
  State state_;

  bool first_player_turn_one() const { return cfg_.going_first && state_.turn == 1; }
  bool supporter_allowed() const { return !state_.supporter_used && !first_player_turn_one() && !state_.turn_ended; }
  bool item_locked() const {
    switch (cfg_.locks) {
      case LockMode::FullItem:
      case LockMode::FullCombined:
        return true;
      case LockMode::TurnTwoItem:
        return state_.turn >= 2;
      default:
        return false;
    }
  }
  bool rule_box_abilities_locked() const {
    return cfg_.locks == LockMode::FullRuleBoxAbility || cfg_.locks == LockMode::FullCombined;
  }
  bool pokemon_abilities_available() const { return !rule_box_abilities_locked(); }

  static int count(const std::vector<Card>& cards, Card c) {
    return static_cast<int>(std::count(cards.begin(), cards.end(), c));
  }
  int deck_count(Card c) const { return count(state_.deck, c); }
  int hand_count(Card c) const { return count(state_.hand, c); }
  int prize_count(Card c) const { return count(state_.prizes, c); }

  static bool erase_one(std::vector<Card>& cards, Card c) {
    const auto it = std::find(cards.begin(), cards.end(), c);
    if (it == cards.end()) return false;
    cards.erase(it);
    return true;
  }

  void shuffle(std::vector<Card>& cards) { std::shuffle(cards.begin(), cards.end(), rng_); }

  void initialize() {
    for (const auto& [card, copies] : recipe_) {
      for (int i = 0; i < copies; ++i) state_.deck.push_back(card);
    }
    shuffle(state_.deck);
    while (true) {
      state_.hand.clear();
      for (int i = 0; i < 7; ++i) draw_one();
      if (std::any_of(state_.hand.begin(), state_.hand.end(), is_basic)) break;
      state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
      state_.hand.clear();
      shuffle(state_.deck);
      ++out_.mulligans;
    }

    select_opening_active();
    bench_opening_basics();
    shuffle(state_.deck);
    for (int i = 0; i < 6; ++i) {
      state_.prizes.push_back(state_.deck.back());
      state_.deck.pop_back();
    }
  }

  void draw_one() {
    if (state_.deck.empty()) return;
    state_.hand.push_back(state_.deck.back());
    state_.deck.pop_back();
  }

  void select_opening_active() {
    const std::array<Card, 6> priority{Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX,
                                       Card::Oricorio, Card::MawileGX, Card::DialgaGX};
    for (Card candidate : priority) {
      if (erase_one(state_.hand, candidate)) {
        state_.active = Pokemon{candidate, 0};
        out_.started_regi = candidate == Card::RegidragoV;
        return;
      }
    }
  }

  int bench_size() const { return static_cast<int>(state_.bench.size()); }
  bool has_in_play(Card c) const {
    if (state_.active.has_value() && state_.active->card == c) return true;
    return std::any_of(state_.bench.begin(), state_.bench.end(), [c](const Pokemon& p) { return p.card == c; });
  }
  bool has_regi_in_play() const { return has_in_play(Card::RegidragoV) || has_in_play(Card::RegidragoVstar); }
  bool has_regi_v_in_play() const { return has_in_play(Card::RegidragoV); }
  bool has_regi_vstar_active() const { return state_.active.has_value() && state_.active->card == Card::RegidragoVstar; }

  Pokemon* primary_regi() {
    if (state_.active.has_value() && (state_.active->card == Card::RegidragoV || state_.active->card == Card::RegidragoVstar)) {
      return &*state_.active;
    }
    for (Pokemon& p : state_.bench) {
      if (p.card == Card::RegidragoV || p.card == Card::RegidragoVstar) return &p;
    }
    return nullptr;
  }

  const Pokemon* primary_regi() const {
    if (state_.active.has_value() && (state_.active->card == Card::RegidragoV || state_.active->card == Card::RegidragoVstar)) {
      return &*state_.active;
    }
    for (const Pokemon& p : state_.bench) {
      if (p.card == Card::RegidragoV || p.card == Card::RegidragoVstar) return &p;
    }
    return nullptr;
  }

  bool bench_from_hand(Card c) {
    if (bench_size() >= 5 || !is_basic(c) || !erase_one(state_.hand, c)) return false;
    state_.bench.push_back(Pokemon{c, state_.turn});
    trigger_bench_ability(state_.bench.back());
    return true;
  }

  void bench_opening_basics() {
    while (bench_size() < 5 && hand_count(Card::RegidragoV) > 0 && !has_in_play(Card::RegidragoV)) {
      bench_from_hand(Card::RegidragoV);
    }
    while (bench_size() < 5 && hand_count(Card::RegidragoV) > 0 &&
           std::count_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoV; }) < 2) {
      bench_from_hand(Card::RegidragoV);
    }
    for (Card c : {Card::TapuLeleGX, Card::Oricorio, Card::LatiasEx}) {
      if (bench_size() < 5 && hand_count(c) > 0 && !has_in_play(c)) {
        erase_one(state_.hand, c);
        state_.bench.push_back(Pokemon{c, 0});
      }
    }
  }

  Card choose_supporter_target() const {
    const Pokemon* regi = primary_regi();
    const bool attack_turn = state_.turn >= 2;
    const bool need_energy = regi == nullptr || regi->grass < 2 || regi->fire < 1;
    const bool missing_vstar = has_regi_v_in_play() && !has_in_play(Card::RegidragoVstar);
    const bool payload_needed = !has_payload_in_discard_for_goal();

    if (state_.turn == 1 && !cfg_.going_first && has_regi_v_in_play() && hand_count(Card::StevensResolve) == 0 &&
        deck_count(Card::StevensResolve) > 0) {
      return Card::StevensResolve;
    }
    if (attack_turn && need_energy && deck_count(Card::Crispin) > 0) return Card::Crispin;
    if (payload_needed && item_locked() && deck_count(Card::ProfessorBurnet) > 0) return Card::ProfessorBurnet;
    if (missing_vstar && prize_count(Card::RegidragoVstar) > 0 && deck_count(Card::Gladion) > 0) return Card::Gladion;
    if (!has_regi_in_play() && prize_count(Card::RegidragoV) > 0 && deck_count(Card::Gladion) > 0) return Card::Gladion;
    if (deck_count(Card::Arven) > 0) return Card::Arven;
    if (deck_count(Card::Crispin) > 0) return Card::Crispin;
    return Card::TateLiza;
  }

  void trigger_bench_ability(const Pokemon& pokemon) {
    if (!pokemon_abilities_available()) return;
    if (pokemon.card == Card::TapuLeleGX) {
      const Card target = choose_supporter_target();
      if (deck_count(target) > 0) search_deck_to_hand(target);
    } else if (pokemon.card == Card::Oricorio) {
      for (Card energy : {Card::Grass, Card::Fire}) {
        if (deck_count(energy) > 0) search_deck_to_hand(energy);
      }
    }
  }

  void search_deck_to_hand(Card c) {
    if (erase_one(state_.deck, c)) {
      state_.hand.push_back(c);
      shuffle(state_.deck);
    }
  }

  bool take_from_deck_to_discard(Card c) {
    if (!erase_one(state_.deck, c)) return false;
    state_.discard.push_back(c);
    state_.discarded_this_turn.push_back(c);
    shuffle(state_.deck);
    return true;
  }

  bool has_latias_in_play() const { return has_in_play(Card::LatiasEx); }

  bool move_regi_to_active() {
    if (has_regi_vstar_active()) return true;
    if (!has_latias_in_play() || !pokemon_abilities_available() || !state_.active.has_value()) return false;
    auto it = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) {
      return p.card == Card::RegidragoVstar;
    });
    if (it == state_.bench.end()) {
      it = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoV; });
    }
    if (it == state_.bench.end()) return false;
    std::swap(*state_.active, *it);
    return true;
  }

  bool manually_attach() {
    if (state_.manual_energy_used) return false;
    Pokemon* regi = primary_regi();
    if (regi == nullptr) return false;
    Card wanted = regi->grass < 2 ? Card::Grass : (regi->fire < 1 ? Card::Fire : Card::Grass);
    if (hand_count(wanted) == 0) {
      wanted = wanted == Card::Grass ? Card::Fire : Card::Grass;
    }
    if (hand_count(wanted) == 0) return false;
    erase_one(state_.hand, wanted);
    if (wanted == Card::Grass) ++regi->grass;
    else ++regi->fire;
    state_.manual_energy_used = true;
    return true;
  }

  bool payload_in_discard() const {
    return std::any_of(state_.discard.begin(), state_.discard.end(), is_payload_as);
  }

  bool has_payload_in_discard_for_goal() const {
    if (!payload_in_discard()) return false;
    if (cfg_.dci == DciProfile::NoDiscardControl) return true;
    return std::any_of(state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), is_payload_as);
  }

  bool can_discard_payload_now() const { return state_.turn >= 2; }

  std::optional<Card> choose_discard(bool allow_current_payload) const {
    if (allow_current_payload) {
      for (Card c : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
        if (hand_count(c) > 0) return c;
      }
    }

    auto can_spend_energy = [this](Card energy) {
      const Pokemon* regi = primary_regi();
      const int need_g = regi == nullptr ? 2 : std::max(0, 2 - regi->grass);
      const int need_f = regi == nullptr ? 1 : std::max(0, 1 - regi->fire);
      const int held = hand_count(energy);
      const int decked = deck_count(energy);
      const int need = energy == Card::Grass ? need_g : need_f;
      return held + decked > need + 1 && held > 0;
    };

    if (cfg_.dci != DciProfile::StrictJit) {
      for (Card c : {Card::MawileGX, Card::Dipplin, Card::Guzma, Card::Channeler, Card::FieldBlower,
                     Card::ChaoticSwell, Card::PathToPeak, Card::TeamYellsCheer, Card::RoseannesBackup,
                     Card::Lusamine, Card::ProfessorTuro, Card::Powerglass}) {
        if (hand_count(c) > 0) return c;
      }
      if (hand_count(Card::GoodraVstar) > 0 && deck_count(Card::Dragapult) + deck_count(Card::MegaDragonite) +
                                                    hand_count(Card::Dragapult) + hand_count(Card::MegaDragonite) > 0) {
        return Card::GoodraVstar;
      }
    }

    if (hand_count(Card::RegidragoV) > 0 && (has_regi_in_play() || hand_count(Card::RegidragoV) >= 2)) return Card::RegidragoV;
    if (hand_count(Card::RegidragoVstar) > 0 && (has_in_play(Card::RegidragoVstar) || hand_count(Card::RegidragoVstar) >= 2)) return Card::RegidragoVstar;
    if (can_spend_energy(Card::Grass)) return Card::Grass;
    if (can_spend_energy(Card::Fire)) return Card::Fire;

    if (cfg_.dci == DciProfile::NoDiscardControl) {
      for (Card c : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
        if (hand_count(c) > 0) return c;
      }
    }
    return std::nullopt;
  }

  bool discard_from_hand(Card c) {
    if (!erase_one(state_.hand, c)) return false;
    state_.discard.push_back(c);
    state_.discarded_this_turn.push_back(c);
    return true;
  }

  bool has_hand_payload() const {
    return std::any_of(state_.hand.begin(), state_.hand.end(), is_payload_as);
  }

  std::optional<Card> any_basic_in_deck() const {
    for (Card c : {Card::RegidragoV, Card::TapuLeleGX, Card::LatiasEx, Card::Oricorio, Card::MawileGX, Card::DialgaGX}) {
      if (deck_count(c) > 0) return c;
    }
    return std::nullopt;
  }

  std::optional<Card> any_dragon_or_psychic_in_deck() const {
    for (Card c : {Card::RegidragoVstar, Card::RegidragoV, Card::MegaDragonite, Card::Dragapult,
                   Card::GoodraVstar, Card::DialgaGX, Card::TapuLeleGX, Card::LatiasEx, Card::Oricorio,
                   Card::Dipplin}) {
      if (deck_count(c) > 0) return c;
    }
    return std::nullopt;
  }

  std::optional<Card> payload_in_deck() const {
    for (Card c : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
      if (deck_count(c) > 0) return c;
    }
    return std::nullopt;
  }

  bool has_followup_payload_outlet() const {
    if (hand_count(Card::QuickBall) > 0 || hand_count(Card::EarthenVessel) > 0) return true;
    if (supporter_allowed() && hand_count(Card::Serena) > 0) return true;
    return hand_count(Card::UltraBall) > 0;
  }

  bool play_mysterious_treasure() {
    if (item_locked() || hand_count(Card::MysteriousTreasure) == 0) return false;
    const bool needs_payload = can_discard_payload_now() && !has_payload_in_discard_for_goal();
    std::optional<Card> target;
    if (!has_regi_in_play()) target = Card::RegidragoV;
    else if (!has_in_play(Card::RegidragoVstar)) target = Card::RegidragoVstar;
    else if (!has_latias_in_play() && !has_regi_vstar_active()) target = Card::LatiasEx;
    else if (hand_count(Card::TapuLeleGX) == 0 && deck_count(Card::TapuLeleGX) > 0 && hand_count(Card::Crispin) == 0) target = Card::TapuLeleGX;
    else if (needs_payload && has_hand_payload()) target = any_dragon_or_psychic_in_deck();
    else if (needs_payload && has_followup_payload_outlet()) target = payload_in_deck();
    if (!target.has_value() || deck_count(*target) == 0) return false;
    const auto discard = choose_discard(needs_payload);
    if (!discard.has_value()) return false;
    erase_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(*discard);
    search_deck_to_hand(*target);
    return true;
  }

  bool play_quick_ball() {
    if (item_locked() || hand_count(Card::QuickBall) == 0) return false;
    const bool needs_payload = can_discard_payload_now() && !has_payload_in_discard_for_goal();
    std::optional<Card> target;
    if (!has_regi_in_play()) target = Card::RegidragoV;
    else if (!has_latias_in_play() && !has_regi_vstar_active()) target = Card::LatiasEx;
    else if (hand_count(Card::TapuLeleGX) == 0 && deck_count(Card::TapuLeleGX) > 0 && hand_count(Card::Crispin) == 0) target = Card::TapuLeleGX;
    else if (needs_payload && has_hand_payload()) target = any_basic_in_deck();
    else if (needs_payload && has_followup_payload_outlet() && deck_count(Card::DialgaGX) > 0) target = Card::DialgaGX;
    if (!target.has_value() || deck_count(*target) == 0) return false;
    const auto discard = choose_discard(needs_payload);
    if (!discard.has_value()) return false;
    erase_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    discard_from_hand(*discard);
    search_deck_to_hand(*target);
    return true;
  }

  bool play_ultra_ball() {
    if (item_locked() || hand_count(Card::UltraBall) == 0) return false;
    Card target = Card::RegidragoV;
    if (!has_regi_in_play()) target = Card::RegidragoV;
    else if (!has_in_play(Card::RegidragoVstar)) target = Card::RegidragoVstar;
    else if (!has_latias_in_play() && !has_regi_vstar_active()) target = Card::LatiasEx;
    else if (hand_count(Card::TapuLeleGX) == 0 && deck_count(Card::TapuLeleGX) > 0 && hand_count(Card::Crispin) == 0) target = Card::TapuLeleGX;
    else return false;
    if (deck_count(target) == 0) return false;

    const bool allow_payload = can_discard_payload_now() && !has_payload_in_discard_for_goal();
    const auto first = choose_discard(allow_payload);
    if (!first.has_value()) return false;
    erase_one(state_.hand, *first);
    const auto second = choose_discard(allow_payload && !is_payload_as(*first));
    state_.hand.push_back(*first);
    if (!second.has_value()) return false;

    erase_one(state_.hand, Card::UltraBall);
    state_.discard.push_back(Card::UltraBall);
    discard_from_hand(*first);
    discard_from_hand(*second);
    search_deck_to_hand(target);
    return true;
  }

  bool play_evolution_incense() {
    if (item_locked() || hand_count(Card::EvolutionIncense) == 0 || has_in_play(Card::RegidragoVstar) ||
        deck_count(Card::RegidragoVstar) == 0) {
      return false;
    }
    erase_one(state_.hand, Card::EvolutionIncense);
    state_.discard.push_back(Card::EvolutionIncense);
    search_deck_to_hand(Card::RegidragoVstar);
    return true;
  }

  bool play_earthen_vessel() {
    if (item_locked() || hand_count(Card::EarthenVessel) == 0) return false;
    Pokemon* regi = primary_regi();
    const bool need_energy = regi == nullptr || regi->grass < 2 || regi->fire < 1;
    const bool can_be_jit_outlet = can_discard_payload_now() && !has_payload_in_discard_for_goal();
    if (!need_energy && !can_be_jit_outlet) return false;
    const auto discard = choose_discard(can_be_jit_outlet);
    if (!discard.has_value()) return false;
    erase_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    discard_from_hand(*discard);
    for (Card energy : {Card::Grass, Card::Fire}) {
      if (deck_count(energy) > 0) search_deck_to_hand(energy);
    }
    return true;
  }

  bool play_heavy_ball() {
    if (item_locked() || hand_count(Card::HisuianHeavyBall) == 0 || has_regi_in_play()) return false;
    auto it = std::find(state_.prizes.begin(), state_.prizes.end(), Card::RegidragoV);
    if (it == state_.prizes.end()) return false;
    erase_one(state_.hand, Card::HisuianHeavyBall);
    Card revealed = *it;
    *it = Card::HisuianHeavyBall;
    state_.hand.push_back(revealed);
    shuffle(state_.prizes);
    return true;
  }

  bool play_blender() {
    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 || !can_discard_payload_now() ||
        has_payload_in_discard_for_goal()) {
      return false;
    }
    Card target = Card::MegaDragonite;
    if (deck_count(Card::MegaDragonite) == 0) target = Card::Dragapult;
    if (deck_count(target) == 0) target = Card::GoodraVstar;
    if (deck_count(target) == 0) target = Card::DialgaGX;
    if (deck_count(target) == 0) return false;
    erase_one(state_.hand, Card::BrilliantBlender);
    state_.discard.push_back(Card::BrilliantBlender);
    take_from_deck_to_discard(target);
    out_.used_blender = true;
    return true;
  }

  Card fss_target() const {
    const Pokemon* regi = primary_regi();
    if (!has_regi_in_play()) return Card::RegidragoV;
    if (state_.turn == 1 && !cfg_.going_first && has_regi_v_in_play() &&
        (hand_count(Card::RegidragoVstar) == 0 || hand_count(Card::Crispin) == 0 ||
         hand_count(Card::BrilliantBlender) == 0) && deck_count(Card::StevensResolve) > 0) {
      return Card::StevensResolve;
    }
    if (!has_in_play(Card::RegidragoVstar)) return Card::RegidragoVstar;
    if (state_.turn >= 2 && regi != nullptr && (regi->grass < 2 || regi->fire < 1)) return Card::Crispin;
    if (state_.turn >= 2 && !has_payload_in_discard_for_goal()) return Card::BrilliantBlender;
    return Card::Crispin;
  }

  bool use_fss() {
    if (item_locked() || state_.vstar_power_used || hand_count(Card::ForestSealStone) == 0 || !has_regi_v_in_play()) return false;
    Pokemon* holder = nullptr;
    if (state_.active.has_value() && state_.active->card == Card::RegidragoV) holder = &*state_.active;
    else {
      for (Pokemon& p : state_.bench) {
        if (p.card == Card::RegidragoV) { holder = &p; break; }
      }
    }
    if (holder == nullptr || holder->fss_attached || holder->powerglass_attached) return false;
    const Card target = fss_target();
    if (deck_count(target) == 0) return false;
    erase_one(state_.hand, Card::ForestSealStone);
    holder->fss_attached = true;
    search_deck_to_hand(target);
    state_.vstar_power_used = true;
    out_.used_fss = true;
    return true;
  }

  bool play_crispin() {
    if (!supporter_allowed() || hand_count(Card::Crispin) == 0) return false;
    Pokemon* regi = primary_regi();
    if (regi == nullptr || (deck_count(Card::Grass) == 0 && deck_count(Card::Fire) == 0)) return false;
    erase_one(state_.hand, Card::Crispin);
    state_.discard.push_back(Card::Crispin);
    state_.supporter_used = true;

    const bool grass_needed = regi->grass < 2;
    const bool fire_needed = regi->fire < 1;
    Card attach = grass_needed ? Card::Grass : Card::Fire;
    Card hand = attach == Card::Grass ? Card::Fire : Card::Grass;
    if (deck_count(attach) == 0) std::swap(attach, hand);
    if (deck_count(attach) > 0) {
      erase_one(state_.deck, attach);
      if (attach == Card::Grass) ++regi->grass;
      else ++regi->fire;
    }
    if (deck_count(hand) > 0) {
      erase_one(state_.deck, hand);
      state_.hand.push_back(hand);
    }
    shuffle(state_.deck);
    return grass_needed || fire_needed;
  }

  bool play_steven() {
    if (!supporter_allowed() || hand_count(Card::StevensResolve) == 0 || !has_regi_v_in_play()) return false;
    erase_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    out_.used_steven = true;

    for (Card wanted : {Card::RegidragoVstar, Card::Crispin, Card::BrilliantBlender}) {
      if (deck_count(wanted) > 0) search_deck_to_hand(wanted);
    }
    state_.turn_ended = true;
    return true;
  }

  bool play_arven() {
    if (!supporter_allowed() || hand_count(Card::Arven) == 0) return false;
    erase_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;

    Card item = Card::BrilliantBlender;
    if (!has_regi_in_play()) item = Card::QuickBall;
    else if (!has_in_play(Card::RegidragoVstar)) item = Card::MysteriousTreasure;
    else if (primary_regi() != nullptr && (primary_regi()->grass < 2 || primary_regi()->fire < 1)) item = Card::EarthenVessel;
    else if (!has_payload_in_discard_for_goal()) item = Card::BrilliantBlender;

    if (deck_count(item) > 0) search_deck_to_hand(item);
    if (deck_count(Card::ForestSealStone) > 0 && has_regi_v_in_play()) {
      search_deck_to_hand(Card::ForestSealStone);
    } else if (deck_count(Card::Powerglass) > 0) {
      search_deck_to_hand(Card::Powerglass);
    }
    return true;
  }

  bool play_burnet() {
    if (!supporter_allowed() || hand_count(Card::ProfessorBurnet) == 0) return false;
    if (!can_discard_payload_now() || has_payload_in_discard_for_goal()) return false;
    erase_one(state_.hand, Card::ProfessorBurnet);
    state_.discard.push_back(Card::ProfessorBurnet);
    state_.supporter_used = true;
    for (Card c : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
      if (deck_count(c) > 0) {
        take_from_deck_to_discard(c);
        return true;
      }
    }
    return false;
  }

  bool play_gladion() {
    if (!supporter_allowed() || hand_count(Card::Gladion) == 0) return false;
    Card target = Card::RegidragoVstar;
    if (!has_regi_in_play() && prize_count(Card::RegidragoV) > 0) target = Card::RegidragoV;
    else if (prize_count(Card::RegidragoVstar) == 0 && prize_count(Card::Crispin) > 0) target = Card::Crispin;
    else if (prize_count(Card::RegidragoVstar) == 0 && prize_count(Card::BrilliantBlender) > 0) target = Card::BrilliantBlender;
    else if (prize_count(Card::RegidragoVstar) == 0 && prize_count(Card::MegaDragonite) > 0) target = Card::MegaDragonite;
    if (prize_count(target) == 0) return false;
    erase_one(state_.hand, Card::Gladion);
    state_.supporter_used = true;
    auto it = std::find(state_.prizes.begin(), state_.prizes.end(), target);
    state_.hand.push_back(*it);
    *it = Card::Gladion;
    shuffle(state_.prizes);
    return true;
  }

  bool play_serena_draw() {
    if (!supporter_allowed() || hand_count(Card::Serena) == 0) return false;
    const auto discard = choose_discard(can_discard_payload_now() && !has_payload_in_discard_for_goal());
    if (!discard.has_value()) return false;
    erase_one(state_.hand, Card::Serena);
    state_.discard.push_back(Card::Serena);
    discard_from_hand(*discard);
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) draw_one();
    return true;
  }

  bool play_tate_liza_draw() {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0) return false;
    erase_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
    shuffle(state_.deck);
    for (int i = 0; i < 5; ++i) draw_one();
    return true;
  }

  bool evolve_regi() {
    if (state_.turn == 1 || hand_count(Card::RegidragoVstar) == 0) return false;
    auto eligible = [this](Pokemon& p) { return p.card == Card::RegidragoV && p.entered_turn < state_.turn; };
    Pokemon* target = nullptr;
    if (state_.active.has_value() && eligible(*state_.active)) target = &*state_.active;
    if (target == nullptr) {
      for (Pokemon& p : state_.bench) {
        if (eligible(p)) { target = &p; break; }
      }
    }
    if (target == nullptr) return false;
    erase_one(state_.hand, Card::RegidragoVstar);
    target->card = Card::RegidragoVstar;
    return true;
  }

  void play_basics_from_hand() {
    bool changed = true;
    while (changed && bench_size() < 5) {
      changed = false;
      if (!has_regi_in_play() && hand_count(Card::RegidragoV) > 0) {
        changed = bench_from_hand(Card::RegidragoV);
      } else if (has_regi_in_play() && hand_count(Card::RegidragoV) > 0 &&
                 std::count_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoV; }) < 2) {
        changed = bench_from_hand(Card::RegidragoV);
      } else if (!has_latias_in_play() && hand_count(Card::LatiasEx) > 0 && !has_regi_vstar_active()) {
        changed = bench_from_hand(Card::LatiasEx);
      } else if (hand_count(Card::TapuLeleGX) > 0 && !state_.supporter_used) {
        changed = bench_from_hand(Card::TapuLeleGX);
      } else if (hand_count(Card::Oricorio) > 0 && primary_regi() != nullptr &&
                 (primary_regi()->grass < 2 || primary_regi()->fire < 1)) {
        changed = bench_from_hand(Card::Oricorio);
      }
    }
  }

  void use_items_until_stable(bool include_payload) {
    if (item_locked() || state_.turn_ended) return;
    bool changed = true;
    int guard = 0;
    while (changed && guard++ < 20) {
      changed = false;
      play_basics_from_hand();
      if (play_heavy_ball()) { changed = true; continue; }
      if (play_ultra_ball()) { changed = true; continue; }
      if (play_evolution_incense()) { changed = true; continue; }
      if (play_mysterious_treasure()) { changed = true; continue; }
      if (play_quick_ball()) { changed = true; continue; }
      if (play_earthen_vessel()) { changed = true; continue; }
      if (include_payload && play_blender()) { changed = true; continue; }
    }
  }

  void choose_and_play_supporter() {
    if (!supporter_allowed()) return;
    Pokemon* regi = primary_regi();
    const bool needs_energy = regi == nullptr || regi->grass < 2 || regi->fire < 1;
    const bool need_payload = !has_payload_in_discard_for_goal();

    if ((prize_count(Card::RegidragoVstar) > 0 && has_regi_v_in_play()) ||
        (!has_regi_in_play() && prize_count(Card::RegidragoV) > 0)) {
      if (play_gladion()) return;
    }
    if (state_.turn >= 2 && needs_energy && play_crispin()) return;
    if (state_.turn == 1 && !cfg_.going_first && has_regi_v_in_play() &&
        (hand_count(Card::StevensResolve) > 0 || deck_count(Card::StevensResolve) > 0)) {
      if (hand_count(Card::StevensResolve) > 0 && play_steven()) return;
    }
    if (state_.turn >= 2 && need_payload && (item_locked() || hand_count(Card::BrilliantBlender) == 0) && play_burnet()) return;
    if (hand_count(Card::Arven) > 0 && play_arven()) return;
    if (state_.turn >= 2 && needs_energy && play_crispin()) return;
    if (state_.turn >= 2 && need_payload && play_serena_draw()) return;
    if (hand_count(Card::Serena) > 0 && play_serena_draw()) return;
    if (hand_count(Card::TateLiza) > 0) play_tate_liza_draw();
  }

  void play_turn() {
    play_basics_from_hand();
    use_items_until_stable(false);
    play_basics_from_hand();
    use_fss();
    play_basics_from_hand();
    use_items_until_stable(false);

    manually_attach();
    choose_and_play_supporter();
    if (state_.turn_ended) return;

    play_basics_from_hand();
    use_fss();
    play_basics_from_hand();
    use_items_until_stable(false);
    manually_attach();
    evolve_regi();
    play_basics_from_hand();
    use_items_until_stable(true);
    manually_attach();
    move_regi_to_active();
  }

  void evaluate_ready() {
    const Pokemon* active = state_.active.has_value() ? &*state_.active : nullptr;
    const bool ready = active != nullptr && active->card == Card::RegidragoVstar && active->grass >= 2 &&
                       active->fire >= 1 && has_payload_in_discard_for_goal() && state_.turn >= 2;
    if (!ready) return;
    if (out_.first_ready_turn == 0) out_.first_ready_turn = state_.turn;
    if (state_.turn <= 2) out_.ready_by_2 = true;
    if (state_.turn <= 3) out_.ready_by_3 = true;
    if (state_.turn <= 4) out_.ready_by_4 = true;
  }
};

struct Aggregate {
  std::uint64_t trials{0};
  std::uint64_t by2{0};
  std::uint64_t by3{0};
  std::uint64_t by4{0};
  std::uint64_t started_regi{0};
  std::uint64_t fss{0};
  std::uint64_t steven{0};
  std::uint64_t blender{0};
  double total_mulligans{0.0};
};

class OutputLock {
 public:
  explicit OutputLock(const std::filesystem::path& output) {
    const std::filesystem::path lock_path = output.string() + ".lock";
#if defined(_WIN32)
    handle_ = ::CreateFileW(lock_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle_ == INVALID_HANDLE_VALUE) {
      throw std::runtime_error("Could not exclusively lock output: " + output.string());
    }
#else
    fd_ = ::open(lock_path.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd_ < 0 || ::flock(fd_, LOCK_EX) != 0) {
      if (fd_ >= 0) ::close(fd_);
      throw std::runtime_error("Could not exclusively lock output: " + output.string());
    }
#endif
  }

  ~OutputLock() {
#if defined(_WIN32)
    if (handle_ != INVALID_HANDLE_VALUE) ::CloseHandle(handle_);
#else
    if (fd_ >= 0) {
      ::flock(fd_, LOCK_UN);
      ::close(fd_);
    }
#endif
  }

  OutputLock(const OutputLock&) = delete;
  OutputLock& operator=(const OutputLock&) = delete;

 private:
#if defined(_WIN32)
  HANDLE handle_{INVALID_HANDLE_VALUE};
#else
  int fd_{-1};
#endif
};

void write_atomically_locked(const std::filesystem::path& output, const std::string& content) {
  const std::filesystem::path parent = output.parent_path();
  if (!parent.empty()) std::filesystem::create_directories(parent);
  OutputLock lock(output);
  const std::filesystem::path temporary = output.string() + ".tmp." + std::to_string(
#if defined(_WIN32)
      static_cast<unsigned long long>(::GetCurrentProcessId())
#else
      static_cast<unsigned long long>(::getpid())
#endif
  );
  {
    std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
    if (!stream) throw std::runtime_error("Could not create temporary output: " + temporary.string());
    stream.write(content.data(), static_cast<std::streamsize>(content.size()));
    stream.flush();
    if (!stream) throw std::runtime_error("Could not write temporary output: " + temporary.string());
  }
#if defined(_WIN32)
  if (!::MoveFileExW(temporary.c_str(), output.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    std::filesystem::remove(temporary);
    throw std::runtime_error("Could not atomically replace output: " + output.string());
  }
#else
  std::error_code error;
  std::filesystem::rename(temporary, output, error);
  if (error) {
    std::filesystem::remove(temporary);
    throw std::runtime_error("Could not atomically replace output: " + output.string());
  }
#endif
}

std::string csv_field(std::string_view value) {
  std::string quoted;
  quoted.reserve(value.size() + 2);
  quoted.push_back('"');
  for (const char c : value) {
    if (c == '"') quoted.push_back('"');
    quoted.push_back(c);
  }
  quoted.push_back('"');
  return quoted;
}

double pct(std::uint64_t n, std::uint64_t d) { return d == 0 ? 0.0 : 100.0 * static_cast<double>(n) / static_cast<double>(d); }
double se_pp(std::uint64_t n, std::uint64_t d) {
  if (d == 0) return 0.0;
  const double p = static_cast<double>(n) / static_cast<double>(d);
  return 100.0 * std::sqrt(p * (1.0 - p) / static_cast<double>(d));
}

Aggregate simulate(const Scenario& scenario, const DeckRecipe& recipe, std::uint64_t trials, std::uint64_t seed) {
  Aggregate agg{};
  std::mt19937_64 rng(seed);
  for (std::uint64_t i = 0; i < trials; ++i) {
    TrialOutcome outcome{};
    Engine engine(scenario, recipe, rng, outcome);
    outcome = engine.run();
    ++agg.trials;
    agg.by2 += outcome.ready_by_2 ? 1U : 0U;
    agg.by3 += outcome.ready_by_3 ? 1U : 0U;
    agg.by4 += outcome.ready_by_4 ? 1U : 0U;
    agg.started_regi += outcome.started_regi ? 1U : 0U;
    agg.fss += outcome.used_fss ? 1U : 0U;
    agg.steven += outcome.used_steven ? 1U : 0U;
    agg.blender += outcome.used_blender ? 1U : 0U;
    agg.total_mulligans += static_cast<double>(outcome.mulligans);
  }
  return agg;
}

DeckRecipe baseline_recipe() {
  return DeckRecipe(kDeckRecipe.begin(), kDeckRecipe.end());
}

bool adjust(DeckRecipe& recipe, Card card, int delta) {
  auto it = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) { return entry.first == card; });
  if (it == recipe.end()) {
    if (delta <= 0) return false;
    recipe.push_back({card, delta});
    return true;
  }
  if (it->second + delta < 0) return false;
  it->second += delta;
  return true;
}

struct Variant {
  std::string label;
  DeckRecipe recipe;
};

std::vector<Variant> variants() {
  const DeckRecipe baseline = baseline_recipe();
  std::vector<Variant> result{{"baseline", baseline}};
  const auto add_variant = [&result, &baseline](std::string label, std::initializer_list<std::pair<Card, int>> edits) {
    DeckRecipe changed = baseline;
    for (const auto& [card, delta] : edits) {
      if (!adjust(changed, card, delta)) throw std::runtime_error("invalid variant edit");
    }
    const int total = std::accumulate(changed.begin(), changed.end(), 0,
      [](int sum, const auto& entry) { return sum + entry.second; });
    if (total != 60) throw std::runtime_error("variant must contain 60 cards");
    result.push_back({std::move(label), std::move(changed)});
  };
  add_variant("turbo/-Mawile+QuickBall", {{Card::MawileGX, -1}, {Card::QuickBall, 1}});
  add_variant("turbo/-Mawile+Arven", {{Card::MawileGX, -1}, {Card::Arven, 1}});
  add_variant("turbo/-Mawile+Crispin", {{Card::MawileGX, -1}, {Card::Crispin, 1}});
  add_variant("turbo/-Mawile+Stevens", {{Card::MawileGX, -1}, {Card::StevensResolve, 1}});
  add_variant("turbo/-Mawile+EvolutionIncense", {{Card::MawileGX, -1}, {Card::EvolutionIncense, 1}});
  add_variant("turbo/-Mawile+UltraBall", {{Card::MawileGX, -1}, {Card::UltraBall, 1}});
  add_variant("turbo/-Tate+UltraBall", {{Card::TateLiza, -1}, {Card::UltraBall, 1}});
  add_variant("turbo/-Guzma+QuickBall", {{Card::Guzma, -1}, {Card::QuickBall, 1}});
  add_variant("turbo/-FieldBlower+QuickBall", {{Card::FieldBlower, -1}, {Card::QuickBall, 1}});
  add_variant("turbo/-Mawile,-Tate+QuickBall,+Crispin",
              {{Card::MawileGX, -1}, {Card::TateLiza, -1}, {Card::QuickBall, 1}, {Card::Crispin, 1}});
  add_variant("turbo/-Mawile,-Guzma+QuickBall,+Arven",
              {{Card::MawileGX, -1}, {Card::Guzma, -1}, {Card::QuickBall, 1}, {Card::Arven, 1}});
  add_variant("turbo/-Mawile,-Tate+UltraBall,+Crispin",
              {{Card::MawileGX, -1}, {Card::TateLiza, -1}, {Card::UltraBall, 1}, {Card::Crispin, 1}});
  add_variant("realism/-Tate+Burnet", {{Card::TateLiza, -1}, {Card::ProfessorBurnet, 1}});
  add_variant("realism/-Mawile+Burnet", {{Card::MawileGX, -1}, {Card::ProfessorBurnet, 1}});
  add_variant("realism/-Tate+EarthenVessel", {{Card::TateLiza, -1}, {Card::EarthenVessel, 1}});
  add_variant("realism/-Mawile+Serena", {{Card::MawileGX, -1}, {Card::Serena, 1}});
  add_variant("realism/-Mawile,-Tate+Burnet,+Arven",
              {{Card::MawileGX, -1}, {Card::TateLiza, -1}, {Card::ProfessorBurnet, 1}, {Card::Arven, 1}});
  add_variant("realism/-Mawile,-Tate+Burnet,+EarthenVessel",
              {{Card::MawileGX, -1}, {Card::TateLiza, -1}, {Card::ProfessorBurnet, 1}, {Card::EarthenVessel, 1}});
  return result;
}

std::vector<Scenario> scenarios() {
  std::vector<Scenario> result;
  for (bool first : {true, false}) {
    const std::string pos = first ? "go-first" : "go-second";
    result.push_back({"strict-jit/" + pos, DciProfile::StrictJit, LockMode::None, first});
    result.push_back({"matchup-flex-jit/" + pos, DciProfile::MatchupFlexJit, LockMode::None, first});
    result.push_back({"no-discard-control/" + pos, DciProfile::NoDiscardControl, LockMode::None, first});
    result.push_back({"strict-jit-turn2-item-lock/" + pos, DciProfile::StrictJit, LockMode::TurnTwoItem, first});
    result.push_back({"strict-jit-full-item-lock/" + pos, DciProfile::StrictJit, LockMode::FullItem, first});
    result.push_back({"strict-jit-rulebox-ability-lock/" + pos, DciProfile::StrictJit, LockMode::FullRuleBoxAbility, first});
    result.push_back({"strict-jit-combined-lock/" + pos, DciProfile::StrictJit, LockMode::FullCombined, first});
  }
  return result;
}

}  // namespace sim

int main(int argc, char** argv) {
  std::uint64_t trials = 100000;
  std::uint64_t seed = 20260704;
  std::uint64_t variant_trials = 0;
  std::string output = "results/simulation_results.csv";
  std::string variants_output = "results/variant_results.csv";
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--trials" && i + 1 < argc) trials = std::stoull(argv[++i]);
    else if (arg == "--seed" && i + 1 < argc) seed = std::stoull(argv[++i]);
    else if (arg == "--variant-trials" && i + 1 < argc) variant_trials = std::stoull(argv[++i]);
    else if (arg == "--out" && i + 1 < argc) output = argv[++i];
    else if (arg == "--variants-out" && i + 1 < argc) variants_output = argv[++i];
    else if (arg == "--help") {
      std::cout << "Usage: regidrago_sim [--trials N] [--variant-trials N] [--seed N] [--out PATH] [--variants-out PATH]\n";
      return 0;
    }
  }

  if (variant_trials == 0) variant_trials = trials;

  std::ostringstream csv;
  csv << "scenario,trials,ready_by_t2_pct,ready_by_t2_se_pp,ready_by_t3_pct,ready_by_t3_se_pp,ready_by_t4_pct,ready_by_t4_se_pp,started_regi_pct,fss_used_pct,steven_used_pct,blender_used_pct,mean_mulligans\n";
  std::cout << std::fixed << std::setprecision(3);
  const sim::DeckRecipe baseline = sim::baseline_recipe();
  const std::vector<sim::Scenario> all_scenarios = sim::scenarios();
  for (std::size_t index = 0; index < all_scenarios.size(); ++index) {
    const sim::Scenario& scenario = all_scenarios[index];
    const sim::Aggregate a = sim::simulate(scenario, baseline, trials, seed + 7919U * static_cast<std::uint64_t>(index));
    const double p2 = sim::pct(a.by2, a.trials);
    const double p3 = sim::pct(a.by3, a.trials);
    const double p4 = sim::pct(a.by4, a.trials);
    csv << sim::csv_field(scenario.label) << ',' << a.trials << ',' << p2 << ',' << sim::se_pp(a.by2, a.trials) << ',' << p3 << ','
        << sim::se_pp(a.by3, a.trials) << ',' << p4 << ',' << sim::se_pp(a.by4, a.trials) << ','
        << sim::pct(a.started_regi, a.trials) << ',' << sim::pct(a.fss, a.trials) << ',' << sim::pct(a.steven, a.trials)
        << ',' << sim::pct(a.blender, a.trials) << ',' << (a.total_mulligans / static_cast<double>(a.trials)) << '\n';
    std::cout << std::left << std::setw(47) << scenario.label << " T2=" << std::setw(7) << p2 << "%  T3="
              << std::setw(7) << p3 << "%  T4=" << std::setw(7) << p4 << "%\n";
  }
  std::ostringstream variant_csv;
  variant_csv << "variant,scenario,trials,ready_by_t2_pct,ready_by_t3_pct,ready_by_t4_pct,delta_t2_pp_vs_baseline,delta_t3_pp_vs_baseline,delta_t4_pp_vs_baseline\n";
  const std::array<sim::Scenario, 4> variant_scenarios{{
      {"strict-jit/go-first", sim::DciProfile::StrictJit, sim::LockMode::None, true},
      {"strict-jit/go-second", sim::DciProfile::StrictJit, sim::LockMode::None, false},
      {"matchup-flex-jit/go-first", sim::DciProfile::MatchupFlexJit, sim::LockMode::None, true},
      {"strict-jit-turn2-item-lock/go-second", sim::DciProfile::StrictJit, sim::LockMode::TurnTwoItem, false},
  }};
  const auto all_variants = sim::variants();
  for (std::size_t v = 0; v < all_variants.size(); ++v) {
    for (std::size_t q = 0; q < variant_scenarios.size(); ++q) {
      const auto& scenario = variant_scenarios[q];
      const sim::Aggregate base_a = sim::simulate(scenario, baseline, variant_trials, seed + 200003U * static_cast<std::uint64_t>(q));
      const sim::Aggregate variant_a = sim::simulate(scenario, all_variants[v].recipe, variant_trials,
                                                       seed + 200003U * static_cast<std::uint64_t>(q));
      variant_csv << sim::csv_field(all_variants[v].label) << ',' << sim::csv_field(scenario.label) << ',' << variant_trials << ','
                  << sim::pct(variant_a.by2, variant_a.trials) << ',' << sim::pct(variant_a.by3, variant_a.trials) << ','
                  << sim::pct(variant_a.by4, variant_a.trials) << ','
                  << (sim::pct(variant_a.by2, variant_a.trials) - sim::pct(base_a.by2, base_a.trials)) << ','
                  << (sim::pct(variant_a.by3, variant_a.trials) - sim::pct(base_a.by3, base_a.trials)) << ','
                  << (sim::pct(variant_a.by4, variant_a.trials) - sim::pct(base_a.by4, base_a.trials)) << '\n';
    }
  }
  try {
    sim::write_atomically_locked(output, csv.str());
    sim::write_atomically_locked(variants_output, variant_csv.str());
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
