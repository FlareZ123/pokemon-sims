#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {

struct SetupCheckpoint {
  bool regi{false};
  bool vstar{false};
  bool active_vstar{false};
  bool energy{false};
  bool payload{false};
  bool k1{false};
};

struct CheckpointTrial {
  TrialOutcome outcome;
  std::array<SetupCheckpoint, 5> by_turn{};
};

struct EngineTestAccess {
  static SetupCheckpoint checkpoint(Engine& engine) {
    const Pokemon* target = engine.target_regi();
    return {
        engine.in_play(Card::RegidragoV) ||
            engine.in_play(Card::RegidragoVstar),
        engine.in_play(Card::RegidragoVstar),
        engine.active_is_vstar(),
        target != nullptr && engine.pays_apex_energy_cost(*target),
        engine.payload_ready(),
        engine.prizes_known(),
    };
  }

  static CheckpointTrial run(Engine& engine) {
    CheckpointTrial result;
    engine.setup();
    SetupCheckpoint latest;
    for (int next_turn = 1; next_turn <= engine.scenario_.max_turn; ++next_turn) {
      engine.begin_turn(next_turn);
      if (engine.state_.turn_ended) break;
      engine.run_turn();
      engine.record_ready();
      latest = checkpoint(engine);
      result.by_turn[static_cast<std::size_t>(next_turn - 1)] = latest;
      if (engine.outcome_.first_ready_turn != 0) {
        for (int future = next_turn + 1; future <= 5; ++future) {
          result.by_turn[static_cast<std::size_t>(future - 1)] = latest;
        }
        break;
      }
      engine.resolve_powerglass_end_turn();
    }
    result.outcome = engine.outcome_;
    return result;
  }
};

struct VariantAggregate {
  std::uint64_t trials{0};
  std::uint64_t by2{0};
  std::uint64_t by3{0};
  std::uint64_t by4{0};
  std::array<std::uint64_t, 5> regi{};
  std::array<std::uint64_t, 5> vstar{};
  std::array<std::uint64_t, 5> active_vstar{};
  std::array<std::uint64_t, 5> energy{};
  std::array<std::uint64_t, 5> payload{};
  std::array<std::uint64_t, 5> k1{};
};

DeckRecipe professors_letter_swap_recipe() {
  DeckRecipe recipe = baseline_recipe();
  const auto vessel = std::find_if(
      recipe.begin(), recipe.end(),
      [](const auto& entry) { return entry.first == Card::EarthenVessel; });
  if (vessel == recipe.end() || vessel->second != 2) {
    throw std::logic_error(
        "regidrago-shell no longer has exactly two Earthen Vessel");
  }

  // Preserve the exact two physical recipe slots. Erasing Vessel and appending
  // Letter changes the pre-shuffle vector order, so a shared RNG seed no longer
  // pairs the other 58 physical cards. Replace the identity in place instead.
  // Professor's Letter: https://api.pokemontcg.io/v2/cards/xy1-123
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Confirmed comparison bug: https://github.com/FlareZ123/pokemon-sims/issues/2599
  vessel->first = Card::ProfessorsLetter;

  NamedDeck validation{"professors-letter-temporary-swap", recipe};
  std::string error;
  if (!validate_recipe(validation, &error)) throw std::logic_error(error);
  return recipe;
}

std::vector<Card> expanded_recipe(const DeckRecipe& recipe) {
  std::vector<Card> cards;
  for (const auto& [card, copies] : recipe) {
    for (int copy = 0; copy < copies; ++copy) cards.push_back(card);
  }
  return cards;
}

void verify_slot_preserved_pairing(
    const DeckRecipe& baseline, const DeckRecipe& letter) {
  const auto baseline_cards = expanded_recipe(baseline);
  const auto letter_cards = expanded_recipe(letter);
  if (baseline_cards.size() != 60U || letter_cards.size() != 60U ||
      baseline_cards.size() != letter_cards.size()) {
    throw std::logic_error("Professor's Letter comparison recipes are not 60 cards");
  }

  std::size_t differences = 0;
  for (std::size_t index = 0; index < baseline_cards.size(); ++index) {
    if (baseline_cards[index] == letter_cards[index]) continue;
    ++differences;
    if (baseline_cards[index] != Card::EarthenVessel ||
        letter_cards[index] != Card::ProfessorsLetter) {
      throw std::logic_error(
          "Professor's Letter pairing changed a non-Vessel physical slot");
    }
  }
  if (differences != 2U) {
    throw std::logic_error(
        "Professor's Letter pairing did not replace exactly two Vessel slots");
  }
}

VariantAggregate simulate_checkpoints(
    const Scenario& scenario, const DeckRecipe& recipe,
    const std::uint64_t trials, const std::uint64_t seed) {
  VariantAggregate aggregate;
  for (std::uint64_t trial = 0; trial < trials; ++trial) {
    // Each paired game gets a fresh deterministic RNG. Because the temporary
    // recipe preserves physical slots, both variants begin from the same shuffle
    // permutation until a replaced Vessel/Letter identity affects later actions.
    // Core shuffle/search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Pairing fix: https://github.com/FlareZ123/pokemon-sims/issues/2599
    std::mt19937_64 rng(seed + trial);
    Engine engine(scenario, recipe, rng);
    const CheckpointTrial result = EngineTestAccess::run(engine);
    ++aggregate.trials;
    aggregate.by2 += result.outcome.ready_by_2 ? 1U : 0U;
    aggregate.by3 += result.outcome.ready_by_3 ? 1U : 0U;
    aggregate.by4 += result.outcome.ready_by_4 ? 1U : 0U;
    for (std::size_t turn = 0; turn < aggregate.regi.size(); ++turn) {
      aggregate.regi[turn] += result.by_turn[turn].regi ? 1U : 0U;
      aggregate.vstar[turn] += result.by_turn[turn].vstar ? 1U : 0U;
      aggregate.active_vstar[turn] +=
          result.by_turn[turn].active_vstar ? 1U : 0U;
      aggregate.energy[turn] += result.by_turn[turn].energy ? 1U : 0U;
      aggregate.payload[turn] += result.by_turn[turn].payload ? 1U : 0U;
      aggregate.k1[turn] += result.by_turn[turn].k1 ? 1U : 0U;
    }
  }
  return aggregate;
}

double percent(const std::uint64_t count, const std::uint64_t total) {
  return total == 0 ? 0.0 :
      100.0 * static_cast<double>(count) / static_cast<double>(total);
}

void write_row(std::ostream& out, const std::string& variant,
               const Scenario& scenario, const VariantAggregate& a) {
  out << variant << ',' << scenario.label << ',' << a.trials << ','
      << std::fixed << std::setprecision(6)
      << percent(a.by2, a.trials) << ','
      << percent(a.by3, a.trials) << ','
      << percent(a.by4, a.trials);
  for (const std::size_t index : {1U, 2U, 3U}) {
    out << ',' << percent(a.regi[index], a.trials)
        << ',' << percent(a.vstar[index], a.trials)
        << ',' << percent(a.active_vstar[index], a.trials)
        << ',' << percent(a.energy[index], a.trials)
        << ',' << percent(a.payload[index], a.trials)
        << ',' << percent(a.k1[index], a.trials);
  }
  out << '\n';
}

}  // namespace sim

int main(int argc, char** argv) {
  try {
    if (argc != 4) {
      std::cerr << "usage: professors_letter_swap_matrix TRIALS SEED OUTPUT\n";
      return 2;
    }
    const std::uint64_t trials = std::stoull(argv[1]);
    const std::uint64_t seed = std::stoull(argv[2]);
    if (trials == 0) throw std::runtime_error("TRIALS must be positive");

    const sim::DeckRecipe baseline = sim::baseline_recipe();
    const sim::DeckRecipe letter = sim::professors_letter_swap_recipe();
    sim::verify_slot_preserved_pairing(baseline, letter);

    std::ofstream out(argv[3], std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("could not open output");
    out << "variant,scenario,trials,ready_by_t2_pct,ready_by_t3_pct,"
           "ready_by_t4_pct,"
           "t2_regi_pct,t2_vstar_pct,t2_active_vstar_pct,t2_energy_pct,"
           "t2_payload_pct,t2_k1_pct,"
           "t3_regi_pct,t3_vstar_pct,t3_active_vstar_pct,t3_energy_pct,"
           "t3_payload_pct,t3_k1_pct,"
           "t4_regi_pct,t4_vstar_pct,t4_active_vstar_pct,t4_energy_pct,"
           "t4_payload_pct,t4_k1_pct\n";

    const std::vector<sim::Scenario> scenarios = sim::all_scenarios();
    for (std::size_t index = 0; index < scenarios.size(); ++index) {
      const std::size_t seed_slot =
          index + (index >= 4 ? 1U : 0U) + (index >= 10 ? 1U : 0U);
      const std::uint64_t common_seed = seed + 104729ULL * seed_slot;
      const sim::VariantAggregate baseline_result =
          sim::simulate_checkpoints(
              scenarios[index], baseline, trials, common_seed);
      const sim::VariantAggregate letter_result =
          sim::simulate_checkpoints(
              scenarios[index], letter, trials, common_seed);
      sim::write_row(
          out, "regidrago-shell", scenarios[index], baseline_result);
      sim::write_row(out, "letter-swap", scenarios[index], letter_result);
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
