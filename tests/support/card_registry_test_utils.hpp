#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace test_support {

inline void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

inline const sim::cards::CardDefinition& require_card_definition(
    const sim::Card card, const std::string_view message) {
  const auto* definition = sim::cards::find_definition(card);
  require(definition != nullptr, message);
  return *definition;
}

}  // namespace test_support
