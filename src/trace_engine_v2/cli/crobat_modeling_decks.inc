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

class CrobatModelingRegistry {
 public:
  static const std::vector<CrobatModelingDeck>& all() {
    // These temporary recipes are deliberately absent from deck_registry() and
    // --all-decks: https://github.com/FlareZ123/pokemon-sims/issues/1394
    static const std::vector<CrobatModelingDeck> decks{
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
