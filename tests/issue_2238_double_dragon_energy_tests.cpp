#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool pays_apex_energy_cost(const Engine& engine, const Pokemon& pokemon) {
    return engine.pays_apex_energy_cost(pokemon);
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static void search_basic_energy(Engine& engine) {
    engine.search_energy_to_hand(2, "R-EV-01; R-GAME-ITEM", "Issue-2238 Basic Energy search");
  }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool use_legacy_star(Engine& engine) {
    return engine.use_legacy_star();
  }
  static bool issue_1796_t2_route(const Engine& engine) {
    return engine.issue_1796_t2_steven_route_available();
  }
  static bool issue_1875_route(const Engine& engine) {
    return engine.issue_1875_quick_ball_tapu_crispin_route_available();
  }
  static bool issue_1895_route(const Engine& engine) {
    return engine.issue_1895_held_crispin_quick_ball_route_available();
  }
  static bool complete_issue_1599(Engine& engine) {
    return engine.complete_issue_1599_quick_ball_tapu_crispin_route();
  }
  static void establish_k1(Engine& engine) {
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool blender_energy_axis_can_finish_this_turn(Engine& engine) {
    return engine.blender_energy_axis_can_finish_this_turn();
  }
  static bool use_celestial_roar(Engine& engine) {
    return engine.use_celestial_roar();
  }
  static void arm_issue_1795_vessel_finish(Engine& engine, const int turn) {
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
    engine.issue_1795_vessel_turn_ = turn;
  }
  static bool complete_issue_1795_vessel_finish(Engine& engine) {
    return engine.complete_issue_1795_vessel_finish();
  }
  static bool record_ready(Engine& engine) {
    engine.record_ready();
    return engine.outcome_.first_ready_turn != 0;
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  explicit Fixture(const bool going_first = false,
                   const sim::DciProfile dci = sim::DciProfile::StrictJit)
      : scenario{"issue-2238/exact", dci,
                 sim::LockMode::None, going_first, 5},
        recipe(sim::double_dragon_modeling_recipe()),
        rng(2238),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {}
};

sim::Pokemon regi(const sim::Card card, const int grass = 0,
                  const int fire = 0, const int dde = 0) {
  sim::Pokemon pokemon{card, 1, grass, fire, sim::Tool::None};
  pokemon.double_dragon = dde;
  return pokemon;
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_dde_payment_combinations() {
  Fixture fixture;

  // DDE provides every Energy type and exactly two Energy while legally attached
  // to a Dragon Pokémon. One DDE plus either Basic Grass or Basic Fire therefore
  // pays Apex Dragon's GGF cost; two DDE also pay it:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(sim::EngineTestAccess::pays_apex_energy_cost(
              fixture.engine, regi(sim::Card::RegidragoVstar, 1, 0, 1)),
          "DDE plus Grass did not pay GGF.");
  require(sim::EngineTestAccess::pays_apex_energy_cost(
              fixture.engine, regi(sim::Card::RegidragoVstar, 0, 1, 1)),
          "DDE plus Fire did not pay GGF.");
  require(sim::EngineTestAccess::pays_apex_energy_cost(
              fixture.engine, regi(sim::Card::RegidragoVstar, 0, 0, 2)),
          "Two DDE did not pay GGF.");
  require(!sim::EngineTestAccess::pays_apex_energy_cost(
              fixture.engine, regi(sim::Card::RegidragoVstar, 0, 0, 1)),
          "One DDE incorrectly paid a three-Energy attack cost.");
}

void test_manual_attachment_prefers_compression() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoVstar, 1, 0, 0);
  state.hand = {sim::Card::DoubleDragonEnergy, sim::Card::Grass,
                sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // From one attached Basic Energy, DDE completes GGF in this one attachment while
  // either Basic Energy leaves only two total Energy. Earliest-route and AMR policy
  // therefore require DDE here:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(sim::EngineTestAccess::attach_manual(fixture.engine),
          "DDE compression attachment did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(after.active && after.active->double_dragon == 1,
          "DDE was not selected for the strictly faster attachment.");
  require(count(after.hand, sim::Card::DoubleDragonEnergy) == 0 &&
              count(after.hand, sim::Card::Grass) == 1 &&
              count(after.hand, sim::Card::Fire) == 1,
          "The DDE route consumed a lower-value Basic Energy instead.");
}

void test_manual_attachment_preserves_dde_on_equal_finish() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoVstar, 1, 1, 0);
  state.hand = {sim::Card::DoubleDragonEnergy, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Basic Grass and DDE both finish GGF immediately. Resource preservation keeps
  // the two-unit flexible Special Energy and spends the replaceable one-unit Basic:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(sim::EngineTestAccess::attach_manual(fixture.engine),
          "Equal-finish Basic attachment did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(after.active && after.active->grass == 2 &&
              after.active->double_dragon == 0,
          "The equal-turn route wasted DDE instead of Basic Grass.");
  require(count(after.hand, sim::Card::DoubleDragonEnergy) == 1,
          "DDE was not preserved on an equal earliest-ready route.");
}

void test_basic_energy_search_excludes_dde() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoVstar);
  state.deck = {sim::Card::DoubleDragonEnergy, sim::Card::Grass,
                sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Earthen Vessel and Vital Dance search Basic Energy. DDE is Special Energy and
  // must remain in deck when the shared Basic-Energy search helper resolves:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  sim::EngineTestAccess::search_basic_energy(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(count(after.hand, sim::Card::Grass) == 1 &&
              count(after.hand, sim::Card::Fire) == 1 &&
              count(after.deck, sim::Card::DoubleDragonEnergy) == 1,
          "A Basic-Energy search illegally selected DDE.");
}

void test_star_alchemy_takes_dde_when_it_is_strictly_faster() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoVstar, 1, 0, 0);
  state.active->tool = sim::Tool::ForestSealStone;
  state.deck = {sim::Card::DoubleDragonEnergy, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Klara};
  state.discard = {sim::Card::Dragapult};
  state.discarded_this_turn = {sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Star Alchemy searches any card. With one Basic already attached, DDE alone
  // turns the unused manual attachment into GGF; a Basic Energy cannot. The
  // unrestricted search must therefore choose DDE before longer connectors:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(sim::EngineTestAccess::use_fss(fixture.engine),
          "Star Alchemy did not resolve in the DDE exact state.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(count(after.hand, sim::Card::DoubleDragonEnergy) == 1,
          "Star Alchemy failed to choose the strictly faster DDE connector.");
}

void test_celestial_roar_attaches_dde_as_energy() {
  Fixture fixture;
  sim::State state;
  state.turn = 1;
  state.active = regi(sim::Card::RegidragoV, 1, 0, 0);
  // Engine pops from the back, so DDE is guaranteed among the processed top three.
  state.deck = {sim::Card::Klara, sim::Card::Fire, sim::Card::Grass,
                sim::Card::DoubleDragonEnergy};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Celestial Roar attaches Energy cards from its processed top three. DDE is an
  // Energy card and Regidrago V is Dragon, so it attaches legally and remains one
  // physical Special Energy providing two flexible Energy:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(sim::is_energy(sim::Card::DoubleDragonEnergy),
          "DDE was not classified as an Energy before Celestial Roar.");
  require(sim::is_dragon(sim::Card::RegidragoV),
          "Regidrago V was not classified as Dragon before Celestial Roar.");
  const bool used = sim::EngineTestAccess::use_celestial_roar(fixture.engine);
  require(used, "Celestial Roar did not resolve in the DDE exact state.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->double_dragon != 1) {
    std::cerr << "Celestial diagnostic: active=" << static_cast<bool>(after.active)
              << " grass=" << (after.active ? after.active->grass : -1)
              << " fire=" << (after.active ? after.active->fire : -1)
              << " dde=" << (after.active ? after.active->double_dragon : -1)
              << " deck_dde=" << count(after.deck, sim::Card::DoubleDragonEnergy)
              << " discard_dde=" << count(after.discard, sim::Card::DoubleDragonEnergy)
              << '\n';
  }
  require(after.active && after.active->double_dragon == 1,
          "Celestial Roar failed to attach DDE as an Energy card.");
}

void test_blender_energy_gate_accepts_dde_payment() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::RegidragoVstar, 0, 1, 1);
  state.hand = {sim::Card::BrilliantBlender};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // DDE + Fire is already a legal GGF payment. The strict-JIT Brilliant Blender
  // preflight must therefore agree with the engine's DDE-aware readiness predicate:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(sim::EngineTestAccess::blender_energy_axis_can_finish_this_turn(
              fixture.engine),
          "Brilliant Blender rejected an already-ready DDE + Fire attacker.");

  sim::State one_dde;
  one_dde.turn = 3;
  one_dde.active = regi(sim::Card::RegidragoVstar, 0, 0, 1);
  one_dde.hand = {sim::Card::BrilliantBlender, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(one_dde));
  require(sim::EngineTestAccess::blender_energy_axis_can_finish_this_turn(
              fixture.engine),
          "Brilliant Blender missed a one-Basic manual finish after DDE.");
}

void test_issue_1796_does_not_bank_redundant_crispin_for_dde() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None};
  state.bench = {regi(sim::Card::RegidragoV, 1, 0, 1)};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure,
                sim::Card::Dragapult};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  state.manual_energy_used = true;
  state.prizes = {sim::Card::QuickBall};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Grass + DDE already pays the projected GGF cost, so the historical issue-1796
  // Steven package must not reserve Crispin as though two Basic types were missing:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(!sim::EngineTestAccess::issue_1796_t2_route(fixture.engine),
          "Issue-1796 banked redundant Crispin for a DDE-ready Regidrago.");
}

void test_issue_1795_vessel_preserves_dde_ready_energy_axis() {
  Fixture fixture{true};
  sim::State state;
  state.turn = 4;
  state.active = regi(sim::Card::RegidragoV, 1, 1, 1);
  state.active->entered_turn = 3;
  state.hand = {sim::Card::RegidragoVstar, sim::Card::EarthenVessel,
                sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::LatiasEx};
  state.prizes = {sim::Card::QuickBall, sim::Card::Arven, sim::Card::Serena,
                  sim::Card::Klara, sim::Card::ProfessorTuro, sim::Card::PathToPeak};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::arm_issue_1795_vessel_finish(fixture.engine, 4);

  // Grass + Fire + DDE is already more than sufficient for GGF. Vessel remains
  // useful to put the strict-JIT Dragon payload into discard, but no extra Basic
  // search or manual attachment should be forced afterward:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(sim::EngineTestAccess::complete_issue_1795_vessel_finish(fixture.engine),
          "Issue-1795 failed to finish with an already DDE-ready Energy axis.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(after.active && after.active->card == sim::Card::RegidragoVstar &&
              after.active->double_dragon == 1,
          "Issue-1795 did not preserve the DDE-ready attacker.");
  require(!after.manual_energy_used,
          "Issue-1795 wasted a manual attachment after DDE already completed GGF.");
}

void test_issue_1875_rejects_redundant_crispin_when_dde_ready() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::RegidragoVstar, 1, 0, 1);
  state.hand = {sim::Card::Fire, sim::Card::QuickBall, sim::Card::Dragapult};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Arven, sim::Card::Serena, sim::Card::Klara,
                  sim::Card::ProfessorTuro, sim::Card::PathToPeak,
                  sim::Card::RoseannesBackup};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::establish_k1(fixture.engine);

  // Grass + DDE already pays Apex Dragon's GGF cost. The issue-1875 connector was
  // created to finish a missing Energy axis, so spending Fire, Quick Ball, Wonder
  // Tag, and Crispin here would be strictly dominated resource use:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(!sim::EngineTestAccess::issue_1875_route(fixture.engine),
          "Issue-1875 selected redundant Tapu-Crispin Energy work after DDE completed GGF.");
}

void test_issue_1895_rejects_redundant_crispin_when_dde_ready() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::RegidragoVstar, 1, 1, 1);
  state.hand = {sim::Card::QuickBall, sim::Card::Crispin,
                sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoV,
                sim::Card::LatiasEx};
  state.prizes = {sim::Card::Arven, sim::Card::Serena, sim::Card::Klara,
                  sim::Card::ProfessorTuro, sim::Card::PathToPeak,
                  sim::Card::RoseannesBackup};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::establish_k1(fixture.engine);

  // Grass + Fire + DDE already pays GGF. Issue-1895 may not consume Crispin merely
  // because its historical raw counters still look one Grass short:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(!sim::EngineTestAccess::issue_1895_route(fixture.engine),
          "Issue-1895 selected redundant Crispin after DDE completed GGF.");
}

void test_issue_1599_rejects_redundant_crispin_when_dde_ready() {
  Fixture fixture{false, sim::DciProfile::MatchupFlexJit};
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoVstar, 2, 0, 1);
  state.hand = {sim::Card::QuickBall, sim::Card::GoodraVstar};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoV};
  state.discard = {sim::Card::Dragapult};
  state.discarded_this_turn = {sim::Card::Dragapult};
  state.prizes = {sim::Card::Arven, sim::Card::Serena, sim::Card::Klara,
                  sim::Card::ProfessorTuro, sim::Card::PathToPeak,
                  sim::Card::RoseannesBackup};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::establish_k1(fixture.engine);

  // GG + DDE already pays GGF. The Matchup-Flex issue-1599 connector is only an
  // Energy-completion route and must not spend four resources to fetch Crispin:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(!sim::EngineTestAccess::complete_issue_1599(fixture.engine),
          "Issue-1599 entered a redundant Tapu-Crispin route after DDE completed GGF.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(std::find(after.hand.begin(), after.hand.end(), sim::Card::QuickBall) != after.hand.end() &&
              std::find(after.hand.begin(), after.hand.end(), sim::Card::GoodraVstar) != after.hand.end(),
          "Issue-1599 consumed resources before rejecting a DDE-ready state.");
}

void test_star_alchemy_does_not_fetch_duplicate_dde() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoVstar, 1, 0, 0);
  state.active->tool = sim::Tool::ForestSealStone;
  state.hand = {sim::Card::DoubleDragonEnergy};
  state.deck = {sim::Card::DoubleDragonEnergy, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Klara};
  state.discard = {sim::Card::Dragapult};
  state.discarded_this_turn = {sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The held DDE already turns the unused manual attachment into GGF. Spending the
  // one-use Star Alchemy to fetch the second DDE advances no setup axis and is
  // strictly dominated by preserving the VSTAR Power:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  const bool used = sim::EngineTestAccess::use_fss(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(count(after.hand, sim::Card::DoubleDragonEnergy) == 1,
          "Star Alchemy fetched a duplicate DDE that did not improve Energy progress.");
  require(!used,
          "Star Alchemy spent the one-use VSTAR Power when held DDE already solved Energy.");
}

void test_active_legacy_star_recovers_dde_when_strictly_best() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoVstar, 1, 0, 0);
  state.hand = {};
  // Engine pops from the back. These seven cards guarantee that Legacy Star mills
  // one DDE and a current-turn Dragon payload without exposing a higher-priority
  // Blender/Burnet/Latias/Incense recovery route.
  state.deck = {sim::Card::Klara, sim::Card::Fire, sim::Card::Grass,
                sim::Card::DoubleDragonEnergy, sim::Card::Dragapult,
                sim::Card::Serena, sim::Card::Arven};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Legacy Star may recover any two cards. From one attached Grass, recovered DDE
  // is the only single manual attachment that reaches GGF, so the *active wrapper*
  // must return DDE rather than a one-unit Basic Energy:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  require(sim::EngineTestAccess::use_legacy_star(fixture.engine),
          "Active Legacy Star wrapper did not resolve in the exact DDE recovery state.");
  const sim::State& after_power = sim::EngineTestAccess::state(fixture.engine);
  require(count(after_power.hand, sim::Card::DoubleDragonEnergy) == 1,
          "Active Legacy Star failed to recover DDE when it was the strictly best Energy.");
  require(sim::EngineTestAccess::attach_manual(fixture.engine),
          "Recovered DDE could not be attached manually.");
  const sim::State& after_attach = sim::EngineTestAccess::state(fixture.engine);
  require(after_attach.active && after_attach.active->double_dragon == 1 &&
              sim::EngineTestAccess::pays_apex_energy_cost(
                  fixture.engine, *after_attach.active),
          "Legacy-Star-recovered DDE did not complete GGF.");
}

void test_special_energy_copy_limit_and_model_recipe() {
  const sim::NamedDeck model{"regidrago-dde-model",
                             sim::double_dragon_modeling_recipe()};
  std::string error;
  require(sim::validate_recipe(model, &error),
          "The two-DDE modeling recipe was rejected.");

  sim::DeckRecipe illegal = model.recipe;
  for (auto& [card, copies] : illegal) {
    if (card == sim::Card::DoubleDragonEnergy) copies = 5;
    if (card == sim::Card::Grass) copies -= 3;
  }
  const sim::NamedDeck invalid{"regidrago-dde-five-copy", illegal};
  require(!sim::validate_recipe(invalid, &error),
          "Five copies of a Special Energy bypassed the four-copy limit.");
}

void test_card_classification() {
  require(sim::is_energy(sim::Card::DoubleDragonEnergy),
          "DDE is not classified as Energy.");
  require(!sim::is_basic_energy(sim::Card::DoubleDragonEnergy),
          "DDE was incorrectly classified as Basic Energy.");
  require(sim::is_dragon(sim::Card::RegidragoV) &&
              sim::is_dragon(sim::Card::RegidragoVstar) &&
              !sim::is_dragon(sim::Card::TapuLeleGX),
          "Dragon legality classification is incorrect for DDE attachment.");
}

}  // namespace

int main() {
  try {
    test_dde_payment_combinations();
    test_manual_attachment_prefers_compression();
    test_manual_attachment_preserves_dde_on_equal_finish();
    test_basic_energy_search_excludes_dde();
    test_star_alchemy_takes_dde_when_it_is_strictly_faster();
    test_celestial_roar_attaches_dde_as_energy();
    test_blender_energy_gate_accepts_dde_payment();
    test_issue_1796_does_not_bank_redundant_crispin_for_dde();
    test_issue_1795_vessel_preserves_dde_ready_energy_axis();
    test_issue_1875_rejects_redundant_crispin_when_dde_ready();
    test_issue_1895_rejects_redundant_crispin_when_dde_ready();
    test_issue_1599_rejects_redundant_crispin_when_dde_ready();
    test_star_alchemy_does_not_fetch_duplicate_dde();
    test_active_legacy_star_recovers_dde_when_strictly_best();
    test_special_energy_copy_limit_and_model_recipe();
    test_card_classification();
    std::cout << "Issue 2238 Double Dragon Energy tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}