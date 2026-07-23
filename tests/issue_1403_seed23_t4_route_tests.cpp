#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static bool fss_grass_route(const Engine& engine) {
    return engine.fss_should_take_grass_for_next_turn_latias_burnet_route();
  }
  static Card fss_target(const Engine& engine) {
    return engine.fss_target_after_search_started();
  }
  static std::optional<Card> quick_ball_tate_cost(const Engine& engine) {
    return engine.quick_ball_latias_replaces_tate_cost();
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static const State& state(const Engine& engine) {
    return engine.state_;
  }
  static void prepare_seed_23_before_fss(Engine& engine) {
    engine.setup();
    for (const int turn : {1, 2}) {
      engine.begin_turn(turn);
      engine.run_turn();
      engine.record_ready();
      engine.resolve_powerglass_end_turn();
    }
    engine.begin_turn(3);
    engine.bench_latias_for_pending_vstar_promotion();
    engine.play_basics_from_hand();
    engine.attach_opening_oricorio_for_known_t4_fss_route();
    engine.play_known_steven_t3_payload_outlet();
    engine.play_items_until_stable(!engine.strict_payload_timing());
    engine.attach_live_fss();
  }
  static std::string fss_route_diagnostic(const Engine& engine) {
    const Pokemon* regi = engine.target_regi();
    std::ostringstream out;
    out << engine.state_line()
        << " deck_seen=" << engine.deck_seen_
        << " dci=" << static_cast<int>(engine.scenario_.dci)
        << " locks=" << static_cast<int>(engine.scenario_.locks)
        << " max_turn=" << engine.scenario_.max_turn
        << " ended=" << engine.state_.turn_ended
        << " vstar_used=" << engine.state_.vstar_power_used
        << " supporter_allowed=" << engine.supporter_allowed()
        << " manual_used=" << engine.state_.manual_energy_used
        << " retreat_used=" << engine.state_.retreat_used
        << " has_vstar=" << engine.has_vstar()
        << " need_energy=" << engine.need_energy()
        << " need_payload=" << engine.need_payload()
        << " grass_needed=" << engine.grass_needed()
        << " fire_needed=" << engine.fire_needed()
        << " bench_space=" << engine.bench_space()
        << " regi=" << (regi ? static_cast<int>(regi->card) : -1)
        << " regi_entered=" << (regi ? regi->entered_turn : -1)
        << " regi_grass=" << (regi ? regi->grass : -1)
        << " regi_fire=" << (regi ? regi->fire : -1)
        << " latias_ability=" << engine.ability_available_for_pokemon(Card::LatiasEx)
        << " h_vstar=" << engine.hand_count(Card::RegidragoVstar)
        << " h_crispin=" << engine.hand_count(Card::Crispin)
        << " h_qb=" << engine.hand_count(Card::QuickBall)
        << " h_tate=" << engine.hand_count(Card::TateLiza)
        << " h_burnet=" << engine.hand_count(Card::ProfessorBurnet)
        << " h_grass=" << engine.hand_count(Card::Grass)
        << " h_fire=" << engine.hand_count(Card::Fire)
        << " d_grass=" << engine.deck_count_after_search_started(Card::Grass)
        << " d_fire=" << engine.deck_count_after_search_started(Card::Fire)
        << " d_latias=" << engine.deck_count_after_search_started(Card::LatiasEx)
        << " payload=" << std::any_of(engine.state_.deck.begin(), engine.state_.deck.end(), is_payload);
    return out.str();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void remove_one_or_fail(std::vector<sim::Card>& zone, const sim::Card card) {
  const auto it = std::find(zone.begin(), zone.end(), card);
  if (it == zone.end()) throw std::runtime_error("Test fixture card was absent.");
  zone.erase(it);
}

const sim::Scenario& strict_scenario(
    const sim::LockMode locks = sim::LockMode::None, const int max_turn = 5) {
  // Engine retains a reference to its Scenario, so regression fixtures must use
  // stable-lifetime objects rather than temporaries:
  // C++ temporary lifetime: https://eel.is/c++draft/class.temporary#6.10
  // Repository sanitizer precedent: https://github.com/FlareZ123/pokemon-sims/issues/907
  static const sim::Scenario unlocked{
      "issue-1403", sim::DciProfile::StrictJit, sim::LockMode::None, true, 5};
  static const sim::Scenario short_horizon{
      "issue-1403-short", sim::DciProfile::StrictJit, sim::LockMode::None, true, 3};
  static const sim::Scenario item_locked{
      "issue-1403-item-lock", sim::DciProfile::StrictJit,
      sim::LockMode::FullItem, true, 5};
  static const sim::Scenario rulebox_locked{
      "issue-1403-rulebox-lock", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, true, 5};

  if (locks == sim::LockMode::None && max_turn == 5) return unlocked;
  if (locks == sim::LockMode::None && max_turn == 3) return short_horizon;
  if (locks == sim::LockMode::FullItem && max_turn == 5) return item_locked;
  if (locks == sim::LockMode::FullRuleBoxAbility && max_turn == 5) {
    return rulebox_locked;
  }
  throw std::runtime_error("Unsupported issue-1403 test Scenario.");
}

sim::State fss_route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 2},
      sim::Pokemon{sim::Card::RegidragoV, 3, 0, 0,
                   sim::Tool::ForestSealStone},
  };
  state.hand = {
      sim::Card::TateLiza,
      sim::Card::QuickBall,
      sim::Card::RegidragoVstar,
      sim::Card::Crispin,
      sim::Card::ProfessorBurnet,
      sim::Card::Arven,
  };
  state.deck = {
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::LatiasEx,
      sim::Card::MegaDragonite,
      sim::Card::Dragapult,
  };
  return state;
}

sim::State quick_ball_route_state() {
  sim::State state;
  state.turn = 3;
  state.supporter_used = true;
  state.manual_energy_used = true;
  state.vstar_power_used = true;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 2},
      sim::Pokemon{sim::Card::RegidragoV, 3, 2, 0,
                   sim::Tool::ForestSealStone},
  };
  state.hand = {
      sim::Card::QuickBall,
      sim::Card::TateLiza,
      sim::Card::RegidragoVstar,
      sim::Card::Fire,
      sim::Card::ProfessorBurnet,
  };
  state.deck = {
      sim::Card::LatiasEx,
      sim::Card::MegaDragonite,
      sim::Card::Dragapult,
  };
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_seed_23_exact_pre_fss_state() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-first scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{23};
  sim::Engine engine(*scenario, recipe, rng);
  sim::EngineTestAccess::prepare_seed_23_before_fss(engine);
  if (!sim::EngineTestAccess::fss_grass_route(engine)) {
    std::cerr << sim::EngineTestAccess::fss_route_diagnostic(engine) << '\n';
  }
  expect(sim::EngineTestAccess::fss_grass_route(engine),
         "The exact seed-23 pre-FSS state must admit direct Grass.");
}

void test_fss_direct_grass_route_and_controls() {
  std::mt19937_64 rng{140300};
  sim::Engine engine = make_engine(strict_scenario(), rng, fss_route_state());

  // Star Alchemy must take the direct Grass only when the complete K1 two-turn
  // Crispin, Quick Ball, Latias ex, Fire, evolution, and Burnet route is legal:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1403
  expect(sim::EngineTestAccess::fss_grass_route(engine),
         "The exact K1 route must admit direct Grass.");
  expect(sim::EngineTestAccess::fss_target(engine) == sim::Card::Grass,
         "Star Alchemy must search Grass instead of a duplicate Crispin.");

  const auto blocked_state = [&](sim::State state, const std::uint64_t seed,
                                 const char* message) {
    std::mt19937_64 local_rng{seed};
    sim::Engine blocked = make_engine(strict_scenario(), local_rng, std::move(state));
    expect(!sim::EngineTestAccess::fss_grass_route(blocked), message);
  };

  sim::State one_grass = fss_route_state();
  remove_one_or_fail(one_grass.deck, sim::Card::Grass);
  blocked_state(one_grass, 140301, "Two deck-resident Grass copies are required.");

  sim::State no_fire = fss_route_state();
  remove_one_or_fail(no_fire.deck, sim::Card::Fire);
  blocked_state(no_fire, 140302, "Crispin must retain a Fire target.");

  sim::State no_latias = fss_route_state();
  remove_one_or_fail(no_latias.deck, sim::Card::LatiasEx);
  blocked_state(no_latias, 140303, "The route must retain Latias ex in deck.");

  sim::State no_payload = fss_route_state();
  no_payload.deck.erase(std::remove_if(no_payload.deck.begin(), no_payload.deck.end(),
                                       sim::is_payload),
                        no_payload.deck.end());
  blocked_state(no_payload, 140304, "The route must retain a Burnet payload.");

  for (const sim::Card required : {sim::Card::QuickBall, sim::Card::TateLiza,
                                   sim::Card::ProfessorBurnet,
                                   sim::Card::RegidragoVstar}) {
    sim::State missing = fss_route_state();
    remove_one_or_fail(missing.hand, required);
    blocked_state(missing, 140305 + static_cast<std::uint64_t>(required),
                  "Every held route component must remain present.");
  }

  sim::State full_bench = fss_route_state();
  while (full_bench.bench.size() < 5U) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  }
  blocked_state(full_bench, 140310, "Latias requires an open Bench slot.");

  sim::State retreat_used = fss_route_state();
  retreat_used.retreat_used = true;
  blocked_state(retreat_used, 140311, "The route must preserve the Retreat action.");

  sim::State manual_used = fss_route_state();
  manual_used.manual_energy_used = true;
  blocked_state(manual_used, 140312,
                "The current-turn manual Grass attachment must remain unused.");

  sim::State supporter_used = fss_route_state();
  supporter_used.supporter_used = true;
  blocked_state(supporter_used, 140313,
                "Crispin requires the current turn's Supporter action.");

  sim::State wrong_evolution_timing = fss_route_state();
  wrong_evolution_timing.bench[1].entered_turn = 2;
  blocked_state(wrong_evolution_timing, 140314,
                "The exact delayed route must remain current-turn Regidrago only.");

  std::mt19937_64 horizon_rng{140315};
  sim::Engine no_horizon = make_engine(strict_scenario(sim::LockMode::None, 3),
                                       horizon_rng, fss_route_state());
  expect(!sim::EngineTestAccess::fss_grass_route(no_horizon),
         "The T4 route requires another modeled turn.");

  for (const sim::LockMode lock : {sim::LockMode::FullItem,
                                   sim::LockMode::FullRuleBoxAbility}) {
    std::mt19937_64 lock_rng{140316 + static_cast<std::uint64_t>(lock)};
    sim::Engine locked = make_engine(strict_scenario(lock), lock_rng,
                                     fss_route_state());
    expect(!sim::EngineTestAccess::fss_grass_route(locked),
           "Item or Rule Box Ability lock must reject the route.");
  }
}

void test_quick_ball_tate_replacement_and_controls() {
  std::mt19937_64 rng{140320};
  sim::Engine engine = make_engine(strict_scenario(), rng,
                                   quick_ball_route_state());

  // Latias ex replaces Tate & Liza's sole remaining switch role, making Tate a
  // route-conditioned Quick Ball cost while Fire, VSTAR, and Burnet stay protected:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Official discard-cost, Bench, Ability, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1403
  expect(sim::EngineTestAccess::quick_ball_tate_cost(engine) ==
             sim::Card::TateLiza,
         "The complete route must admit Tate & Liza as Quick Ball cost.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must execute the route-conditioned Latias search.");
  const sim::State& resolved = sim::EngineTestAccess::state(engine);
  expect(std::count(resolved.discard.begin(), resolved.discard.end(),
                    sim::Card::QuickBall) == 1,
         "Quick Ball must enter discard after play.");
  expect(std::count(resolved.discard.begin(), resolved.discard.end(),
                    sim::Card::TateLiza) == 1,
         "Tate & Liza must pay the Quick Ball cost.");
  expect(std::count(resolved.hand.begin(), resolved.hand.end(),
                    sim::Card::LatiasEx) == 1,
         "Quick Ball must search Latias ex.");
  expect(std::count(resolved.hand.begin(), resolved.hand.end(),
                    sim::Card::Fire) == 1,
         "The next-turn Fire attachment must remain protected.");
  expect(std::count(resolved.hand.begin(), resolved.hand.end(),
                    sim::Card::ProfessorBurnet) == 1,
         "The next-turn Burnet action must remain protected.");

  const auto blocked = [&](sim::State state, const std::uint64_t seed,
                           const char* message) {
    std::mt19937_64 local_rng{seed};
    sim::Engine candidate = make_engine(strict_scenario(), local_rng,
                                        std::move(state));
    expect(!sim::EngineTestAccess::quick_ball_tate_cost(candidate), message);
  };

  for (const sim::Card required : {sim::Card::TateLiza, sim::Card::Fire,
                                   sim::Card::RegidragoVstar,
                                   sim::Card::ProfessorBurnet}) {
    sim::State missing = quick_ball_route_state();
    remove_one_or_fail(missing.hand, required);
    blocked(missing, 140321 + static_cast<std::uint64_t>(required),
            "Every protected continuation card must remain present.");
  }

  sim::State no_latias = quick_ball_route_state();
  remove_one_or_fail(no_latias.deck, sim::Card::LatiasEx);
  blocked(no_latias, 140326, "Quick Ball must retain a Latias target.");

  sim::State no_payload = quick_ball_route_state();
  no_payload.deck.erase(std::remove_if(no_payload.deck.begin(), no_payload.deck.end(),
                                       sim::is_payload),
                        no_payload.deck.end());
  blocked(no_payload, 140327, "Burnet must retain a deck payload.");

  sim::State missing_grass = quick_ball_route_state();
  missing_grass.bench[1].grass = 1;
  blocked(missing_grass, 140328,
          "Tate remains protected until the Regidrago has both Grass.");

  sim::State fire_already_attached = quick_ball_route_state();
  fire_already_attached.bench[1].fire = 1;
  blocked(fire_already_attached, 140329,
          "The exact delayed Fire continuation must remain unresolved.");

  sim::State full_bench = quick_ball_route_state();
  while (full_bench.bench.size() < 5U) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  }
  blocked(full_bench, 140330, "Latias requires an open Bench slot.");

  sim::State retreat_used = quick_ball_route_state();
  retreat_used.retreat_used = true;
  blocked(retreat_used, 140331, "The Latias route requires the unused Retreat.");

  sim::State supporter_unused = quick_ball_route_state();
  supporter_unused.supporter_used = false;
  blocked(supporter_unused, 140332,
          "The replacement cost is limited to the post-Crispin state.");

  sim::State manual_unused = quick_ball_route_state();
  manual_unused.manual_energy_used = false;
  blocked(manual_unused, 140333,
          "The replacement cost is limited to the post-attachment state.");

  for (const sim::LockMode lock : {sim::LockMode::FullItem,
                                   sim::LockMode::FullRuleBoxAbility}) {
    std::mt19937_64 lock_rng{140334 + static_cast<std::uint64_t>(lock)};
    sim::Engine locked = make_engine(strict_scenario(lock), lock_rng,
                                     quick_ball_route_state());
    expect(!sim::EngineTestAccess::quick_ball_tate_cost(locked),
           "Item or Rule Box Ability lock must preserve Tate & Liza.");
  }
}
}  // namespace

int main() {
  try {
    test_seed_23_exact_pre_fss_state();
    test_fss_direct_grass_route_and_controls();
    test_quick_ball_tate_replacement_and_controls();
    std::cout << "Issue 1403 T4 Latias-Burnet route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
