#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {

void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

struct ExpectedSupporter {
  sim::Card card;
  std::string_view canonical_id;
  std::string_view name;
};

void test_recovered_supporter_metadata() {
  constexpr std::array expected{
      ExpectedSupporter{sim::Card::TateLiza, "sm7-148", "Tate & Liza"}, // Exact print: https://api.pokemontcg.io/v2/cards/sm7-148 ; issue: https://github.com/FlareZ123/pokemon-sims/issues/3562
      ExpectedSupporter{sim::Card::Serena, "swsh12-164", "Serena"}, // Exact print: https://api.pokemontcg.io/v2/cards/swsh12-164 ; issue: https://github.com/FlareZ123/pokemon-sims/issues/3585
      ExpectedSupporter{sim::Card::Grant, "swsh10-144", "Grant"}, // Exact print: https://api.pokemontcg.io/v2/cards/swsh10-144 ; issue: https://github.com/FlareZ123/pokemon-sims/issues/3589
      ExpectedSupporter{sim::Card::StevensResolve, "sm7-145", "Steven's Resolve"}, // Exact print: https://api.pokemontcg.io/v2/cards/sm7-145 ; issue: https://github.com/FlareZ123/pokemon-sims/issues/3595
  };

  for (const ExpectedSupporter& card : expected) {
    const auto* definition = sim::cards::find_definition(card.card);
    require(definition != nullptr, "Recovered Supporter must be explicitly registered.");
    require(definition->canonical_id == card.canonical_id, "Recovered Supporter canonical print changed.");
    require(definition->name == card.name, "Recovered Supporter display name changed.");
    require(definition->kind == sim::cards::CardKind::Trainer, "Recovered Supporter must remain a Trainer.");
    require(definition->trainer_kind == sim::cards::TrainerKind::Supporter, "Recovered Supporter subtype changed."); // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    require(sim::is_supporter(card.card), "Compatibility classification must source the registry.");
    require(!sim::is_item(card.card), "Recovered Supporter must not be classified as an Item.");
    require(sim::name(card.card) == card.name, "Compatibility name must source the registry.");
  }
}

}  // namespace

int main() {
  try {
    test_recovered_supporter_metadata();
    std::cout << "Recovered Supporter card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
