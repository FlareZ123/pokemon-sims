#pragma once

// Stateless physical-range mechanics shared by every Forretress-side scenario
// extension. Keep append and label traversal in one owner so feature-specific
// registries only own their scenario data and fallback composition.
// C++ range insertion semantics: https://eel.is/c++draft/vector.modifiers
// C++ find_if semantics: https://eel.is/c++draft/alg.find
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
class ScenarioRangePolicy final {
 public:
  template <typename ScenarioRange>
  [[nodiscard]] static ScenarioList append(
      ScenarioList result, const ScenarioRange& scenarios) {
    result.reserve(result.size() + scenarios.size());
    result.insert(result.end(), scenarios.begin(), scenarios.end());
    return result;
  }

  template <typename ScenarioRange>
  [[nodiscard]] static std::optional<Scenario> find(
      const ScenarioRange& scenarios, const std::string& label) {
    const auto match = std::find_if(
        scenarios.begin(), scenarios.end(),
        [&label](const Scenario& scenario) { return scenario.label == label; });
    return match == scenarios.end()
        ? std::nullopt
        : std::optional<Scenario>{*match};
  }
};

// Shared namespace-scope scenario extension helper for Forretress-side scenario
// composition. Keep range ownership beside the centralized physical traversal so
// feature-specific scenario ranges do not grow forwarding helpers at the root.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// C++ optional semantics: https://eel.is/c++draft/optional
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
template <typename ScenarioRange>
class ScenarioExtension {
 public:
  explicit ScenarioExtension(const ScenarioRange& scenarios)
      : scenarios_(scenarios) {}

  [[nodiscard]] ScenarioList append_to(ScenarioList result) const {
    return ScenarioRangePolicy::append(std::move(result), scenarios_);
  }

  [[nodiscard]] std::optional<Scenario> find(const std::string& label) const {
    return ScenarioRangePolicy::find(scenarios_, label);
  }

  template <typename Fallback>
  [[nodiscard]] std::optional<Scenario> find_or_fallback(
      const std::string& label, Fallback fallback) const {
    // Keep extension-first precedence in the registry abstraction so each scenario
    // family cannot accidentally reverse lookup order when it composes a base set.
    // C++ optional semantics: https://eel.is/c++draft/optional
    // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
    if (const auto match = find(label)) return match;
    return fallback();
  }

 private:
  // Own the extension range so append and lookup cannot outlive external storage.
  // C++ object lifetime rules: https://eel.is/c++draft/basic.life
  ScenarioRange scenarios_;
};

// Canonical Forretress-side owner for the Garbodor / Boost Shake scenario family.
// Keep scenario data beside the reusable ScenarioExtension machinery so the root
// feature boundary only composes runtime and namespace-scope scenario extensions.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
// Scenario specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
const ScenarioExtension<GarbodorScenarioSet> kGarbodorScenarioExtension{
    GarbodorScenarioSet{{
        {"garbodor-shake-ability-lock/go-first",
         DciProfile::StrictJit, LockMode::None, true, 5},
        {"garbodor-shake-ability-lock/go-second",
         DciProfile::StrictJit, LockMode::None, false, 5},
    }}};

ScenarioList all_scenarios_with_garbodor() {
  // Append these rows so existing scenario indexes and seeded common-random-number
  // comparisons remain stable. Garbotoxin suppresses Pokemon Abilities while
  // Garbodor has a Pokemon Tool attached, and Boost Shake establishes the evolved
  // Pokemon during the opponent's first turn:
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
  // Advanced Ability and evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  return kGarbodorScenarioExtension.append_to(all_scenarios());
}

std::optional<Scenario> scenario_by_label_with_garbodor(const std::string& label) {
  return kGarbodorScenarioExtension.find_or_fallback(
      label, [&label] { return scenario_by_label(label); });
}
