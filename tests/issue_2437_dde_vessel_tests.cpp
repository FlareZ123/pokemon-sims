#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static std::optional<Card> completing_basic(
      const Engine& engine, const Pokemon& pokemon,
      const bool grass_available, const bool fire_available) {
    return engine.issue_2437_apex_completing_basic(
        pokemon, grass_available, fire_available);
  }
  static bool burnet_visible(const Engine& engine) {
    return engine.issue_1646_vessel_burnet_finish_visible();
  }
  static bool play_burnet_finish(Engine& engine) {
    return engine.play_issue_1646_vessel_burnet_finish();
  }
  static bool play_latias_finish(Engine& engine) {
    return engine.play_issue_1672_vessel_payload_finish();
  }
  static bool play_active_vstar_finish(Engine& engine) {
    return engine.play_issue_1868_vessel_payload_finish();
  }
  static bool run_search_item(Engine& engine, const bool permit_payload) {
    return engine.run_search_items_one_step(permit_payload);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

struct Fixture {
  sim::Scenario scenario{"issue-2437", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2437};
  sim::Engine engine{scenario, recipe, rng};
};

sim::Pokemon dde_regi(const sim::Card card, const int basic_kind = 0) {
  sim::Pokemon pokemon{card, 1, basic_kind == 1 ? 1 : 0,
                       basic_kind == 2 ? 1 : 0, sim::Tool::None, 1};
  return pokemon;
}

bool hand_has(const sim::State& state, const sim::Card card) {
  return std::find(state.hand.begin(), state.hand.end(), card) !=
      state.hand.end();
}

bool discard_has(const sim::State& state, const sim::Card card) {
  return std::find(state.discard.begin(), state.discard.end(), card) !=
      state.discard.end();
}

void test_shared_projection_accepts_either_basic() {
  Fixture fixture;
  const sim::Pokemon dde_only = dde_regi(sim::Card::RegidragoVstar);

  // DDE supplies two units of every type to a Dragon. One additional Grass or
  // Fire is therefore one physical attachment from Apex Dragon's GGF cost.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Energy attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
  require(sim::EngineTestAccess::completing_basic(
              fixture.engine, dde_only, true, false) == sim::Card::Grass,
          "DDE plus Grass was not projected as a one-attachment Apex finish.");
  require(sim::EngineTestAccess::completing_basic(
              fixture.engine, dde_only, false, true) == sim::Card::Fire,
          "DDE plus Fire was not projected as a one-attachment Apex finish.");
}

void test_burnet_finish_with_dde_basic(const sim::Card basic) {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = dde_regi(sim::Card::RegidragoVstar);
  state.hand = {sim::Card::EarthenVessel, sim::Card::ProfessorBurnet,
                sim::Card::FieldBlower};
  state.deck = {basic, sim::Card::MegaDragonite, sim::Card::Grass,
                sim::Card::Fire};
  state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(),
                                  [basic](const sim::Card card) {
                                    return (basic == sim::Card::Grass &&
                                            card == sim::Card::Fire) ||
                                           (basic == sim::Card::Fire &&
                                            card == sim::Card::Grass);
                                  }),
                   state.deck.end());
  state.prizes = {sim::Card::QuickBall, sim::Card::RegidragoV,
                  sim::Card::RegidragoVstar, sim::Card::Gladion,
                  sim::Card::Crispin, sim::Card::MysteriousTreasure};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Vessel searches the specific one-card semantic completion while Burnet uses
  // the only Supporter play for the current-turn payload.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, Supporter, search, discard, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
  require(sim::EngineTestAccess::burnet_visible(fixture.engine),
          "DDE one-Basic Vessel-Burnet route was not visible.");
  require(sim::EngineTestAccess::play_burnet_finish(fixture.engine),
          "DDE one-Basic Vessel-Burnet route did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(hand_has(after, basic),
          "Vessel did not search the semantic one-Basic completion.");
  require(discard_has(after, sim::Card::MegaDragonite),
          "Burnet did not establish the current-turn Dragon payload.");
  require(after.supporter_used,
          "Burnet did not consume the Supporter action.");
}

void test_active_vstar_payload_with_dde_basic(const sim::Card basic) {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = dde_regi(sim::Card::RegidragoVstar);
  state.hand = {sim::Card::EarthenVessel, sim::Card::MegaDragonite};
  state.deck = {basic};
  state.prizes = {sim::Card::QuickBall, sim::Card::RegidragoV,
                  sim::Card::RegidragoVstar, sim::Card::Gladion,
                  sim::Card::Crispin, sim::Card::MysteriousTreasure};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Vessel's mandatory discard is the strict-JIT payload, and the searched Basic
  // is the one physical attachment that completes DDE-backed Apex payment.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
  require(sim::EngineTestAccess::play_active_vstar_finish(fixture.engine),
          "DDE Active-VSTAR Vessel payload route did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(hand_has(after, basic),
          "Active-VSTAR route searched the wrong Basic completion.");
  require(discard_has(after, sim::Card::MegaDragonite),
          "Active-VSTAR route did not discard the Dragon payload.");
}

void test_latias_payload_with_dde_basic(const sim::Card basic) {
  sim::Scenario scenario{"issue-2437-latias", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2438};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 4;
  state.supporter_used = true;
  state.vstar_power_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None, 0};
  sim::Pokemon setup = dde_regi(sim::Card::RegidragoV);
  setup.tool = sim::Tool::ForestSealStone;
  state.bench.push_back(setup);
  state.hand = {sim::Card::EarthenVessel, sim::Card::LatiasEx,
                sim::Card::RegidragoVstar, sim::Card::MegaDragonite};
  state.deck = {basic};
  state.prizes = {sim::Card::QuickBall, sim::Card::Gladion,
                  sim::Card::Crispin, sim::Card::MysteriousTreasure,
                  sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The prior-turn DDE Regidrago V is one Basic attachment from Apex. Vessel can
  // discard the Dragon payload, search either legal Basic, then the held VSTAR and
  // Latias promotion line retain their existing evolution/position gates.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
  require(sim::EngineTestAccess::play_latias_finish(engine),
          "DDE Latias Vessel payload route did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  require(hand_has(after, basic),
          "Latias route searched the wrong Basic completion.");
  require(discard_has(after, sim::Card::MegaDragonite),
          "Latias route did not discard the Dragon payload.");
}

void test_post_crispin_ready_turn_does_not_hold_vessel(
    const int attached_basic_kind) {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.supporter_used = true;
  state.active = dde_regi(sim::Card::RegidragoVstar, attached_basic_kind);
  state.hand = {sim::Card::EarthenVessel, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::QuickBall, sim::Card::RegidragoV,
                  sim::Card::RegidragoVstar, sim::Card::Gladion,
                  sim::Card::Crispin, sim::Card::MysteriousTreasure};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // DDE + the Basic attached by Crispin already pays Apex on T2. Vessel must be
  // allowed to spend the held Dragon as the strict-JIT payload now instead of the
  // legacy #1447 hold delaying the ready turn to T3.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Original hold: https://github.com/FlareZ123/pokemon-sims/issues/1447
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
  require(sim::EngineTestAccess::run_search_item(fixture.engine, true),
          "Post-Crispin DDE-ready turn did not play a search Item.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(discard_has(after, sim::Card::EarthenVessel),
          "Legacy #1447 hold still delayed Earthen Vessel after DDE completion.");
  require(discard_has(after, sim::Card::MegaDragonite),
          "T2 DDE-ready Vessel did not establish the strict-JIT payload.");
}

void test_direct_vessel_precedes_tapu_crispin() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = dde_regi(sim::Card::RegidragoVstar);
  state.hand = {sim::Card::QuickBall, sim::Card::EarthenVessel,
                sim::Card::FieldBlower};
  state.deck = {sim::Card::Grass, sim::Card::TapuLeleGX,
                sim::Card::Crispin};
  state.discard = {sim::Card::MegaDragonite};
  state.prizes = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                  sim::Card::Gladion, sim::Card::MysteriousTreasure,
                  sim::Card::Fire, sim::Card::ProfessorBurnet};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Direct Vessel + the unused manual attachment completes DDE-backed Apex, so the
  // planner should preserve Quick Ball's discard, the Bench slot, Tapu Lele-GX,
  // and the Supporter action instead of buying the longer Wonder Tag -> Crispin line.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original ordering: https://github.com/FlareZ123/pokemon-sims/issues/962
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
  require(sim::EngineTestAccess::run_search_item(fixture.engine, false),
          "Direct DDE Vessel route did not play an Item.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(discard_has(after, sim::Card::EarthenVessel),
          "Tapu/Crispin connector still preempted direct Vessel.");
  require(hand_has(after, sim::Card::QuickBall),
          "Direct Vessel route failed to preserve Quick Ball.");
}

void test_no_dde_controls() {
  Fixture fixture;
  const sim::Pokemon gf{sim::Card::RegidragoVstar, 1, 1, 1,
                        sim::Tool::None, 0};
  const sim::Pokemon gg{sim::Card::RegidragoVstar, 1, 2, 0,
                        sim::Tool::None, 0};

  // Preserve the original typed one-card completions when DDE is absent.
  // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed DDE regression: https://github.com/FlareZ123/pokemon-sims/issues/2437
  require(sim::EngineTestAccess::completing_basic(
              fixture.engine, gf, true, false) == sim::Card::Grass,
          "Legacy GF plus Grass completion regressed.");
  require(sim::EngineTestAccess::completing_basic(
              fixture.engine, gg, false, true) == sim::Card::Fire,
          "Legacy GG plus Fire completion regressed.");
}

}  // namespace

int main() {
  test_shared_projection_accepts_either_basic();
  test_burnet_finish_with_dde_basic(sim::Card::Grass);
  test_burnet_finish_with_dde_basic(sim::Card::Fire);
  test_active_vstar_payload_with_dde_basic(sim::Card::Grass);
  test_active_vstar_payload_with_dde_basic(sim::Card::Fire);
  test_latias_payload_with_dde_basic(sim::Card::Grass);
  test_latias_payload_with_dde_basic(sim::Card::Fire);
  test_post_crispin_ready_turn_does_not_hold_vessel(1);
  test_post_crispin_ready_turn_does_not_hold_vessel(2);
  test_direct_vessel_precedes_tapu_crispin();
  test_no_dde_controls();
  return 0;
}
