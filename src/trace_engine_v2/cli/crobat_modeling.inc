#pragma once

// Crobat modeling is a CLI-only experimental surface kept outside the main
// translation-unit tail so part_016 remains focused on CLI parsing/dispatch.
// Modeling contract: https://github.com/FlareZ123/pokemon-sims/issues/1394

namespace sim {

struct CrobatModelingDeck {
  std::string id;
  std::string cuts;
  DeckRecipe recipe;
};

void validate_crobat_modeling_deck(const CrobatModelingDeck& deck) {
  // Keep temporary modeling recipes under the same 60-card recipe contract as
  // registered decks: https://github.com/FlareZ123/pokemon-sims/issues/1394
  std::string error;
  if (!validate_recipe({deck.id, deck.recipe}, &error)) throw std::logic_error(error);
}

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
  validate_crobat_modeling_deck(deck);
  return deck;
}

const std::vector<CrobatModelingDeck>& crobat_modeling_decks() {
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

const CrobatModelingDeck* crobat_modeling_deck_by_id(const std::string_view id) {
  const auto& decks = crobat_modeling_decks();
  const auto found = std::find_if(decks.begin(), decks.end(), [id](const auto& deck) {
    return deck.id == id;
  });
  return found == decks.end() ? nullptr : &*found;
}

int crobat_copy_count(const DeckRecipe& recipe) {
  return std::accumulate(recipe.begin(), recipe.end(), 0,
                         [](const int total, const auto& entry) {
                           return total +
                               (entry.first == Card::CrobatV ? entry.second : 0);
                         });
}

constexpr std::size_t crobat_model_seed_slot(const std::size_t scenario_index) {
  // Historical slots 4 and 11 belonged to retired full-turn-one Item-lock rows.
  // Preserve the established random stream for every surviving scenario:
  // https://github.com/FlareZ123/pokemon-sims/issues/1118
  // https://github.com/FlareZ123/pokemon-sims/issues/2247
  return scenario_index + (scenario_index >= 4 ? 1U : 0U) +
         (scenario_index >= 10 ? 1U : 0U);
}

void write_crobat_modeling_matrix(const std::filesystem::path& output,
                                  const std::uint64_t trials,
                                  const std::uint64_t seed) {
  std::ostringstream csv;
  csv << "variant,cuts,crobat_copies,scenario,trials,ready_by_t2_pct,"
         "ready_by_t3_pct,ready_by_t4_pct,dark_asset_use_pct,avg_dark_asset_cards_drawn_per_game_using\n";
  const auto scenarios = all_scenarios();
  for (std::size_t scenario_index = 0; scenario_index < scenarios.size(); ++scenario_index) {
    const Scenario& scenario = scenarios[scenario_index];
    const std::size_t seed_slot = crobat_model_seed_slot(scenario_index);
    const std::uint64_t common_seed = seed + 104729ULL * seed_slot;
    for (const CrobatModelingDeck& deck : crobat_modeling_decks()) {
      const Aggregate aggregate = simulate(scenario, deck.recipe, trials, common_seed);
      // aggregate.dark_asset counts games with at least one use; cards may accumulate
      // across turns in two-copy variants: https://github.com/FlareZ123/pokemon-sims/issues/1394
      const double average_draw = aggregate.dark_asset == 0 ? 0.0 :
          static_cast<double>(aggregate.dark_asset_cards_drawn) /
          static_cast<double>(aggregate.dark_asset);
      csv << deck.id << ',' << '"' << deck.cuts << '"' << ','
          << crobat_copy_count(deck.recipe) << ','
          << scenario.label << ',' << trials << ',' << std::fixed
          << std::setprecision(6) << pct(aggregate.by2, trials) << ','
          << pct(aggregate.by3, trials) << ',' << pct(aggregate.by4, trials) << ','
          << pct(aggregate.dark_asset, trials) << ',' << average_draw << '\n';
    }
  }
  write_atomic(output, csv.str());
}

constexpr std::string_view crobat_model_default_output() {
  return "results/crobat_variant_model.csv";  // Reproduction contract: https://github.com/FlareZ123/pokemon-sims/issues/1394
}

constexpr std::uint64_t crobat_model_default_seed() {
  return 20260723ULL;  // Reproduction contract: https://github.com/FlareZ123/pokemon-sims/issues/1394
}

}  // namespace sim
