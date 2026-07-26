from pathlib import Path
from textwrap import dedent

source_path = Path("src/trace_engine_v2/part_issue_1118_secret_box.inc")
source = source_path.read_text(encoding="utf-8")
old = "    if (!fire_held && !fss_fire && !crispin_fire && !vessel_fire) return false;\n"
new = dedent(
    '''\
        // An already satisfied Fire requirement must not block a same-turn Secret Box
        // route whose only missing Energy is Grass. Dawn, Pineco, Forretress ex, and
        // Exploding Energy can provide that Grass while held Mysterious Treasure pays
        // the current-turn payload outlet:
        // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
        // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
        // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
        // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
        // Core Energy, Supporter, Item, Ability, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1594
        if (fire_needed() > 0 && !fire_held && !fss_fire && !crispin_fire &&
            !vessel_fire) {
          return false;
        }
    '''
)
if source.count(old) != 1:
    raise SystemExit(f"issue-1594 source anchor count: {source.count(old)}")
source_path.write_text(source.replace(old, new, 1), encoding="utf-8")

Path("tests/issue_1594_secret_box_grass_only_tests.cpp").write_text(
    dedent(
        r'''
        #define REGIDRAGO_SIM_NO_MAIN
        #include "../src/regidrago_sim.cpp"

        #include <algorithm>
        #include <cstdint>
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

        struct Result {
          sim::TrialOutcome outcome;
          sim::TraceLog trace;
        };

        Result run(const std::string& scenario_label, const std::uint64_t seed) {
          const auto scenario = sim::scenario_by_label(scenario_label);
          const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
          expect(scenario.has_value() && deck != nullptr,
                 "The issue-1594 fixture is unavailable.");
          std::mt19937_64 rng{seed};
          sim::TraceLog trace{true, {}};
          sim::Engine engine(*scenario, deck->recipe, rng, &trace);
          return {engine.run(), std::move(trace)};
        }

        void test_seed_72_uses_grass_only_secret_box_completion() {
          const Result result = run("strict-jit/go-first", 72);

          // Fire is already complete, so Secret Box may preserve the visible Dawn,
          // Pineco, Forretress ex, Exploding Energy, and Mysterious Treasure route:
          // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
          // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
          // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
          // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
          // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1594
          expect(result.outcome.first_ready_turn == 4 && !result.outcome.setup_failed,
                 "Seed 72 did not reach strict-JIT readiness on turn four.");
          expect(trace_contains(result.trace, "T4 | PLAY ITEM") &&
                     trace_contains(result.trace, "Secret Box discarded three other cards") &&
                     trace_contains(result.trace, "T4 | PLAY SUPPORTER") &&
                     trace_contains(result.trace, "Dawn searched") &&
                     trace_contains(result.trace, "T4 | USE ABILITY") &&
                     trace_contains(result.trace, "T4 | READY"),
                 "Seed 72 did not execute the grass-only Secret Box route.");
        }

        void test_existing_routes_remain_live() {
          const Result fast = run("strict-jit/go-second", 35);
          expect(fast.outcome.first_ready_turn == 2 && !fast.outcome.setup_failed,
                 "Seed 35 control regressed.");
          const Result star_alchemy = run("strict-jit/go-first", 16);
          expect(star_alchemy.outcome.first_ready_turn == 3 &&
                     !star_alchemy.outcome.setup_failed,
                 "Seed 16 Star Alchemy control regressed.");
        }
        }

        int main() {
          test_seed_72_uses_grass_only_secret_box_completion();
          test_existing_routes_remain_live();
        }
        '''
    ).lstrip(),
    encoding="utf-8",
)
