from pathlib import Path
from textwrap import dedent

helper_path = Path("src/trace_engine_v2/part_006.inc")
helper_source = helper_path.read_text(encoding="utf-8")
helper_anchor = (
    "  std::optional<Card> choose_discard(const bool permit_payload, const bool flex_fodder,\n"
)
helper_insert = dedent(
    '''\
      std::optional<Card> issue_1605_redundant_payload_cost() const {
        int held_payloads = 0;
        int distinct_payloads = 0;
        for (const Card card : {Card::Appletun, Card::MegaDragonite,
                                Card::Dragapult, Card::GoodraVstar,
                                Card::DialgaGX}) {
          const int copies = hand_count(card);
          held_payloads += copies;
          distinct_payloads += copies > 0 ? 1 : 0;
        }
        if (held_payloads < 3 || distinct_payloads < 2) return std::nullopt;
        for (const Card card : {Card::Appletun, Card::MegaDragonite,
                                Card::Dragapult, Card::GoodraVstar,
                                Card::DialgaGX}) {
          if (hand_count(card) >= 2) return card;
        }
        return std::nullopt;
      }

      bool issue_1605_arven_crobat_route_available() const {
        const bool one_discard_item_plausible =
            might_be_unseen(Card::QuickBall) ||
            might_be_unseen(Card::MysteriousTreasure);
        return scenario_.dci == DciProfile::StrictJit &&
               !scenario_.going_first && state_.turn == 1 &&
               supporter_allowed() && !item_locked() && need_regi() &&
               bench_space() >= 2 && hand_count(Card::Arven) > 0 &&
               hand_count(Card::CrobatV) > 0 &&
               hand_count(Card::ForestSealStone) > 0 &&
               !state_.vstar_power_used && !state_.dark_asset_used &&
               ability_available_for_pokemon(Card::CrobatV) &&
               state_.hand.size() <= 9U &&
               might_be_unseen(Card::RegidragoV) &&
               one_discard_item_plausible &&
               issue_1605_redundant_payload_cost().has_value();
      }

    '''
)
if helper_source.count(helper_anchor) != 1:
    raise SystemExit(f"issue-1605 helper anchor count: {helper_source.count(helper_anchor)}")
helper_path.write_text(
    helper_source.replace(helper_anchor, helper_insert + helper_anchor, 1),
    encoding="utf-8",
)

arven_path = Path("src/trace_engine_v2/part_012.inc")
arven_source = arven_path.read_text(encoding="utf-8")
member_anchor = "  bool play_arven() {\n"
member_insert = dedent(
    '''\
      bool issue_1605_arven_redundant_payload_route_{false};

      bool play_arven() {
    '''
)
if arven_source.count(member_anchor) != 1:
    raise SystemExit(f"issue-1605 member anchor count: {arven_source.count(member_anchor)}")
arven_source = arven_source.replace(member_anchor, member_insert, 1)

cost_anchor = dedent(
    '''\
        const bool can_pay_search_cost =
            is_legal_cost_available(can_play_payload_this_turn(), true) ||
            final_energy_vessel_dead_role_cost;
    '''
)
cost_insert = dedent(
    '''\
        const bool issue_1605_public_arven_route =
            issue_1605_arven_crobat_route_available();
        // A duplicated Dragon may pay the one-discard Item selected by Arven only in
        // this complete public K0 continuation. One copy of that identity and a
        // different modeled payload remain protected, Regidrago V advances the Basic
        // axis, Forest Seal Stone opens Star Alchemy, and Crobat V converts its UDP
        // hand role into Dark Asset after the hand is compressed:
        // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
        // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
        // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
        // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
        // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
        // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
        // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
        // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
        // Core Supporter, Item, discard, search, Bench, Tool, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // K0/K1, dynamic DCI, UDP, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1605
        const bool can_pay_search_cost =
            is_legal_cost_available(can_play_payload_this_turn(), true) ||
            final_energy_vessel_dead_role_cost ||
            issue_1605_public_arven_route;
    '''
)
if arven_source.count(cost_anchor) != 1:
    raise SystemExit(f"issue-1605 Arven cost anchor count: {arven_source.count(cost_anchor)}")
arven_source = arven_source.replace(cost_anchor, cost_insert, 1)

found_anchor = (
    "    if (live_tool && move_deck_to_hand(Card::ForestSealStone)) found.push_back(Card::ForestSealStone);\n"
    "    shuffle(state_.deck);\n"
)
found_insert = (
    "    if (live_tool && move_deck_to_hand(Card::ForestSealStone)) found.push_back(Card::ForestSealStone);\n"
    "    issue_1605_arven_redundant_payload_route_ =\n"
    "        issue_1605_public_arven_route &&\n"
    "        std::any_of(found.begin(), found.end(), [](const Card card) {\n"
    "          return card == Card::QuickBall || card == Card::MysteriousTreasure;\n"
    "        });\n"
    "    shuffle(state_.deck);\n"
)
if arven_source.count(found_anchor) != 1:
    raise SystemExit(f"issue-1605 Arven found anchor count: {arven_source.count(found_anchor)}")
arven_path.write_text(
    arven_source.replace(found_anchor, found_insert, 1), encoding="utf-8"
)

quick_path = Path("src/trace_engine_v2/part_009b1.inc")
quick_source = quick_path.read_text(encoding="utf-8")
quick_anchor = (
    "    if (!cost && want_regi && hand_count(Card::RegidragoVstar) >= 2) {\n"
)
quick_insert = dedent(
    '''\
        if (!cost && want_regi && issue_1605_arven_redundant_payload_route_) {
          // Arven already established K1 and selected this exact one-discard search
          // Item. Spend only the duplicated payload identity, after all ordinary DCI
          // costs fail, while two distinct Dragon payloads survive for later strict-JIT:
          // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
          // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
          // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
          // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
          // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1605
          cost = issue_1605_redundant_payload_cost();
        }
        if (!cost && want_regi && hand_count(Card::RegidragoVstar) >= 2) {
    '''
)
if quick_source.count(quick_anchor) != 1:
    raise SystemExit(f"issue-1605 Quick Ball anchor count: {quick_source.count(quick_anchor)}")
quick_source = quick_source.replace(quick_anchor, quick_insert, 1)
quick_clear_anchor = (
    "    remove_one(state_.hand, Card::QuickBall);\n"
    "    state_.discard.push_back(Card::QuickBall);\n"
)
quick_clear_insert = (
    "    remove_one(state_.hand, Card::QuickBall);\n"
    "    issue_1605_arven_redundant_payload_route_ = false;\n"
    "    state_.discard.push_back(Card::QuickBall);\n"
)
if quick_source.count(quick_clear_anchor) != 1:
    raise SystemExit(f"issue-1605 Quick Ball clear anchor count: {quick_source.count(quick_clear_anchor)}")
quick_path.write_text(
    quick_source.replace(quick_clear_anchor, quick_clear_insert, 1),
    encoding="utf-8",
)

treasure_path = Path("src/trace_engine_v2/part_009a.inc")
treasure_source = treasure_path.read_text(encoding="utf-8")
treasure_anchor = (
    "    if (!cost && want_regi && issue_1304_t1_treasure_erika_cost_available()) {\n"
)
treasure_insert = dedent(
    '''\
        if (!cost && want_regi && issue_1605_arven_redundant_payload_route_) {
          // Arven already established K1 and selected this exact one-discard search
          // Item. Spend only the duplicated payload identity, after all ordinary DCI
          // costs fail, while two distinct Dragon payloads survive for later strict-JIT:
          // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
          // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
          // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
          // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
          // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1605
          cost = issue_1605_redundant_payload_cost();
        }
        if (!cost && want_regi && issue_1304_t1_treasure_erika_cost_available()) {
    '''
)
if treasure_source.count(treasure_anchor) != 1:
    raise SystemExit(f"issue-1605 Treasure anchor count: {treasure_source.count(treasure_anchor)}")
treasure_source = treasure_source.replace(treasure_anchor, treasure_insert, 1)
treasure_clear_anchor = (
    "    remove_one(state_.hand, Card::MysteriousTreasure);\n"
    "    state_.discard.push_back(Card::MysteriousTreasure);\n"
)
treasure_clear_insert = (
    "    remove_one(state_.hand, Card::MysteriousTreasure);\n"
    "    issue_1605_arven_redundant_payload_route_ = false;\n"
    "    state_.discard.push_back(Card::MysteriousTreasure);\n"
)
if treasure_source.count(treasure_clear_anchor) != 1:
    raise SystemExit(f"issue-1605 Treasure clear anchor count: {treasure_source.count(treasure_clear_anchor)}")
treasure_path.write_text(
    treasure_source.replace(treasure_clear_anchor, treasure_clear_insert, 1),
    encoding="utf-8",
)

Path("tests/issue_1605_arven_redundant_payload_tests.cpp").write_text(
    dedent(
        r'''
        #define REGIDRAGO_SIM_NO_MAIN
        #include "../src/regidrago_sim.cpp"

        #include <algorithm>
        #include <random>
        #include <stdexcept>
        #include <string>

        namespace sim {
        struct EngineTestAccess {
          static State& state(Engine& engine) { return engine.state_; }
          static bool route(const Engine& engine) {
            return engine.issue_1605_arven_crobat_route_available();
          }
        };
        }

        namespace {
        void expect(bool condition, const char* message) {
          if (!condition) throw std::runtime_error(message);
        }
        bool has(const sim::TraceLog& trace, const std::string& needle) {
          return std::any_of(trace.lines.begin(), trace.lines.end(),
                             [&](const std::string& line) {
                               return line.find(needle) != std::string::npos;
                             });
        }

        sim::Engine make_unit_engine(sim::Scenario& scenario,
                                     sim::DeckRecipe& recipe,
                                     std::mt19937_64& rng) {
          return sim::Engine(scenario, recipe, rng);
        }

        void install_public_state(sim::Engine& engine) {
          auto& state = sim::EngineTestAccess::state(engine);
          state.turn = 1;
          state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                                      sim::Tool::None};
          state.hand = {sim::Card::Arven, sim::Card::MegaDragonite,
                        sim::Card::MegaDragonite, sim::Card::GoodraVstar,
                        sim::Card::TeamYellsCheer, sim::Card::CrobatV,
                        sim::Card::ForestSealStone};
          state.deck = {sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                        sim::Card::RegidragoV};
        }

        bool route_for(sim::LockMode locks, std::vector<sim::Card> hand,
                       int bench_count = 0, int accounted_regi = 0) {
          sim::Scenario scenario{"issue-1605-unit", sim::DciProfile::StrictJit,
                                 locks, false, 5};
          sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
          std::mt19937_64 rng{1};
          sim::Engine engine = make_unit_engine(scenario, recipe, rng);
          install_public_state(engine);
          auto& state = sim::EngineTestAccess::state(engine);
          state.hand = std::move(hand);
          for (int i = 0; i < bench_count; ++i) {
            state.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 0, 0, 0,
                                                sim::Tool::None});
          }
          for (int i = 0; i < accounted_regi; ++i) {
            state.discard.push_back(sim::Card::RegidragoV);
          }
          return sim::EngineTestAccess::route(engine);
        }

        void test_public_controls() {
          const std::vector<sim::Card> exact{
              sim::Card::Arven, sim::Card::MegaDragonite,
              sim::Card::MegaDragonite, sim::Card::GoodraVstar,
              sim::Card::TeamYellsCheer, sim::Card::CrobatV,
              sim::Card::ForestSealStone};
          expect(route_for(sim::LockMode::None, exact),
                 "The exact public Arven route was rejected.");
          expect(!route_for(sim::LockMode::None,
                            {sim::Card::Arven, sim::Card::MegaDragonite,
                             sim::Card::TeamYellsCheer, sim::Card::CrobatV,
                             sim::Card::ForestSealStone}),
                 "A single payload was exposed.");
          expect(!route_for(sim::LockMode::None,
                            {sim::Card::Arven, sim::Card::MegaDragonite,
                             sim::Card::MegaDragonite,
                             sim::Card::MegaDragonite,
                             sim::Card::TeamYellsCheer, sim::Card::CrobatV,
                             sim::Card::ForestSealStone}),
                 "The route failed to preserve a distinct payload identity.");
          expect(!route_for(sim::LockMode::FullItem, exact),
                 "The route ignored Item lock.");
          expect(!route_for(sim::LockMode::FullSupporter, exact),
                 "The route ignored Supporter lock.");
          expect(!route_for(sim::LockMode::FullRuleBoxAbility, exact),
                 "The route ignored Rule Box Ability lock.");
          expect(!route_for(sim::LockMode::None, exact, 5),
                 "The route ignored a full Bench.");
          expect(!route_for(sim::LockMode::None, exact, 0, 4),
                 "The route invented an absent Regidrago V target.");
          auto large_hand = exact;
          large_hand.insert(large_hand.end(),
                            {sim::Card::Lusamine, sim::Card::Channeler,
                             sim::Card::RoseannesBackup});
          expect(!route_for(sim::LockMode::None, large_hand),
                 "The route used Crobat when the projected hand already had six cards.");
        }

        void test_seed_7_executes_arven_crobat_route() {
          const auto scenario = sim::scenario_by_label("strict-jit/go-second");
          const auto* deck = sim::crobat_modeling_deck_by_id("crobat1-erika");
          expect(scenario && deck, "Issue-1605 fixture unavailable.");
          std::mt19937_64 rng{7};
          sim::TraceLog trace{true, {}};
          sim::Engine engine(*scenario, deck->recipe, rng, &trace);
          engine.run();

          // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
          // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
          // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
          // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
          // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
          // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
          // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
          // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
          // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // Dynamic DCI and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1605
          expect(has(trace, "T1 | PLAY SUPPORTER") && has(trace, "Arven") &&
                     has(trace, "T1 | DISCARD") &&
                     has(trace, "Mega Dragonite ex") &&
                     has(trace, "T1 | BENCH") && has(trace, "Regidrago V") &&
                     has(trace, "Forest Seal Stone") &&
                     has(trace, "Crobat V") && has(trace, "Dark Asset"),
                 "Seed 7 did not execute the public Arven-Crobat continuation.");
        }
        }

        int main() {
          test_public_controls();
          test_seed_7_executes_arven_crobat_route();
        }
        '''
    ).lstrip(),
    encoding="utf-8",
)
