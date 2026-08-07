from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def replace_once(path: str, old: str, new: str) -> None:
    target = ROOT / path
    text = target.read_text(encoding="utf-8")
    if new in text:
        return
    if text.count(old) != 1:
        raise RuntimeError(f"{path}: expected one overlap anchor, found {text.count(old)}")
    target.write_text(text.replace(old, new, 1), encoding="utf-8")

replace_once(
    "src/trace_engine_v2/part_003.inc",
    '  for (int i = 0; i < p.fire; ++i) out << "F";\n  if (p.grass == 0 && p.fire == 0) out << "-";\n',
    '  for (int i = 0; i < p.fire; ++i) out << "F";\n  // D is one physical Double Dragon Energy card.\n  for (int i = 0; i < p.double_dragon; ++i) out << "D";\n  if (p.grass == 0 && p.fire == 0 && p.double_dragon == 0) out << "-";\n')
replace_once(
    "src/trace_engine_v2/part_003.inc",
    '        case Card::Grass: return pokemon.grass;\n        case Card::Fire: return pokemon.fire;\n        case Card::ForestSealStone:',
    '        case Card::Grass: return pokemon.grass;\n        case Card::Fire: return pokemon.fire;\n        case Card::DoubleDragonEnergy: return pokemon.double_dragon;\n        case Card::ForestSealStone:')

replace_once(
    "src/trace_engine_v2/part_014c.inc",
    '    const bool active_vstar = active_is_vstar();\n    const bool grass_ready = active_vstar && state_.active->grass >= 2;\n    const bool fire_ready = active_vstar && state_.active->fire >= 1;\n    const bool payload_is_ready = payload_ready();\n    const bool ready = state_.turn >= 2 && active_vstar && grass_ready &&\n        fire_ready && payload_is_ready;\n',
    '    const bool active_vstar = active_is_vstar();\n    const bool energy_ready = active_vstar && pays_apex_energy_cost(*state_.active);\n    const bool has_dde = active_vstar && state_.active->double_dragon > 0;\n    const bool grass_ready = active_vstar &&\n        (has_dde ? energy_ready : state_.active->grass >= 2);\n    const bool fire_ready = active_vstar &&\n        (has_dde ? energy_ready : state_.active->fire >= 1);\n    const bool payload_is_ready = payload_ready();\n    const bool ready = state_.turn >= 2 && active_vstar && energy_ready &&\n        payload_is_ready;\n')
replace_once(
    "src/trace_engine_v2/part_014c.inc",
    'DeckRecipe baseline_recipe() {\n  return DeckRecipe(kDeckRecipe.begin(), kDeckRecipe.end());\n}\n\nDeckRecipe pineco_recipe() {',
    'DeckRecipe baseline_recipe() {\n  return DeckRecipe(kDeckRecipe.begin(), kDeckRecipe.end());\n}\n\nDeckRecipe double_dragon_modeling_recipe() {\n  DeckRecipe recipe = baseline_recipe();\n  for (auto& [card, copies] : recipe) {\n    if (card == Card::Grass) --copies;\n    if (card == Card::Fire) --copies;\n  }\n  recipe.push_back({Card::DoubleDragonEnergy, 2});\n  return recipe;\n}\n\nDeckRecipe pineco_recipe() {')
replace_once(
    "src/trace_engine_v2/part_014c.inc",
    '    if (!is_energy(card) && copies_by_card[card] > 4) {\n',
    '    if (!is_basic_energy(card) && copies_by_card[card] > 4) {\n')
replace_once(
    "src/trace_engine_v2/part_014c.inc",
    '  const std::string_view normalized =\n      id == "regidrago-pineco-secret-box" ? "regidrago-pineco" : id;\n  const auto& decks = deck_registry();\n',
    '  const std::string_view normalized =\n      id == "regidrago-pineco-secret-box" ? "regidrago-pineco" : id;\n  if (normalized == "regidrago-dde-model") {\n    static const NamedDeck dde_model = [] {\n      NamedDeck deck{"regidrago-dde-model", double_dragon_modeling_recipe()};\n      std::string error;\n      if (!validate_recipe(deck, &error)) throw std::logic_error(error);\n      return deck;\n    }();\n    return &dde_model;\n  }\n  const auto& decks = deck_registry();\n')
