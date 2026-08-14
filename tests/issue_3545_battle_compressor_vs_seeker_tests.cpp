#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static std::vector<Card> compressor_targets(const Engine& engine) {
    return engine.issue_3545_battle_compressor_targets();
  }
  static std::optional<Card> compressor_supporter(const Engine& engine) {
    return engine.issue_3545_compressor_supporter_target();
  }
  static std::optional<Card> vs_target(const Engine& engine) {
    return engine.issue_3545_vs_seeker_target();
  }
  static bool play_compressor(Engine& engine) {
    return engine.play_battle_compressor();
  }
  static bool play_vs(Engine& engine) { return engine.play_vs_seeker(); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool deck_seen(const Engine& engine) { return engine.deck_seen_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State ready_attacker_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::BattleCompressor};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::GoodraVstar, sim::Card::Grass,
                sim::Card::Fire};
  return state;
}

sim::Engine make_engine(const sim::DciProfile dci, const sim::LockMode locks,
                        std::mt19937_64& rng, const int max_turn = 4) {
  sim::Scenario scenario{"issue-3545", dci, locks, false, max_turn};
  return sim::Engine{scenario, sim::baseline_recipe(), rng};
}

void test_registered_metadata() {
  // Exact intrinsic metadata belongs to CardDefinition, while route policy remains
  // in Engine. Both cards are Items according to their printed Trainer subtype.
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
  // Card architecture: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  expect(sim::cards::BattleCompressor::definition.canonical_id == "xy4-92",
         "BC canonical ID drifted.");
  expect(sim::cards::VsSeeker::definition.canonical_id == "xy4-109",
         "VS Seeker canonical ID drifted.");
  expect(sim::is_item(sim::Card::BattleCompressor), "BC is not classified as an Item.");
  expect(sim::is_item(sim::Card::VsSeeker), "VS Seeker is not classified as an Item.");
}

void test_jit_profile_payload_counts() {
  // The printed effect permits up to three deck cards. Strict and matchup-flex JIT
  // require the Dragon to enter discard on the ready turn and preserve unused Dragon
  // options; NoDiscardControl explicitly permits broader early compression.
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  for (const sim::DciProfile profile : {sim::DciProfile::StrictJit,
                                        sim::DciProfile::MatchupFlexJit}) {
    std::mt19937_64 rng{3545};
    sim::Engine engine = make_engine(profile, sim::LockMode::None, rng);
    sim::EngineTestAccess::set_state(engine, ready_attacker_state());
    const auto targets = sim::EngineTestAccess::compressor_targets(engine);
    expect(targets.size() == 1, "JIT BC selected more than one Dragon payload.");
    expect(targets.front() == sim::Card::MegaDragonite,
           "JIT BC did not preserve established Dragon priority.");
  }

  std::mt19937_64 rng{3546};
  sim::Engine engine = make_engine(sim::DciProfile::NoDiscardControl,
                                   sim::LockMode::None, rng);
  sim::EngineTestAccess::set_state(engine, ready_attacker_state());
  const auto targets = sim::EngineTestAccess::compressor_targets(engine);
  expect(targets.size() == 3, "NoDiscardControl did not use legal BC payload capacity.");
  expect(targets[0] == sim::Card::MegaDragonite &&
         targets[1] == sim::Card::Dragapult &&
         targets[2] == sim::Card::GoodraVstar,
         "NoDiscardControl BC payload order drifted.");
}

void test_exact_resolution_and_up_to_three() {
  // A legal BC deck search establishes K1, moves only selected deck cards to discard,
  // records current-turn Dragon provenance, and may legally stop after one selection.
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K1/JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  std::mt19937_64 rng{3547};
  sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                   sim::LockMode::None, rng);
  sim::State state = ready_attacker_state();
  sim::EngineTestAccess::set_state(engine, std::move(state), false);
  expect(sim::EngineTestAccess::play_compressor(engine), "BC did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(sim::EngineTestAccess::deck_seen(engine), "BC search did not establish K1.");
  expect(std::count(after.discard.begin(), after.discard.end(), sim::Card::BattleCompressor) == 1,
         "Played BC did not enter discard.");
  expect(std::count(after.discard.begin(), after.discard.end(), sim::Card::MegaDragonite) == 1,
         "Selected Dragon did not enter discard.");
  expect(after.discarded_this_turn == std::vector<sim::Card>{sim::Card::MegaDragonite},
         "BC JIT provenance was not exact.");
  expect(std::count(after.deck.begin(), after.deck.end(), sim::Card::Dragapult) == 1 &&
         std::count(after.deck.begin(), after.deck.end(), sim::Card::GoodraVstar) == 1,
         "Strict BC discarded extra legal-but-unneeded cards instead of using up to three.");
}

void test_existing_payload_and_item_lock_hold_compressor() {
  // Strict JIT does not need a second current-turn Dragon after payload is already
  // live. Item lock independently prohibits BC and VS Seeker.
  // Battle Compressor / VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-92 https://api.pokemontcg.io/v2/cards/xy4-109
  // Persistent lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  {
    std::mt19937_64 rng{1};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, rng);
    sim::State state = ready_attacker_state();
    state.discard.push_back(sim::Card::MegaDragonite);
    state.discarded_this_turn.push_back(sim::Card::MegaDragonite);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::compressor_targets(engine).empty(),
           "Strict BC redundantly discarded after current-turn payload was live.");
  }
  {
    std::mt19937_64 rng{2};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::FullItem, rng);
    sim::State state = ready_attacker_state();
    state.hand.push_back(sim::Card::VsSeeker);
    state.discard.push_back(sim::Card::ProfessorBurnet);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::compressor_targets(engine).empty(),
           "BC ignored full Item lock.");
    expect(!sim::EngineTestAccess::vs_target(engine).has_value(),
           "VS Seeker ignored full Item lock.");
  }
}

void test_payload_connector_domination() {
  // If held Mysterious Treasure can spend a held Dragon while searching the missing
  // Regidrago VSTAR, that one-discard connector advances two axes and preserves BC.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // Connector domination: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  std::mt19937_64 rng{3548};
  sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                   sim::LockMode::None, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1};
  state.hand = {sim::Card::BattleCompressor, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(sim::EngineTestAccess::compressor_targets(engine).empty(),
         "BC preempted the two-axis held-payload Treasure connector.");
}

void test_vs_seeker_immediate_recovery() {
  // VS Seeker should recover Professor Burnet when the existing shared Supporter
  // policy can use Burnet to establish the only missing Dragon payload this turn.
  // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  std::mt19937_64 rng{3549};
  sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                   sim::LockMode::None, rng);
  sim::State state = ready_attacker_state();
  state.hand = {sim::Card::VsSeeker};
  state.discard = {sim::Card::ProfessorBurnet};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(sim::EngineTestAccess::vs_target(engine) == sim::Card::ProfessorBurnet,
         "VS Seeker did not identify the live Burnet payload route.");
  expect(sim::EngineTestAccess::play_vs(engine), "VS Seeker did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(std::count(after.hand.begin(), after.hand.end(), sim::Card::ProfessorBurnet) == 1,
         "VS Seeker did not recover Burnet to hand.");
  expect(std::count(after.discard.begin(), after.discard.end(), sim::Card::VsSeeker) == 1,
         "VS Seeker did not enter discard after use.");
}

void test_bc_vs_rejects_redundant_burnet_and_accepts_crispin_synergy() {
  // BC itself solves a lone payload axis, so BC->Burnet->VS is strictly redundant.
  // When Energy is also missing, BC can discard the Dragon and Crispin together;
  // VS Seeker then converts Crispin into the Energy-axis Supporter on the same turn.
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  {
    std::mt19937_64 rng{3550};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, rng);
    sim::State state = ready_attacker_state();
    state.hand = {sim::Card::BattleCompressor, sim::Card::VsSeeker};
    state.deck = {sim::Card::ProfessorBurnet, sim::Card::MegaDragonite,
                  sim::Card::Grass, sim::Card::Fire};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::compressor_supporter(engine).has_value(),
           "BC selected redundant Burnet instead of the cheaper direct payload line.");
    const auto targets = sim::EngineTestAccess::compressor_targets(engine);
    expect(targets == std::vector<sim::Card>{sim::Card::MegaDragonite},
           "Redundant Burnet route changed the direct strict-JIT BC target.");
  }
  {
    std::mt19937_64 rng{3551};
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, rng);
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1};
    state.hand = {sim::Card::BattleCompressor, sim::Card::VsSeeker};
    state.deck = {sim::Card::Crispin, sim::Card::MegaDragonite,
                  sim::Card::Grass, sim::Card::Fire, sim::Card::Dragapult};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::compressor_supporter(engine) == sim::Card::Crispin,
           "BC+VS did not recognize Crispin Energy/payload synergy.");
    const auto targets = sim::EngineTestAccess::compressor_targets(engine);
    expect(targets.size() == 2 && targets[0] == sim::Card::Crispin &&
           targets[1] == sim::Card::MegaDragonite,
           "BC+VS synergy did not select exactly Crispin plus one strict-JIT Dragon.");
  }
}

void test_vs_future_bank_before_persistent_item_lock() {
  // After the current Supporter has been used, VS Seeker may still legally recover a
  // future Supporter. Spending it now is strategically justified when the modeled
  // persistent T2 Item lock would make VS unusable next turn and public state proves
  // Crispin advances the next-turn Energy axis without relying on the unknown draw.
  // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Persistent lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Future-card policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  std::mt19937_64 rng{3552};
  sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                   sim::LockMode::TurnTwoItem, rng);
  sim::State state;
  state.turn = 1;
  state.supporter_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 0, 1, 1};
  state.hand = {sim::Card::VsSeeker};
  state.discard = {sim::Card::Crispin};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(sim::EngineTestAccess::vs_target(engine) == sim::Card::Crispin,
         "VS Seeker failed to bank Crispin before scheduled persistent Item lock.");
}
}  // namespace

int main() {
  try {
    test_registered_metadata();
    test_jit_profile_payload_counts();
    test_exact_resolution_and_up_to_three();
    test_existing_payload_and_item_lock_hold_compressor();
    test_payload_connector_domination();
    test_vs_seeker_immediate_recovery();
    test_bc_vs_rejects_redundant_burnet_and_accepts_crispin_synergy();
    test_vs_future_bank_before_persistent_item_lock();
    std::cout << "Issue 3545 BC / VS Seeker tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
