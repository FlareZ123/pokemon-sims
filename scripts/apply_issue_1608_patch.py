from pathlib import Path
from textwrap import dedent

source_path = Path("src/regidrago_sim.cpp")
source = source_path.read_text(encoding="utf-8")
anchor = (
    '#define play_gladion play_gladion_issue1595_original\n'
    '#include "trace_engine_v2/part_issue_1191_gladion_steven_override.inc"\n'
    '#undef play_gladion\n'
    '#include "trace_engine_v2/part_issue_1595_gladion_grass_turo_blender_override.inc"\n'
)
replacement = (
    '#define play_gladion play_gladion_issue1595_original\n'
    '#include "trace_engine_v2/part_issue_1191_gladion_steven_override.inc"\n'
    '#undef play_gladion\n'
    '#define play_gladion play_gladion_issue1608_original\n'
    '#include "trace_engine_v2/part_issue_1595_gladion_grass_turo_blender_override.inc"\n'
    '#undef play_gladion\n'
    '#include "trace_engine_v2/part_issue_1608_burnet_before_dead_crispin_override.inc"\n'
)
if source.count(anchor) != 1:
    raise SystemExit(f"issue-1608 Gladion include anchor count: {source.count(anchor)}")
source_path.write_text(source.replace(anchor, replacement, 1), encoding="utf-8")

Path("src/trace_engine_v2/part_issue_1608_burnet_before_dead_crispin_override.inc").write_text(
    dedent(
        '''\
          bool play_gladion() {
            const bool complete_benched_regidrago_v = std::any_of(
                state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
                  return pokemon.card == Card::RegidragoV && pokemon.grass >= 2 &&
                      pokemon.fire >= 1;
                });
            const bool issue_1608_burnet_before_dead_crispin =
                scenario_.dci == DciProfile::NoDiscardControl &&
                scenario_.locks == LockMode::None && state_.turn == 3 &&
                prizes_known() && supporter_allowed() && !need_energy() &&
                need_payload() && state_.active &&
                state_.active->card == Card::TapuLeleGX &&
                complete_benched_regidrago_v &&
                hand_count(Card::Gladion) > 0 &&
                hand_count(Card::ProfessorBurnet) > 0 &&
                prize_count_after_reveal(Card::Crispin) > 0 &&
                prize_count_after_reveal(Card::RegidragoV) == 0 &&
                prize_count_after_reveal(Card::RegidragoVstar) == 0 &&
                professor_burnet_has_live_ready_turn_route();
            if (!issue_1608_burnet_before_dead_crispin) {
              return play_gladion_issue1608_original();
            }

            // The Benched Regidrago V already has GGF, so the revealed prized Crispin
            // advances no unresolved setup axis. Held Professor Burnet can legally
            // search the deck for Dragon payloads, discard them, and shuffle. Preserve
            // the Supporter permission for that immediate public payload progress. Do
            // not project a specific next draw through Burnet's required shuffle:
            // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
            // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
            // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
            // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
            // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
            // Core Supporter, deck-search, discard, and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
            // K1, no-discard-control, dynamic DCI, and earliest-axis policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
            // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1608
            if (trace_ != nullptr) {
              trace_->add_policy_once(
                  state_, "issue-1608-burnet-before-dead-crispin", state_.turn,
                  "HOLD SUPPORTER",
                  "R-GLADION-01; R-BURNET-01; R-CRISPIN-01; P-DCI-01; P-COMPRESS-01; P-KNOWLEDGE-01",
                  "Held Gladion because prized Crispin cannot improve complete GGF and Professor Burnet immediately advances payload.");
            }
            return false;
          }
        '''
    ),
    encoding="utf-8",
)

Path("tests/issue_1608_burnet_before_dead_crispin_tests.cpp").write_text(
    dedent(
        '''\
        #define REGIDRAGO_SIM_NO_MAIN
        #include "../src/regidrago_sim.cpp"

        #include <algorithm>
        #include <random>
        #include <stdexcept>
        #include <string>
        #include <utility>

        namespace sim { struct EngineTestAccess {}; }

        namespace {
        void expect(const bool condition, const char* message) {
          if (!condition) throw std::runtime_error(message);
        }

        bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
          return std::any_of(trace.lines.begin(), trace.lines.end(),
                             [&needle](const std::string& line) {
                               return line.find(needle) != std::string::npos;
                             });
        }

        struct SeedResult {
          sim::TrialOutcome outcome;
          sim::TraceLog trace;
        };

        SeedResult run_crobat_seed(const std::string& variant_id,
                                   const std::string& scenario_label,
                                   const std::uint64_t seed) {
          const auto scenario = sim::scenario_by_label(scenario_label);
          const sim::CrobatModelingDeck* deck =
              sim::crobat_modeling_deck_by_id(variant_id);
          expect(scenario.has_value() && deck != nullptr,
                 "The issue-1608 Crobat fixture is unavailable.");
          std::mt19937_64 rng{seed};
          sim::TraceLog trace{true, {}};
          sim::Engine engine(*scenario, deck->recipe, rng, &trace);
          return {engine.run(), std::move(trace)};
        }

        SeedResult run_shell_seed(const std::string& scenario_label,
                                  const std::uint64_t seed) {
          const auto scenario = sim::scenario_by_label(scenario_label);
          const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
          expect(scenario.has_value() && deck != nullptr,
                 "The issue-971 control fixture is unavailable.");
          std::mt19937_64 rng{seed};
          sim::TraceLog trace{true, {}};
          sim::Engine engine(*scenario, deck->recipe, rng, &trace);
          return {engine.run(), std::move(trace)};
        }

        void test_seed_33_uses_burnet_before_dead_crispin() {
          const SeedResult result = run_crobat_seed(
              "crobat2-erika-channeler", "no-discard-control/go-first", 33);

          // GGF is already complete, so Crispin has zero immediate Energy-axis value.
          // Professor Burnet legally searches and discards the two deck payloads, then
          // shuffles. The regression asserts the observable T3 choice and never assumes
          // a particular post-shuffle T4 draw:
          // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
          // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
          // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
          // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
          // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
          // Core Supporter, search, discard, and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1608
          expect(trace_contains(result.trace,
                                "Held Gladion because prized Crispin cannot improve complete GGF") &&
                     trace_contains(result.trace,
                                    "T3 | PLAY SUPPORTER | rules: R-BURNET-01") &&
                     trace_contains(result.trace, "Mega Dragonite ex") &&
                     trace_contains(result.trace, "Dragapult ex") &&
                     !trace_contains(result.trace,
                                     "T3 | PLAY SUPPORTER | rules: R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01 | Looked at Prize cards and exchanged Gladion for Crispin"),
                 "Crobat seed 33 did not prefer observable Burnet payload progress over dead Crispin.");
        }

        void test_live_prized_crispin_route_remains_available() {
          const SeedResult result = run_shell_seed("strict-jit/go-second", 28);
          // Crispin remains live when Energy is unresolved, preserving completed #971:
          // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
          // Prior regression: https://github.com/FlareZ123/pokemon-sims/issues/971
          expect(result.outcome.first_ready_turn == 2 && !result.outcome.setup_failed,
                 "The live prized-Crispin seed lost its turn-two route.");
          expect(trace_contains(result.trace, "exchanged Gladion for Crispin"),
                 "The live prized-Crispin control no longer selects Crispin.");
        }
        }

        int main() {
          test_seed_33_uses_burnet_before_dead_crispin();
          test_live_prized_crispin_route_remains_available();
          return 0;
        }
        '''
    ),
    encoding="utf-8",
)
