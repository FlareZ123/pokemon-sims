#pragma once

// Shared namespace-scope scenario extension helper for Forretress-side scenario
// composition. Keep append and lookup traversal together so feature-specific
// scenario ranges do not grow another forwarding helper at the root boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// C++ find_if semantics: https://eel.is/c++draft/alg.find
// C++ optional semantics: https://eel.is/c++draft/optional
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
template <typename ScenarioRange>
class ScenarioExtension final {
 public:
  explicit ScenarioExtension(const ScenarioRange& scenarios)
      : scenarios_(scenarios) {}

  [[nodiscard]] ScenarioList append_to(ScenarioList result) const {
    result.reserve(result.size() + scenarios_.size());
    result.insert(result.end(), scenarios_.begin(), scenarios_.end());
    return result;
  }

  [[nodiscard]] std::optional<Scenario> find(const std::string& label) const {
    const auto match = std::find_if(
        scenarios_.begin(), scenarios_.end(),
        [&label](const Scenario& scenario) { return scenario.label == label; });
    return match == scenarios_.end()
        ? std::nullopt
        : std::optional<Scenario>{*match};
  }

 private:
  // Own the extension range so append and lookup cannot outlive external storage.
  // C++ object lifetime rules: https://eel.is/c++draft/basic.life
  const ScenarioRange scenarios_;
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
  if (const auto match = kGarbodorScenarioExtension.find(label)) {
    return match;
  }
  return scenario_by_label(label);
}