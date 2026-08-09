#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace sim {

struct EngineTestAccess {
  struct Snapshot {
    bool ready{false};
    bool regi_in_play{false};
    bool vstar_in_play{false};
    bool active_vstar{false};
    bool energy_ready_any_regi{false};
    bool active_energy_ready{false};
    bool payload_ready{false};
    bool k1{false};
  };

  static bool any_regi_pays_apex(const Engine& engine) {
    const auto pays = [&engine](const Pokemon& pokemon) {
      return (pokemon.card == Card::RegidragoV ||
              pokemon.card == Card::RegidragoVstar) &&
             engine.pays_apex_energy_cost(pokemon);
    };
    if (engine.state_.active && pays(*engine.state_.active)) return true;
    return std::any_of(engine.state_.bench.begin(), engine.state_.bench.end(), pays);
  }

  static Snapshot snapshot(const Engine& engine) {
    const bool active_vstar = engine.active_is_vstar();
    return {
        engine.outcome_.first_ready_turn != 0,
        engine.has_any_regi(),
        engine.has_vstar(),
        active_vstar,
        any_regi_pays_apex(engine),
        active_vstar && engine.pays_apex_energy_cost(*engine.state_.active),
        engine.payload_ready(),
        engine.prizes_known(),
    };
  }

  static std::array<Snapshot, 3> run_checkpoints(Engine& engine) {
    // Exact production turn order and ready observation:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    std::array<Snapshot, 3> snapshots{};
    engine.setup();
    for (int turn = 1; turn <= 4; ++turn) {
      engine.begin_turn(turn);
      if (engine.state_.turn_ended) break;
      engine.run_turn();
      engine.record_ready();
      if (turn >= 2) snapshots[static_cast<std::size_t>(turn - 2)] = snapshot(engine);
      if (engine.outcome_.first_ready_turn != 0) {
        const Snapshot ready = snapshot(engine);
        for (int later = std::max(turn + 1, 2); later <= 4; ++later) {
          snapshots[static_cast<std::size_t>(later - 2)] = ready;
        }
        break;
      }
      engine.resolve_powerglass_end_turn();
    }
    return snapshots;
  }
};

}  // namespace sim

namespace {

struct Variant {
  std::string id;
  std::string cut;
  std::string add;
  sim::DeckRecipe recipe;
};

struct Stats {
  std::uint64_t trials{0};
  std::uint64_t ready{0};
  std::uint64_t regi_in_play{0};
  std::uint64_t vstar_in_play{0};
  std::uint64_t active_vstar{0};
  std::uint64_t energy_ready_any_regi{0};
  std::uint64_t active_energy_ready{0};
  std::uint64_t payload_ready{0};
  std::uint64_t k1{0};
  std::uint64_t primary_no_regi{0};
  std::uint64_t primary_no_vstar{0};
  std::uint64_t primary_inactive_vstar{0};
  std::uint64_t primary_energy{0};
  std::uint64_t primary_payload{0};
  std::uint64_t primary_other{0};
};

int recipe_count(const sim::DeckRecipe& recipe, const sim::Card card) {
  for (const auto& [candidate, copies] : recipe) {
    if (candidate == card) return copies;
  }
  return 0;
}

void adjust(sim::DeckRecipe& recipe, const sim::Card card, const int delta) {
  const auto found = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) {
    return entry.first == card;
  });
  if (found == recipe.end()) {
    if (delta > 0) recipe.push_back({card, delta});
    return;
  }
  found->second += delta;
  if (found->second == 0) recipe.erase(found);
}

std::optional<Variant> make_swap(const sim::Card cut, const sim::Card add) {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  if (recipe_count(recipe, cut) <= 0 || cut == add) return std::nullopt;
  // Non-Basic-Energy cards obey the four-copy rule. The repository validator is
  // the final authority, but skip obviously impossible fifth copies here:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
  if (!sim::is_basic_energy(add) && recipe_count(recipe, add) >= 4) return std::nullopt;
  adjust(recipe, cut, -1);
  adjust(recipe, add, 1);
  Variant variant{
      std::string(sim::name(cut)) + " -> " + std::string(sim::name(add)),
      std::string(sim::name(cut)), std::string(sim::name(add)), std::move(recipe)};
  std::string error;
  if (!sim::validate_recipe({variant.id, variant.recipe}, &error)) return std::nullopt;
  return variant;
}

std::vector<Variant> variants() {
  // Only cards whose modeled setup role is to search/fetch/connect another resource
  // are eligible cuts. High-discrete-value matchup/recovery/utility singletons are
  // deliberately excluded from the cut set.
  // Modeled connector inventory: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
  const std::array cuts{
      sim::Card::TapuLeleGX,          // Wonder Tag -> Supporter: https://api.pokemontcg.io/v2/cards/sm2-60
      sim::Card::MysteriousTreasure,  // Dragon/Psychic search: https://api.pokemontcg.io/v2/cards/sm6-113
      sim::Card::QuickBall,           // Basic search: https://api.pokemontcg.io/v2/cards/swsh1-179
      sim::Card::EarthenVessel,       // Basic Energy search: https://api.pokemontcg.io/v2/cards/sv4-163
      sim::Card::Arven,               // Item + Tool search: https://api.pokemontcg.io/v2/cards/sv1-166
      sim::Card::Crispin,             // Basic Energy search/attach: https://api.pokemontcg.io/v2/cards/sv7-133
      sim::Card::ProfessorBurnet,     // Deck -> discard payload search: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      sim::Card::StevensResolve,      // Any 3 cards: https://api.pokemontcg.io/v2/cards/sm7-145
      sim::Card::Gladion,             // Prize connector: https://api.pokemontcg.io/v2/cards/sm4-95
      sim::Card::ForestSealStone,     // Any-card VSTAR search: https://api.pokemontcg.io/v2/cards/swsh12-156
      sim::Card::HisuianHeavyBall,    // Basic from Prizes: https://api.pokemontcg.io/v2/cards/swsh10-146
  };

  // Adds cover direct bottleneck density plus every implemented search family that
  // could plausibly replace one connector with another. Battle VIP Pass is a real
  // first-turn two-Basic connector in the production engine:
  // https://api.pokemontcg.io/v2/cards/swsh8-225
  // https://github.com/FlareZ123/pokemon-sims/issues/1647
  const std::array adds{
      sim::Card::RegidragoVstar,
      sim::Card::QuickBall,
      sim::Card::BattleVipPass,
      sim::Card::EarthenVessel,
      sim::Card::Arven,
      sim::Card::Crispin,
      sim::Card::TapuLeleGX,
      sim::Card::LatiasEx,
      sim::Card::ProfessorBurnet,
      sim::Card::Fire,
      sim::Card::Grass,
      sim::Card::ForestSealStone,
      sim::Card::Dragapult,
      sim::Card::UltraBall,
      sim::Card::EvolutionIncense,
      sim::Card::PokemonCommunication,
      sim::Card::StevensResolve,
      sim::Card::Gladion,
      sim::Card::HisuianHeavyBall,
  };

  std::vector<Variant> result;
  result.push_back({"baseline", "none", "none", sim::baseline_recipe()});
  for (const sim::Card cut : cuts) {
    for (const sim::Card add : adds) {
      const auto variant = make_swap(cut, add);
      if (variant) result.push_back(*variant);
    }
  }
  return result;
}

void record(Stats& stats, const sim::EngineTestAccess::Snapshot& snapshot) {
  ++stats.trials;
  stats.ready += snapshot.ready ? 1U : 0U;
  stats.regi_in_play += snapshot.regi_in_play ? 1U : 0U;
  stats.vstar_in_play += snapshot.vstar_in_play ? 1U : 0U;
  stats.active_vstar += snapshot.active_vstar ? 1U : 0U;
  stats.energy_ready_any_regi += snapshot.energy_ready_any_regi ? 1U : 0U;
  stats.active_energy_ready += snapshot.active_energy_ready ? 1U : 0U;
  stats.payload_ready += snapshot.payload_ready ? 1U : 0U;
  stats.k1 += snapshot.k1 ? 1U : 0U;
  if (snapshot.ready) return;
  if (!snapshot.regi_in_play) ++stats.primary_no_regi;
  else if (!snapshot.vstar_in_play) ++stats.primary_no_vstar;
  else if (!snapshot.active_vstar) ++stats.primary_inactive_vstar;
  else if (!snapshot.active_energy_ready) ++stats.primary_energy;
  else if (!snapshot.payload_ready) ++stats.primary_payload;
  else ++stats.primary_other;
}

double pct(const std::uint64_t count, const std::uint64_t total) {
  return total == 0 ? 0.0 : 100.0 * static_cast<double>(count) /
      static_cast<double>(total);
}

std::string quoted(const std::string& value) {
  std::string escaped{"\""};
  for (const char character : value) {
    if (character == '"') escaped.push_back('"');
    escaped.push_back(character);
  }
  escaped.push_back('"');
  return escaped;
}

}  // namespace

int main(int argc, char** argv) {
  const std::uint64_t trials = argc >= 2 ? std::stoull(argv[1]) : 5000ULL;
  const std::filesystem::path output_dir = argc >= 3
      ? std::filesystem::path(argv[2])
      : std::filesystem::path("results/connector-swap-screen");
  if (trials == 0) return 2;

  const std::array scenario_labels{
      "strict-jit/go-first",
      "strict-jit/go-second",
      "matchup-flex-jit/go-first",
      "matchup-flex-jit/go-second",
  };
  const std::vector<Variant> deck_variants = variants();
  constexpr std::uint64_t base_seed = 202608090300ULL;
  constexpr std::uint64_t trial_stride = 104729ULL;
  constexpr std::uint64_t scenario_stride = 1000000007ULL;

  std::ostringstream csv;
  csv << "variant,cut,add,scenario,turn,attempted_trials,completed_trials,engine_error_pct,"
         "ready_pct,regi_in_play_pct,vstar_in_play_pct,active_vstar_pct,"
         "energy_ready_any_regi_pct,active_energy_ready_pct,payload_ready_pct,k1_pct,"
         "primary_no_regi_pct,primary_no_vstar_pct,primary_inactive_vstar_pct,"
         "primary_energy_pct,primary_payload_pct,primary_other_pct\n";
  std::ostringstream errors;
  errors << "variant,cut,add,scenario,error,count\n";

  for (std::size_t variant_index = 0; variant_index < deck_variants.size(); ++variant_index) {
    const Variant& variant = deck_variants[variant_index];
    for (std::size_t scenario_index = 0; scenario_index < scenario_labels.size(); ++scenario_index) {
      const auto found = sim::scenario_by_label(scenario_labels[scenario_index]);
      if (!found) throw std::logic_error("scenario disappeared");
      sim::Scenario scenario = *found;
      scenario.max_turn = 4;
      std::array<Stats, 3> stats{};
      std::map<std::string, std::uint64_t> scenario_errors;

      for (std::uint64_t trial = 0; trial < trials; ++trial) {
        const std::uint64_t trial_seed = base_seed +
            scenario_stride * static_cast<std::uint64_t>(scenario_index) +
            trial_stride * trial;
        try {
          std::mt19937_64 rng(trial_seed);
          sim::Engine engine(scenario, variant.recipe, rng);
          const auto snapshots = sim::EngineTestAccess::run_checkpoints(engine);
          for (std::size_t checkpoint = 0; checkpoint < snapshots.size(); ++checkpoint) {
            record(stats[checkpoint], snapshots[checkpoint]);
          }
        } catch (const std::exception& error) {
          ++scenario_errors[error.what()];
        }
      }

      const std::uint64_t completed = stats[0].trials;
      const std::uint64_t error_count = trials - completed;
      for (const auto& [message, count] : scenario_errors) {
        errors << quoted(variant.id) << ',' << quoted(variant.cut) << ','
               << quoted(variant.add) << ',' << quoted(scenario.label) << ','
               << quoted(message) << ',' << count << '\n';
      }
      for (std::size_t checkpoint = 0; checkpoint < stats.size(); ++checkpoint) {
        const Stats& s = stats[checkpoint];
        csv << quoted(variant.id) << ',' << quoted(variant.cut) << ','
            << quoted(variant.add) << ',' << quoted(scenario.label) << ','
            << (checkpoint + 2U) << ',' << trials << ',' << s.trials << ','
            << pct(error_count, trials) << ',' << pct(s.ready, s.trials) << ','
            << pct(s.regi_in_play, s.trials) << ',' << pct(s.vstar_in_play, s.trials) << ','
            << pct(s.active_vstar, s.trials) << ',' << pct(s.energy_ready_any_regi, s.trials) << ','
            << pct(s.active_energy_ready, s.trials) << ',' << pct(s.payload_ready, s.trials) << ','
            << pct(s.k1, s.trials) << ',' << pct(s.primary_no_regi, s.trials) << ','
            << pct(s.primary_no_vstar, s.trials) << ',' << pct(s.primary_inactive_vstar, s.trials) << ','
            << pct(s.primary_energy, s.trials) << ',' << pct(s.primary_payload, s.trials) << ','
            << pct(s.primary_other, s.trials) << '\n';
      }
    }
    if ((variant_index + 1U) % 20U == 0U || variant_index + 1U == deck_variants.size()) {
      std::cout << "completed " << (variant_index + 1U) << '/' << deck_variants.size() << " variants\n";
    }
  }

  std::filesystem::create_directories(output_dir);
  sim::write_atomic(output_dir / "checkpoint_summary.csv", csv.str());
  sim::write_atomic(output_dir / "engine_errors.csv", errors.str());
  std::cout << "screened " << deck_variants.size() << " variants x "
            << scenario_labels.size() << " scenarios x " << trials << " paired trials\n";
  return 0;
}
