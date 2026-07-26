from pathlib import Path
from textwrap import dedent

path = Path("src/trace_engine_v2/part_issue_991_wonder_tag_burnet_legacy_star_override.inc")
source = path.read_text(encoding="utf-8")
anchor = "  bool bench_tapu_if_useful() {\n"
insert = dedent(
    '''\
      bool issue_1514_hold_tapu_until_public_search() const {
        if (deck_seen_ || item_locked() || !supporter_allowed() ||
            hand_count(Card::TapuLeleGX) != 1 ||
            hand_count(Card::Crispin) == 0 || !need_regi() || !need_energy() ||
            bench_space() <= 0 ||
            !ability_available_for_pokemon(Card::TapuLeleGX) ||
            hand_count(Card::QuickBall) == 0 ||
            !might_be_unseen(Card::RegidragoV)) {
          return false;
        }

        // Quick Ball may be ranked from public K0 facts: it has a legal ordinary DCI
        // cost and a Regidrago V may remain unseen. Hold the physical Tapu Lele-GX until
        // that real Item resolves and establishes K1, then re-run the existing Wonder
        // Tag policy with legally known deck and Prize identities. This preflight never
        // executes a shadow search or inspects the hidden deck or Prizes:
        // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
        // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
        // Core Item, search, Bench, Ability, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // K0/K1 and future-card-oracle contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
        // DCI and connector-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1514
        return choose_discard(false, true, true, Card::QuickBall).has_value();
      }

      bool bench_tapu_if_useful() {
        if (issue_1514_hold_tapu_until_public_search()) {
          trace_tapu_hold_once(
              "issue-1514-k0-public-search-first",
              "R-QB-01; R-TAPU-01; R-CRISPIN-01; P-CONNECTOR-01; P-KNOWLEDGE-01; P-DCI-01",
              "Retained Tapu at K0 until a legal Quick Ball search establishes K1 and the duplicate-held-Crispin Wonder Tag can be evaluated from public information.");
          return false;
        }
    '''
)
if source.count(anchor) != 1:
    raise SystemExit(f"issue-1514 source anchor count: {source.count(anchor)}")
path.write_text(source.replace(anchor, insert, 1), encoding="utf-8")

Path("tests/issue_1514_k0_search_order_tests.cpp").write_text(
    dedent(
        r'''
        #define REGIDRAGO_SIM_NO_MAIN
        #include "../src/regidrago_sim.cpp"
        #include <algorithm>
        #include <iostream>
        #include <random>
        #include <stdexcept>
        #include <string>
        namespace sim { struct EngineTestAccess {}; }
        namespace {
        void expect(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
        bool contains(const sim::TraceLog& trace, const std::string& needle) { return std::any_of(trace.lines.begin(), trace.lines.end(), [&](const std::string& line) { return line.find(needle) != std::string::npos; }); }
        sim::TrialOutcome run(const std::string& label, std::uint64_t seed, sim::TraceLog& trace) {
          const auto scenario = sim::scenario_by_label(label); const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
          expect(scenario.has_value() && deck != nullptr, "Issue-1514 fixture unavailable");
          std::mt19937_64 rng{seed}; sim::Engine engine(*scenario, deck->recipe, rng, &trace); return engine.run();
        }
        void test_seed_33_searches_before_duplicate_crispin_wonder_tag() {
          sim::TraceLog trace{true, {}}; const auto outcome = run("no-discard-control/go-second", 33, trace);
          // The physical Quick Ball search establishes K1 before Wonder Tag is evaluated;
          // no hidden deck or Prize identity is consulted during the K0 hold decision.
          // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
          // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
          // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
          // Core search and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1514
          expect(outcome.first_ready_turn == 3 && !outcome.setup_failed, "Seed 33 missed T3 readiness");
          expect(contains(trace, "T1 | HOLD TAPU LELE-GX") && contains(trace, "T1 | PLAY ITEM") && contains(trace, "Quick Ball") && contains(trace, "T3 | READY"), "Seed 33 missed public search-first route");
          expect(!contains(trace, "T1 | WONDER TAG"), "Seed 33 still spent T1 Wonder Tag on duplicate Crispin");
        }
        }
        int main() { try { test_seed_33_searches_before_duplicate_crispin_wonder_tag(); std::cout << "Issue 1514 tests passed\n"; return 0; } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; } }
        '''
    ).lstrip(),
    encoding="utf-8",
)
