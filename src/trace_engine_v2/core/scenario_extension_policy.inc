#pragma once

// Stateless physical-range mechanics shared by scenario-extension owners.
// Keep append and label traversal in one core owner so feature packages only own
// their scenario data and fallback composition.
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

// Shared namespace-scope scenario extension helper. Range ownership stays beside
// the centralized physical traversal so feature-specific registries do not grow
// duplicate forwarding helpers.
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
    // Keep extension-first precedence in the shared abstraction so each scenario
    // family cannot accidentally reverse lookup order when composing a base set.
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
