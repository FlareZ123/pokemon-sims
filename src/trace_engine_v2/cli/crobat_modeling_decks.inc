#pragma once

// Crobat temporary-deck construction and lookup.
// This file is composed inside namespace sim by crobat_modeling.inc.
// Modeling contract: https://github.com/FlareZ123/pokemon-sims/issues/1394
// C++ textual include semantics: https://eel.is/c++draft/cpp.include

struct CrobatModelingDeck {
  std::string id;
  std::string cuts;
  DeckRecipe recipe;
};

void adjust_modeling_recipe(DeckRecipe& recipe, const Card card, const int delta) {
  const auto found = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) {
    return entry.first == card;
  });
  if (found == recipe.end()) {
    if (delta <= 0) throw std::logic_error("modeling cut is absent");
    recipe.push_back({card, delta});
    return;
  }
  found->second += delta;
  if (found->second < 0) throw std::logic_error("modeling cut exceeds copies");
  if (found->second == 0) recipe.erase(found);
}

int modeling_recipe_count(const DeckRecipe& recipe, const Card card) {
  const auto found = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) {
    return entry.first == card;
  });
  return found == recipe.end() ? 0 : found->second;
}

CrobatModelingDeck make_crobat_modeling_deck(
    std::string id, const std::initializer_list<Card> cuts) {
  DeckRecipe recipe = baseline_recipe();
  std::ostringstream cut_names;
  std::size_t index = 0;
  for (const Card cut : cuts) {
    adjust_modeling_recipe(recipe, cut, -1);
    if (index++ > 0) cut_names << " + ";
    cut_names << name(cut);
  }
  adjust_modeling_recipe(recipe, Card::CrobatV, static_cast<int>(cuts.size()));
  CrobatModelingDeck deck{std::move(id), cut_names.str(), std::move(recipe)};
  std::string error;
  if (!validate_recipe({deck.id, deck.recipe}, &error)) throw std::logic_error(error);
  return deck;
}

// Temporary current-list proxy shell for the connector/Guzma & Hala experiment.
// User-locked slots are never cut. Cards not yet represented by named simulator IDs
// are held as fixed same-size proxies rather than being given invented gameplay:
// - 2 Goomy + Minior + Resetting Hole Marshadow -> 4 Pineco (Basic, non-Rule-Box slots)
// - Carmine -> one additional Serena (draw-Supporter slot; conservative on T1 going first)
// The experiment compares connector variants against one another, so these fixed
// proxy slots do not move between arms. Battle VIP Pass, Gladion, Heavy Ball,
// recovery/discrete utility, anti-lock slots, and payloads are never candidate cuts.
// Deck concepts / connector-domination / lock treatment:
// https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
DeckRecipe make_user_current_recipe(const int quick_ball, const int earthen_vessel) {
  if (quick_ball < 0 || earthen_vessel < 0 || quick_ball + earthen_vessel != 2) {
    throw std::logic_error("current-list QB/EV axis must contain exactly two slots");
  }
  DeckRecipe recipe = baseline_recipe();

  // Current payload / utility structure.
  adjust_modeling_recipe(recipe, Card::Dragapult, -1);       // 2 -> 1
  adjust_modeling_recipe(recipe, Card::GoodraVstar, 1);     // 1 -> 2
  adjust_modeling_recipe(recipe, Card::ErikasInvitation, -1);
  adjust_modeling_recipe(recipe, Card::Dipplin, -1);
  adjust_modeling_recipe(recipe, Card::Channeler, -1);
  adjust_modeling_recipe(recipe, Card::Powerglass, -1);
  adjust_modeling_recipe(recipe, Card::PathToPeak, -1);

  // Current connector axis and paper-Expanded DDE package.
  adjust_modeling_recipe(recipe, Card::QuickBall, quick_ball - 3);
  adjust_modeling_recipe(recipe, Card::EarthenVessel, earthen_vessel - 2);
  adjust_modeling_recipe(recipe, Card::Grass, -2);
  adjust_modeling_recipe(recipe, Card::DoubleDragonEnergy, 2);

  // Exact implemented current-list slots.
  adjust_modeling_recipe(recipe, Card::BattleVipPass, 2);
  adjust_modeling_recipe(recipe, Card::WishfulBaton, 1);

  // Fixed neutral/basic-equivalent proxies for currently unnamed engine cards.
  adjust_modeling_recipe(recipe, Card::Pineco, 4);
  adjust_modeling_recipe(recipe, Card::Serena, 1);
  return recipe;
}

struct UserConnectorCut {
  Card card;
  std::string_view slug;
};

constexpr std::array<UserConnectorCut, 10> kUserConnectorCuts{{
    {Card::MysteriousTreasure, "treasure"},
    {Card::QuickBall, "quick-ball"},
    {Card::EarthenVessel, "vessel"},
    {Card::TapuLeleGX, "tapu-lele"},
    {Card::Crispin, "crispin"},
    {Card::Arven, "arven"},
    {Card::ProfessorBurnet, "burnet"},
    {Card::StevensResolve, "steven"},
    {Card::ForestSealStone, "fss"},
    {Card::Oricorio, "oricorio"},
}};

CrobatModelingDeck make_user_connector_variant(
    const int quick_ball, const int earthen_vessel,
    const std::vector<std::size_t>& cut_indices) {
  DeckRecipe recipe = make_user_current_recipe(quick_ball, earthen_vessel);
  std::ostringstream id;
  std::ostringstream cuts;
  id << "user-qb" << quick_ball << "-ev" << earthen_vessel
     << "-gh" << cut_indices.size();
  cuts << quick_ball << " QB / " << earthen_vessel << " EV";

  for (const std::size_t index : cut_indices) {
    const UserConnectorCut& cut = kUserConnectorCuts.at(index);
    if (modeling_recipe_count(recipe, cut.card) <= 0) {
      throw std::logic_error("requested current-list connector cut is absent");
    }
    adjust_modeling_recipe(recipe, cut.card, -1);
    adjust_modeling_recipe(recipe, Card::GuzmaHala, 1);
    id << "-" << cut.slug;
    cuts << "; " << name(cut.card) << " -> Guzma & Hala";
  }

  CrobatModelingDeck deck{id.str(), cuts.str(), std::move(recipe)};
  std::string error;
  if (!validate_recipe({deck.id, deck.recipe}, &error)) throw std::logic_error(error);
  return deck;
}

void append_user_connector_scan(std::vector<CrobatModelingDeck>& decks) {
  for (int quick_ball = 0; quick_ball <= 2; ++quick_ball) {
    const int earthen_vessel = 2 - quick_ball;
    decks.push_back(make_user_connector_variant(quick_ball, earthen_vessel, {}));

    // Every legal one-connector replacement by one Guzma & Hala.
    for (std::size_t first = 0; first < kUserConnectorCuts.size(); ++first) {
      DeckRecipe base = make_user_current_recipe(quick_ball, earthen_vessel);
      if (modeling_recipe_count(base, kUserConnectorCuts[first].card) <= 0) continue;
      decks.push_back(make_user_connector_variant(quick_ball, earthen_vessel, {first}));
    }

    // Every legal two-slot replacement by two Guzma & Hala, including two copies
    // of the same connector when the list actually contains at least two copies.
    for (std::size_t first = 0; first < kUserConnectorCuts.size(); ++first) {
      for (std::size_t second = first; second < kUserConnectorCuts.size(); ++second) {
        DeckRecipe base = make_user_current_recipe(quick_ball, earthen_vessel);
        const Card first_card = kUserConnectorCuts[first].card;
        const Card second_card = kUserConnectorCuts[second].card;
        const int first_count = modeling_recipe_count(base, first_card);
        const int second_count = modeling_recipe_count(base, second_card);
        if (first_count <= 0 || second_count <= 0) continue;
        if (first == second && first_count < 2) continue;
        decks.push_back(make_user_connector_variant(
            quick_ball, earthen_vessel, {first, second}));
      }
    }
  }
}

class CrobatModelingRegistry {
 public:
  static const std::vector<CrobatModelingDeck>& all() {
    // These temporary recipes are deliberately absent from deck_registry() and
    // --all-decks: https://github.com/FlareZ123/pokemon-sims/issues/1394
    static const std::vector<CrobatModelingDeck> decks = [] {
      std::vector<CrobatModelingDeck> result{
          {"regidrago-shell", "none", baseline_recipe()},
          make_crobat_modeling_deck("crobat1-erika", {Card::ErikasInvitation}),
          make_crobat_modeling_deck("crobat1-channeler", {Card::Channeler}),
          make_crobat_modeling_deck("crobat1-team-yell", {Card::TeamYellsCheer}),
          make_crobat_modeling_deck("crobat1-klara", {Card::Klara}),
          make_crobat_modeling_deck("crobat1-turo", {Card::ProfessorTuro}),
          make_crobat_modeling_deck("crobat1-powerglass", {Card::Powerglass}),
          make_crobat_modeling_deck("crobat1-heavy-ball", {Card::HisuianHeavyBall}),
          make_crobat_modeling_deck("crobat1-tapu-lele", {Card::TapuLeleGX}),
          make_crobat_modeling_deck("crobat2-erika-channeler",
                                    {Card::ErikasInvitation, Card::Channeler}),
          make_crobat_modeling_deck("crobat2-erika-team-yell",
                                    {Card::ErikasInvitation, Card::TeamYellsCheer}),
          make_crobat_modeling_deck("crobat2-erika-tapu-lele",
                                    {Card::ErikasInvitation, Card::TapuLeleGX}),
          make_crobat_modeling_deck("crobat2-tapu-lele-both",
                                    {Card::TapuLeleGX, Card::TapuLeleGX}),
      };
      append_user_connector_scan(result);
      return result;
    }();
    return decks;
  }

  static const CrobatModelingDeck* find(const std::string_view id) {
    const auto& decks = all();
    const auto found = std::find_if(decks.begin(), decks.end(), [id](const auto& deck) {
      return deck.id == id;
    });
    return found == decks.end() ? nullptr : &*found;
  }
};

const std::vector<CrobatModelingDeck>& crobat_modeling_decks() {
  return CrobatModelingRegistry::all();
}

const CrobatModelingDeck* crobat_modeling_deck_by_id(const std::string_view id) {
  return CrobatModelingRegistry::find(id);
}

int crobat_copy_count(const DeckRecipe& recipe) {
  return std::accumulate(recipe.begin(), recipe.end(), 0,
                         [](const int total, const auto& entry) {
                           return total +
                               (entry.first == Card::CrobatV ? entry.second : 0);
                         });
}
