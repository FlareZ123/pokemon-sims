#pragma once

// Canonical namespace-scope composition boundary for the Forretress feature.
// Keep runtime and scenario extension ownership adjacent here while Engine is closed.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
#include "core/forretress/runtime.inc"

// The historical registry is defined later in part_014c.inc. This extension is
// composed at the verified namespace boundary after Engine closes, so forward
// declarations preserve the public historical registry for the existing corpus:
// Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
using ScenarioList = std::vector<Scenario>;
using GarbodorScenarioSet = std::array<Scenario, 2>;
ScenarioList all_scenarios();
std::optional<Scenario> scenario_by_label(const std::string& label);

template <typename ScenarioRange>
class ScenarioExtension {
 public:
  explicit ScenarioExtension(const ScenarioRange& scenarios)
      : scenarios_(scenarios) {}

  [[nodiscard]] ScenarioList append_to(ScenarioList result) const {
    result.reserve(result.size() + scenarios_.size());
    result.insert(result.end(), scenarios_.begin(), scenarios_.end());
    return result;
  }

  [[nodiscard]] std::optional<Scenario> find(const std::string& label) const {
    // Keep extension lookup traversal beside append ownership so future scenario
    // ranges reuse one abstraction instead of adding another forwarding .inc file.
    // Return the same optional value shape as scenario_by_label() instead of
    // exposing a pointer tied to the extension range's storage lifetime.
    // C++ find_if semantics: https://eel.is/c++draft/alg.find
    // C++ optional semantics: https://eel.is/c++draft/optional
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
  ScenarioRange scenarios_;
};

// Keep the Garbodor rows inside their sole append/lookup owner instead of exposing
// a second namespace-scope scenario container with coupled lifetime and ordering.
// Scenario specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
const ScenarioExtension<GarbodorScenarioSet> kGarbodorScenarioExtension{
    GarbodorScenarioSet{{
        {"garbodor-shake-ability-lock/go-first",
         DciProfile::StrictJit, LockMode::None, true, 5},
        {"garbodor-shake-ability-lock/go-second",
         DciProfile::StrictJit, LockMode::None, false, 5},
    }}};

ScenarioList all_scenarios_with_garbodor() {
  // Garbodor BKP 57 suppresses Pokemon Abilities while it has a Tool. Boost Shake
  // establishes Garbodor during the opponent's first turn, producing different
  // Regidrago T1 timing by seat. Append these rows so existing scenario indexes and
  // seeded common-random-number comparisons remain stable:
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
  // Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
  return kGarbodorScenarioExtension.append_to(all_scenarios());
}

std::optional<Scenario> scenario_by_label_with_garbodor(const std::string& label) {
  if (const auto match = kGarbodorScenarioExtension.find(label)) {
    return match;
  }
  return scenario_by_label(label);
}