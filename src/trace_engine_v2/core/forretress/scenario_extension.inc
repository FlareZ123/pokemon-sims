#pragma once

// Shared namespace-scope scenario extension helper for Forretress-side scenario
// composition. Keep append and lookup traversal together so feature-specific
// scenario ranges do not grow another forwarding helper at the root boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// C++ find_if semantics: https://eel.is/c++draft/alg.find
// C++ optional semantics: https://eel.is/c++draft/optional
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
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
