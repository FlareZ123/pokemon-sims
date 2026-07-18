#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static State& state(Engine& engine) { return engine.state_; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool deck_seen(const Engine& engine) { return engine.deck_seen_; }
  static bool resolve_pokemon_communication(Engine& engine, const Card returned,
                                            const Card target) {
    return engine.resolve_pokemon_communication(returned, target);
  }
  static bool play_pokemon_communication(Engine& engine, const bool permit_payload = false) {
    return engine.play_pokemon_communication(permit_payload);
  }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool play_quick_ball(Engine& engine, const bool permit_payload) {
    return engine.play_quick_ball(permit_payload);
  }
  static bool communication_plan_after_supporter(const Engine& engine,
                                                  const Card supporter) {
    return engine.pokemon_communication_plan_after_playing_supporter(
        supporter, false).has_value();
  }
  static bool communication_plan_after_wonder_tag_arven(const Engine& engine) {
    return engine.pokemon_communication_plan_after_wonder_tag_arven().has_value();
  }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool play_team_yell_vstar_recovery(Engine& engine) {
    return engine.play_team_yell_vstar_recovery();
  }
  static bool play_team_yell_latias_recovery(Engine& engine) {
    return engine.play_team_yell_latias_recovery();
  }
  static bool play_roseanne_vstar_recovery(Engine& engine) {
    return engine.play_roseanne_vstar_recovery();
  }
  static bool play_roseanne_vstar_energy_recovery(Engine& engine) {
    return engine.play_roseanne_vstar_energy_recovery();
  }
  static bool play_earthen_vessel(Engine& engine, const bool permit_payload) {
    return engine.play_earthen_vessel(permit_payload);
  }
  static std::vector<Card> lusamine_recovery_targets(const Engine& engine) {
    return engine.lusamine_recovery_targets();
  }
  static bool needs_tapu_connector(Engine& engine) {
    return engine.needs_tapu_connector();
  }
  static void choose_opening_active(Engine& engine) { engine.choose_opening_active(); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool in_play(const sim::State& state, const sim::Card card) {
  return (state.active && state.active->card == card) ||
      std::any_of(state.bench.begin(), state.bench.end(), [card](const sim::Pokemon& pokemon) {
        return pokemon.card == card;
      });
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(std::string label, const sim::DciProfile dci = sim::DciProfile::StrictJit,
          const sim::LockMode locks = sim::LockMode::None,
          const bool going_first = false, const std::uint64_t seed = 701)
      : scenario{std::move(label), dci, locks, going_first, 4},
        rng(seed), engine(scenario, recipe, rng) {}
};

void test_baseline_recipe_is_unchanged() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  int total = 0;
  int communication = 0;
  for (const auto& [card, copies] : recipe) {
    total += copies;
    if (card == sim::Card::PokemonCommunication) communication += copies;
  }
  if (total != 60 || communication != 0) {
    throw std::runtime_error("Pokémon Communication support must not change the baseline 60-card recipe.");
  }
  if (sim::name(sim::Card::PokemonCommunication) != "Pokémon Communication") {
    throw std::runtime_error("Pokémon Communication must have a stable display identity.");
  }
}

void test_variant_recipe_supports_communication_without_changing_baseline() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  auto mawile = std::find_if(recipe.begin(), recipe.end(), [](const auto& entry) {
    return entry.first == sim::Card::MawileGX;
  });
  if (mawile == recipe.end() || mawile->second != 1) {
    throw std::runtime_error("The variant fixture could not identify its one-card swap.");
  }
  recipe.erase(mawile);
  recipe.emplace_back(sim::Card::PokemonCommunication, 1);

  int total = 0;
  for (const auto& [card, copies] : recipe) {
    (void)card;
    total += copies;
  }
  if (total != 60) {
    throw std::runtime_error("The Pokémon Communication variant recipe must remain 60 cards.");
  }

  // The card remains absent from baseline_recipe(), while an explicit DeckRecipe
  // swap must drive K0 fixed-list accounting and complete simulations correctly:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/701
  const sim::Scenario direct{"communication-variant-k0", sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 4};
  std::mt19937_64 direct_rng{70101};
  sim::Engine direct_engine(direct, recipe, direct_rng);
  sim::State direct_state;
  direct_state.turn = 1;
  direct_state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  direct_state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin};
  direct_state.deck = {sim::Card::RegidragoV, sim::Card::Grass};
  sim::EngineTestAccess::set_state(direct_engine, std::move(direct_state));
  if (!sim::EngineTestAccess::play_pokemon_communication(direct_engine) ||
      !contains(sim::EngineTestAccess::state(direct_engine).hand,
                sim::Card::RegidragoV)) {
    throw std::runtime_error("A custom recipe did not expose its K0 Communication route.");
  }

  constexpr std::array<sim::LockMode, 5> locks{{
      sim::LockMode::None, sim::LockMode::TurnTwoItem,
      sim::LockMode::FullItem, sim::LockMode::FullRuleBoxAbility,
      sim::LockMode::FullCombined,
  }};
  for (const bool going_first : {false, true}) {
    for (const sim::LockMode lock : locks) {
      const sim::Scenario scenario{"communication-variant-smoke",
                                   sim::DciProfile::StrictJit, lock,
                                   going_first, 4};
      for (std::uint64_t seed = 1; seed <= 10; ++seed) {
        std::mt19937_64 rng{seed};
        sim::Engine engine(scenario, recipe, rng);
        (void)engine.run();
      }
    }
  }
}

void test_same_returned_pokemon_can_be_searched_from_an_empty_deck() {
  Fixture fixture("communication-same-card-empty-deck");
  sim::State state;
  state.turn = 1;
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Pokémon Communication puts the revealed Pokémon into the deck before searching.
  // The same Dipplin is therefore a legal target even when the deck began empty:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://www.pokemon.com/us/pokemon-tcg/rules
  if (!sim::EngineTestAccess::resolve_pokemon_communication(
          fixture.engine, sim::Card::Dipplin, sim::Card::Dipplin)) {
    throw std::runtime_error("The returned Pokémon must be searchable by the same resolving Item.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.deck.empty() || count(after.hand, sim::Card::Dipplin) != 1 ||
      !contains(after.discard, sim::Card::PokemonCommunication) ||
      !sim::EngineTestAccess::deck_seen(fixture.engine)) {
    throw std::runtime_error("The empty-deck same-card exchange produced incorrect zones or knowledge.");
  }
}

void test_resolver_requires_a_pokemon_in_hand() {
  Fixture fixture("communication-no-pokemon");
  sim::State state;
  state.turn = 1;
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Arven};
  state.deck = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // The printed effect begins by revealing a Pokémon from hand. A Trainer card cannot
  // satisfy that condition: https://api.pokemontcg.io/v2/cards/sm9-152
  if (sim::EngineTestAccess::resolve_pokemon_communication(
          fixture.engine, sim::Card::Arven, sim::Card::RegidragoV) ||
      sim::EngineTestAccess::play_pokemon_communication(fixture.engine)) {
    throw std::runtime_error("Pokémon Communication must remain unplayed without a hand Pokémon.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand != state.hand || after.deck != state.deck || !after.discard.empty()) {
    throw std::runtime_error("An illegal exchange attempt must be zone-preserving.");
  }
}

void test_every_item_lock_blocks_communication() {
  constexpr std::array<std::pair<sim::LockMode, int>, 3> cases{{
      {sim::LockMode::FullItem, 1},
      {sim::LockMode::FullCombined, 1},
      {sim::LockMode::TurnTwoItem, 2},
  }};
  for (std::size_t index = 0; index < cases.size(); ++index) {
    const auto [locks, turn] = cases[index];
    Fixture fixture("communication-item-lock", sim::DciProfile::StrictJit,
                    locks, false, 710U + static_cast<unsigned>(index));
    sim::State state;
    state.turn = turn;
    state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin};
    state.deck = {sim::Card::RegidragoV};
    sim::EngineTestAccess::set_state(fixture.engine, state);
    sim::EngineTestAccess::set_deck_seen(fixture.engine);

    // Pokémon Communication is an Item and is unavailable in each modeled Item-lock
    // state: https://api.pokemontcg.io/v2/cards/sm9-152
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (sim::EngineTestAccess::play_pokemon_communication(fixture.engine) ||
        sim::EngineTestAccess::state(fixture.engine).hand != state.hand) {
      throw std::runtime_error("A modeled Item lock allowed Pokémon Communication.");
    }
  }
}

void test_turn_two_item_lock_allows_turn_one_communication() {
  Fixture fixture("communication-turn-one-before-lock", sim::DciProfile::StrictJit,
                  sim::LockMode::TurnTwoItem, false, 714);
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin};
  state.deck = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);
  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::RegidragoV)) {
    throw std::runtime_error("Turn-two Item lock must not suppress a legal turn-one exchange.");
  }
}

void test_direct_search_returns_dipplin_for_missing_regidrago() {
  Fixture fixture("communication-regidrago-route");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                sim::Card::LatiasEx};
  state.deck = {sim::Card::RegidragoV, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine)) {
    throw std::runtime_error("Pokémon Communication should find the missing Basic Regidrago V.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::RegidragoV) ||
      !contains(after.hand, sim::Card::LatiasEx) ||
      contains(after.hand, sim::Card::Dipplin) ||
      !contains(after.deck, sim::Card::Dipplin)) {
    throw std::runtime_error("The route-aware exchange should return Dipplin and preserve Latias ex.");
  }
}

void test_direct_search_protects_live_latias_while_finding_vstar() {
  Fixture fixture("communication-protect-latias");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::LatiasEx,
                sim::Card::Dipplin};
  state.deck = {sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine)) {
    throw std::runtime_error("Pokémon Communication should use redundant Dipplin to find VSTAR.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::LatiasEx) ||
      !contains(after.hand, sim::Card::RegidragoVstar) ||
      contains(after.hand, sim::Card::Dipplin)) {
    throw std::runtime_error("The exchange must preserve the live Latias ex connector.");
  }
}

void test_policy_holds_a_no_progress_known_deck_cycle() {
  Fixture fixture("communication-known-no-progress");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  if (sim::EngineTestAccess::play_pokemon_communication(fixture.engine)) {
    throw std::runtime_error("Policy must not spend Pokémon Communication on a same-card no-progress cycle.");
  }
  if (sim::EngineTestAccess::state(fixture.engine).hand != state.hand ||
      sim::EngineTestAccess::state(fixture.engine).deck != state.deck) {
    throw std::runtime_error("The held no-progress route must preserve every zone.");
  }
}

void test_communication_is_not_a_payload_discard_outlet() {
  Fixture fixture("communication-no-payload-outlet");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Pokémon Communication returns a Pokémon to the deck. It never puts a Dragon in
  // discard for Apex Dragon: https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (sim::EngineTestAccess::play_pokemon_communication(fixture.engine, true)) {
    throw std::runtime_error("Pokémon Communication alone must not be classified as a DCI payload outlet.");
  }
  if (!sim::EngineTestAccess::state(fixture.engine).discard.empty()) {
    throw std::runtime_error("A rejected payload route must not discard any card.");
  }
}

void test_communication_fetches_payload_for_a_separate_treasure_outlet() {
  Fixture fixture("communication-payload-continuation");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // The returned Dipplin is a Dragon target for the later Mysterious Treasure.
  // Treasure can therefore discard the fetched payload and still resolve its search:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine, true) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::MegaDragonite)) {
    throw std::runtime_error("Pokémon Communication should start the live Treasure payload continuation.");
  }
  if (!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, true)) {
    throw std::runtime_error("Mysterious Treasure should complete the fetched-payload route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The separate outlet must establish the current-turn payload discard.");
  }
}

void test_arven_can_fetch_a_live_communication_bridge() {
  Fixture fixture("arven-communication-vstar");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::Arven, sim::Card::Dipplin};
  state.deck = {sim::Card::PokemonCommunication, sim::Card::RegidragoVstar,
                sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Arven searches an Item. After Arven consumes the Supporter play, Pokémon
  // Communication can still exchange Dipplin for Regidrago VSTAR:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::play_arven(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::PokemonCommunication)) {
    throw std::runtime_error("Arven should fetch a live Pokémon Communication bridge.");
  }
  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Arven's searched Pokémon Communication should find VSTAR.");
  }
}

void test_arven_rejects_communication_without_an_exchange_pokemon() {
  Fixture fixture("arven-dead-communication");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::Arven};
  state.deck = {sim::Card::PokemonCommunication, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);
  if (sim::EngineTestAccess::play_arven(fixture.engine) ||
      sim::EngineTestAccess::state(fixture.engine).hand != state.hand) {
    throw std::runtime_error("Arven must not fetch Pokémon Communication without a hand Pokémon to exchange.");
  }
}

void test_forest_seal_stone_prefers_the_direct_pokemon_target() {
  Fixture fixture("fss-dominates-communication");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::Dipplin};
  state.deck = {sim::Card::PokemonCommunication, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Star Alchemy searches any card, so the direct VSTAR target strictly dominates
  // fetching an exchange Item first:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm9-152
  if (!sim::EngineTestAccess::use_fss(fixture.engine)) {
    throw std::runtime_error("Forest Seal Stone should use the direct live VSTAR route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) ||
      contains(after.hand, sim::Card::PokemonCommunication)) {
    throw std::runtime_error("Star Alchemy must take VSTAR directly instead of Pokémon Communication.");
  }
}

void test_gladion_preserves_itself_behind_a_held_communication_route() {
  Fixture fixture("gladion-held-communication");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::Gladion, sim::Card::PokemonCommunication,
                sim::Card::Dipplin};
  state.deck = {sim::Card::RegidragoVstar};
  state.prizes = {sim::Card::Grass, sim::Card::Fire, sim::Card::MawileGX};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  if (sim::EngineTestAccess::play_gladion(fixture.engine)) {
    throw std::runtime_error("Gladion should be preserved while held Pokémon Communication solves VSTAR.");
  }
  if (sim::EngineTestAccess::state(fixture.engine).hand != state.hand) {
    throw std::runtime_error("The preserved Gladion route must leave the hand unchanged.");
  }
}

void test_gladion_can_recover_a_known_prized_communication() {
  Fixture fixture("gladion-prized-communication");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::Gladion, sim::Card::Dipplin};
  state.deck = {sim::Card::RegidragoVstar};
  state.prizes = {sim::Card::PokemonCommunication, sim::Card::Grass,
                  sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Gladion may recover the known prized Item, and the Item remains playable after
  // the Supporter resolves: https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sm9-152
  if (!sim::EngineTestAccess::play_gladion(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::PokemonCommunication)) {
    throw std::runtime_error("Gladion should recover a known prized live Pokémon Communication.");
  }
  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::RegidragoVstar)) {
    throw std::runtime_error("The recovered Pokémon Communication should complete the VSTAR route.");
  }
}

void test_legacy_star_recovers_communication_and_exchange_pokemon() {
  Fixture fixture("legacy-star-communication-recovery");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.discard = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                   sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.deck = {sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Arven, sim::Card::Crispin,
                sim::Card::Guzma, sim::Card::Channeler};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Legacy Star returns up to two cards. Recovering Pokémon Communication and
  // Dipplin creates a legal exchange for Latias ex and the same-turn Skyliner route:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv8-76
  if (!sim::EngineTestAccess::use_legacy_star(fixture.engine)) {
    throw std::runtime_error("Legacy Star should recover the known Communication bridge.");
  }
  const sim::State& recovered = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(recovered.hand, sim::Card::PokemonCommunication) ||
      !contains(recovered.hand, sim::Card::Dipplin)) {
    throw std::runtime_error("Legacy Star should recover both exchange components.");
  }
  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::LatiasEx)) {
    throw std::runtime_error("The Legacy Star bridge should exchange for Latias ex.");
  }
}

void test_legacy_star_does_not_recover_communication_under_item_lock() {
  Fixture fixture("legacy-star-communication-lock", sim::DciProfile::StrictJit,
                  sim::LockMode::FullItem);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.discard = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                   sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.deck = {sim::Card::LatiasEx};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::use_legacy_star(fixture.engine) ||
      sim::EngineTestAccess::state(fixture.engine).discard != state.discard) {
    throw std::runtime_error("Item lock must suppress a Communication-only Legacy Star route.");
  }
}

sim::State final_slot_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None},
      sim::Pokemon{sim::Card::Oricorio, 1},
      sim::Pokemon{sim::Card::MawileGX, 1},
      sim::Pokemon{sim::Card::DialgaGX, 1},
  };
  state.hand = {sim::Card::LatiasEx, sim::Card::RegidragoV,
                sim::Card::PokemonCommunication, sim::Card::Dipplin};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::Fire};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  return state;
}

void test_team_yell_can_restore_vstar_for_held_communication() {
  Fixture fixture("team-yell-communication-vstar");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::TeamYellsCheer, sim::Card::PokemonCommunication,
                sim::Card::Dipplin};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Team Yell's Cheer shuffles the known-absent VSTAR into the deck. The Supporter
  // leaves hand first, after which Pokémon Communication may exchange Dipplin for
  // that restored Evolution Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::play_team_yell_vstar_recovery(fixture.engine) ||
      !sim::EngineTestAccess::play_pokemon_communication(fixture.engine)) {
    throw std::runtime_error("Team Yell's Cheer should create the exact held-Communication VSTAR route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) ||
      !contains(after.deck, sim::Card::Dipplin)) {
    throw std::runtime_error("The Team Yell recovery and exchange resolved to incorrect zones.");
  }
}

void test_team_yell_can_restore_latias_for_held_communication() {
  Fixture fixture("team-yell-communication-latias");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::TeamYellsCheer, sim::Card::PokemonCommunication,
                sim::Card::Dipplin};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::LatiasEx, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // The payload is already in discard, so Pokémon Communication only needs to find
  // the restored Latias ex for Skyliner. It cannot substitute for a missing payload
  // because its exchange never discards a Dragon:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::play_team_yell_latias_recovery(fixture.engine) ||
      !sim::EngineTestAccess::play_pokemon_communication(fixture.engine)) {
    throw std::runtime_error("Team Yell's Cheer should restore a Communication-searchable Latias ex.");
  }
  if (!contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::LatiasEx)) {
    throw std::runtime_error("The restored Latias ex was not searched into hand.");
  }
}

void test_team_yell_rejects_communication_when_payload_is_still_missing() {
  Fixture fixture("team-yell-communication-latias-payload-control");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::TeamYellsCheer, sim::Card::PokemonCommunication,
                sim::Card::Dipplin};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::LatiasEx};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);
  if (sim::EngineTestAccess::play_team_yell_latias_recovery(fixture.engine)) {
    throw std::runtime_error("Pokémon Communication must not stand in for a missing payload discard outlet.");
  }
  if (sim::EngineTestAccess::state(fixture.engine).hand != state.hand) {
    throw std::runtime_error("Rejected Team Yell routing must preserve hand state.");
  }
}

void test_roseanne_can_restore_vstar_for_held_communication() {
  Fixture fixture("roseanne-communication-vstar");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::RoseannesBackup, sim::Card::PokemonCommunication,
                sim::Card::Dipplin};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Roseanne's Backup may shuffle one Pokémon from discard into the deck. The held
  // exchange Item can then find the exact restored VSTAR:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::play_roseanne_vstar_recovery(fixture.engine) ||
      !sim::EngineTestAccess::play_pokemon_communication(fixture.engine)) {
    throw std::runtime_error("Roseanne's Backup should create a payable Communication VSTAR route.");
  }
  if (!contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::RegidragoVstar)) {
    throw std::runtime_error("The restored VSTAR was not found by Pokémon Communication.");
  }
}

void test_roseanne_multimode_supports_communication_plus_vessel() {
  Fixture fixture("roseanne-communication-vessel", sim::DciProfile::NoDiscardControl);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::RoseannesBackup, sim::Card::PokemonCommunication,
                sim::Card::Dipplin, sim::Card::EarthenVessel,
                sim::Card::Arven, sim::Card::Arven};
  state.deck = {sim::Card::ChaoticSwell};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::Grass,
                   sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Roseanne may choose Pokémon and Energy modes together. Pokémon Communication
  // searches the restored VSTAR, and Earthen Vessel retains Arven as its distinct
  // discard cost before searching the restored Grass Energy:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv4-163
  if (!sim::EngineTestAccess::play_roseanne_vstar_energy_recovery(fixture.engine) ||
      !sim::EngineTestAccess::play_pokemon_communication(fixture.engine) ||
      !sim::EngineTestAccess::play_earthen_vessel(fixture.engine, true)) {
    throw std::runtime_error("Roseanne's combined mode should support the Communication plus Vessel sequence.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) ||
      !contains(after.hand, sim::Card::Grass) ||
      !contains(after.discard, sim::Card::Arven)) {
    throw std::runtime_error("The combined Roseanne route produced incorrect search or cost zones.");
  }
}

void test_lusamine_recognizes_future_arven_communication_route() {
  Fixture fixture("lusamine-arven-communication");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::Lusamine, sim::Card::Dipplin};
  state.deck = {sim::Card::PokemonCommunication, sim::Card::RegidragoVstar};
  state.discard = {sim::Card::Arven, sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Lusamine is a future-turn recovery. It should recognize that recovered Arven
  // can later search Pokémon Communication, which can exchange Dipplin for VSTAR:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm9-152
  const auto targets = sim::EngineTestAccess::lusamine_recovery_targets(fixture.engine);
  if (!contains(targets, sim::Card::Arven)) {
    throw std::runtime_error("Lusamine should retain Arven for the live future Communication route.");
  }
}

void test_final_bench_slot_is_reserved_for_communication_vstar_route() {
  Fixture fixture("communication-final-slot", sim::DciProfile::NoDiscardControl,
                  sim::LockMode::None, true, 730);
  sim::EngineTestAccess::set_state(fixture.engine, final_slot_state());

  // The pre-Item pass must reserve the last Bench slot for held Latias ex when
  // Pokémon Communication can exchange Dipplin for Regidrago VSTAR. Skyliner then
  // promotes the powered evolution:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::run_turn(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoVstar ||
      !in_play(after, sim::Card::LatiasEx) ||
      !contains(after.hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("The final Bench slot was not preserved for the Communication VSTAR route.");
  }
}

void test_opening_selector_counts_only_an_unlocked_communication_route() {
  const auto opening = [] {
    sim::State state;
    state.hand = {sim::Card::TapuLeleGX, sim::Card::Oricorio,
                  sim::Card::PokemonCommunication, sim::Card::Crispin,
                  sim::Card::Grass, sim::Card::Arven, sim::Card::Dipplin};
    state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar};
    return state;
  };

  {
    Fixture fixture("opening-communication-unlocked", sim::DciProfile::StrictJit,
                    sim::LockMode::None, true, 731);
    sim::EngineTestAccess::set_state(fixture.engine, opening());
    sim::EngineTestAccess::choose_opening_active(fixture.engine);
    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
    if (!after.active || after.active->card != sim::Card::Oricorio ||
        !contains(after.hand, sim::Card::TapuLeleGX)) {
      throw std::runtime_error("An unlocked Communication route should preserve Wonder Tag in the dense opening.");
    }
  }

  {
    Fixture fixture("opening-communication-locked", sim::DciProfile::StrictJit,
                    sim::LockMode::FullItem, true, 732);
    sim::EngineTestAccess::set_state(fixture.engine, opening());
    sim::EngineTestAccess::choose_opening_active(fixture.engine);
    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
    if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
        !contains(after.hand, sim::Card::Oricorio)) {
      throw std::runtime_error("Full Item lock must remove Communication from opening-route valuation.");
    }
  }

  {
    Fixture fixture("opening-communication-no-safe-third",
                    sim::DciProfile::StrictJit, sim::LockMode::None, true, 733);
    sim::State state = opening();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                               sim::Card::Dipplin));
    state.hand.push_back(sim::Card::ChaoticSwell);
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    sim::EngineTestAccess::choose_opening_active(fixture.engine);
    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
    // With Tapu Lele-GX and Oricorio as the only exchange Pokémon, Communication
    // cannot make either connector redundant. Starting Tapu preserves Vital Dance:
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm2-55
    if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
        !contains(after.hand, sim::Card::Oricorio)) {
      throw std::runtime_error("Communication must not sacrifice the only remaining opening connector.");
    }
  }
}

void test_direct_communication_routes_preserve_wonder_tag() {
  {
    Fixture fixture("communication-suppresses-tapu-vstar");
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
    state.hand = {sim::Card::TapuLeleGX, sim::Card::PokemonCommunication,
                  sim::Card::Dipplin};
    state.deck = {sim::Card::RegidragoVstar};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    sim::EngineTestAccess::set_deck_seen(fixture.engine);
    if (sim::EngineTestAccess::needs_tapu_connector(fixture.engine)) {
      throw std::runtime_error("Held Communication should preserve Wonder Tag when it directly finds VSTAR.");
    }
  }

  {
    Fixture fixture("communication-suppresses-tapu-oricorio");
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                                sim::Tool::None};
    state.hand = {sim::Card::TapuLeleGX, sim::Card::PokemonCommunication,
                  sim::Card::Dipplin};
    state.deck = {sim::Card::Oricorio, sim::Card::Grass};
    state.discard = {sim::Card::MegaDragonite};
    state.discarded_this_turn = {sim::Card::MegaDragonite};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    sim::EngineTestAccess::set_deck_seen(fixture.engine);
    if (sim::EngineTestAccess::needs_tapu_connector(fixture.engine)) {
      throw std::runtime_error("Held Communication should preserve Wonder Tag when it directly finds Vital Dance.");
    }
  }

  {
    Fixture fixture("communication-suppresses-tapu-tate");
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::MawileGX, 1};
    state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                                sim::Tool::None}};
    state.hand = {sim::Card::TapuLeleGX, sim::Card::PokemonCommunication,
                  sim::Card::Dipplin};
    state.deck = {sim::Card::LatiasEx, sim::Card::TateLiza};
    state.discard = {sim::Card::MegaDragonite};
    state.discarded_this_turn = {sim::Card::MegaDragonite};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    sim::EngineTestAccess::set_deck_seen(fixture.engine);
    if (sim::EngineTestAccess::needs_tapu_connector(fixture.engine)) {
      throw std::runtime_error("Held Communication should find Latias without spending Wonder Tag on Tate & Liza.");
    }
  }
}

void test_k0_search_adapts_when_preferred_vstar_is_prized() {
  Fixture fixture("communication-k0-adaptive-target");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin};
  state.deck = {sim::Card::Oricorio, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // At K0, Regidrago VSTAR remains publicly plausible even though this fixture has
  // it in the unknown Prize zone. The legal search must begin before that absence
  // becomes known, then continue with the strongest Pokémon actually in the deck:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine)) {
    throw std::runtime_error("K0 Communication should resolve despite the preferred VSTAR being prized.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!sim::EngineTestAccess::deck_seen(fixture.engine) ||
      !contains(after.hand, sim::Card::Oricorio) ||
      contains(after.hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("The K1 resolution should adapt from prized VSTAR to the live Oricorio target.");
  }
}

void test_preferred_absence_can_resolve_to_the_returned_pokemon() {
  Fixture fixture("communication-preferred-absent-returned-target");
  sim::State state;
  state.turn = 2;
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The returned Pokémon enters the deck before the search, so it remains a legal
  // fallback when a K0-preferred Pokémon proves absent at K1:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  if (!sim::EngineTestAccess::resolve_pokemon_communication(
          fixture.engine, sim::Card::Dipplin, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Communication should complete through its guaranteed returned target.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::Dipplin) || !after.deck.empty()) {
    throw std::runtime_error("The absent preferred target should resolve back to Dipplin.");
  }
}

void test_arven_does_not_count_itself_as_ultra_ball_fodder() {
  Fixture fixture("arven-communication-exact-post-supporter-hand");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Arven, sim::Card::Dipplin, sim::Card::UltraBall};
  state.deck = {sim::Card::PokemonCommunication, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Arven leaves the hand before its searched Item resolves. After Communication
  // exchanges Dipplin for the payload, Ultra Ball has only one other card and cannot
  // pay its two-card requirement:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  if (sim::EngineTestAccess::communication_plan_after_supporter(
          fixture.engine, sim::Card::Arven) ||
      sim::EngineTestAccess::play_arven(fixture.engine) ||
      sim::EngineTestAccess::state(fixture.engine).hand != state.hand) {
    throw std::runtime_error("Arven must reject the Communication route after removing itself from the cost hand.");
  }
}

void test_wonder_tag_arven_plan_consumes_tapu_and_bench_space() {
  Fixture fixture("wonder-tag-arven-communication-exact-state");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Dipplin,
                sim::Card::UltraBall};
  state.deck = {sim::Card::Arven, sim::Card::PokemonCommunication,
                sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Wonder Tag requires Tapu Lele-GX to be played to the Bench before Arven and the
  // searched Item resolve. Tapu cannot then be exchanged, and Arven cannot pay a
  // later Ultra Ball cost after leaving the hand:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm9-152
  if (sim::EngineTestAccess::communication_plan_after_wonder_tag_arven(
          fixture.engine)) {
    throw std::runtime_error("Wonder Tag planning must use the exact post-Bench, post-Arven state.");
  }
}

void test_gladion_does_not_recover_communication_using_itself_as_cost() {
  Fixture fixture("gladion-communication-exact-post-supporter-hand");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Dipplin, sim::Card::UltraBall};
  state.deck = {sim::Card::MegaDragonite};
  state.prizes = {sim::Card::PokemonCommunication, sim::Card::Grass,
                  sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Gladion is exchanged into the Prize cards before the recovered Item resolves.
  // It cannot remain in hand as Ultra Ball's second cost card:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  if (sim::EngineTestAccess::communication_plan_after_supporter(
          fixture.engine, sim::Card::Gladion)) {
    throw std::runtime_error("The exact post-Gladion hand must reject the unpayable payload route.");
  }
  sim::EngineTestAccess::play_gladion(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (contains(after.hand, sim::Card::PokemonCommunication) ||
      !contains(after.prizes, sim::Card::PokemonCommunication)) {
    throw std::runtime_error("Gladion must leave the dead prized Communication route untouched.");
  }
}

void test_payload_route_rejects_fetched_only_mysterious_treasure_target() {
  Fixture fixture("communication-treasure-fetched-only-target");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::MawileGX,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Mega Dragonite ex leaves the known deck during Pokémon Communication. Mawile-GX
  // then becomes the only deck card, so Mysterious Treasure would have no Psychic or
  // Dragon target after discarding the fetched payload. Reject the one-shot bridge
  // before spending it:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // https://github.com/FlareZ123/pokemon-sims/issues/808
  if (sim::EngineTestAccess::play_pokemon_communication(fixture.engine, true) ||
      sim::EngineTestAccess::state(fixture.engine).hand != state.hand ||
      sim::EngineTestAccess::state(fixture.engine).deck != state.deck) {
    throw std::runtime_error(
        "Communication must reject the known targetless Treasure continuation.");
  }
}

void test_payload_route_rejects_fetched_only_quick_ball_target() {
  Fixture fixture("communication-quick-ball-fetched-only-target");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                sim::Card::QuickBall};
  state.deck = {sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Dialga-GX leaves the known deck during Pokémon Communication. The returned
  // Stage 1 Dipplin cannot satisfy Quick Ball's Basic target requirement after the
  // fetched Dialga-GX pays the discard cost:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://github.com/FlareZ123/pokemon-sims/issues/808
  if (sim::EngineTestAccess::play_pokemon_communication(fixture.engine, true) ||
      sim::EngineTestAccess::state(fixture.engine).hand != state.hand ||
      sim::EngineTestAccess::state(fixture.engine).deck != state.deck) {
    throw std::runtime_error(
        "Communication must reject the known targetless Quick Ball continuation.");
  }
}

void test_payload_route_accepts_a_different_mysterious_treasure_target() {
  Fixture fixture("communication-treasure-alternate-target");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::MawileGX,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Mawile-GX is not a Mysterious Treasure target. The later Item remains legal
  // because another Dragon Pokémon is still in the deck after Communication finds
  // the payload:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine, true) ||
      !sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, true)) {
    throw std::runtime_error("Communication should preserve the alternate Treasure target route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Treasure should discard the fetched payload and search the other Dragon target.");
  }
}

void test_payload_route_accepts_a_different_quick_ball_target() {
  Fixture fixture("communication-quick-ball-alternate-target");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                sim::Card::QuickBall};
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Dipplin is not Basic. Quick Ball remains legal because a different Basic Pokémon
  // stays in the deck after Communication finds the payload:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine, true) ||
      !sim::EngineTestAccess::play_quick_ball(fixture.engine, true)) {
    throw std::runtime_error("Communication should preserve the alternate Quick Ball target route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("Quick Ball should discard the fetched payload and search the other Basic target.");
  }
}


}  // namespace

int main() {
  test_baseline_recipe_is_unchanged();
  test_variant_recipe_supports_communication_without_changing_baseline();
  test_same_returned_pokemon_can_be_searched_from_an_empty_deck();
  test_resolver_requires_a_pokemon_in_hand();
  test_every_item_lock_blocks_communication();
  test_turn_two_item_lock_allows_turn_one_communication();
  test_direct_search_returns_dipplin_for_missing_regidrago();
  test_direct_search_protects_live_latias_while_finding_vstar();
  test_policy_holds_a_no_progress_known_deck_cycle();
  test_communication_is_not_a_payload_discard_outlet();
  test_communication_fetches_payload_for_a_separate_treasure_outlet();
  test_arven_can_fetch_a_live_communication_bridge();
  test_arven_rejects_communication_without_an_exchange_pokemon();
  test_forest_seal_stone_prefers_the_direct_pokemon_target();
  test_gladion_preserves_itself_behind_a_held_communication_route();
  test_gladion_can_recover_a_known_prized_communication();
  test_legacy_star_recovers_communication_and_exchange_pokemon();
  test_legacy_star_does_not_recover_communication_under_item_lock();
  test_team_yell_can_restore_vstar_for_held_communication();
  test_team_yell_can_restore_latias_for_held_communication();
  test_team_yell_rejects_communication_when_payload_is_still_missing();
  test_roseanne_can_restore_vstar_for_held_communication();
  test_roseanne_multimode_supports_communication_plus_vessel();
  test_lusamine_recognizes_future_arven_communication_route();
  test_final_bench_slot_is_reserved_for_communication_vstar_route();
  test_opening_selector_counts_only_an_unlocked_communication_route();
  test_direct_communication_routes_preserve_wonder_tag();
  test_k0_search_adapts_when_preferred_vstar_is_prized();
  test_preferred_absence_can_resolve_to_the_returned_pokemon();
  test_arven_does_not_count_itself_as_ultra_ball_fodder();
  test_wonder_tag_arven_plan_consumes_tapu_and_bench_space();
  test_gladion_does_not_recover_communication_using_itself_as_cost();
  test_payload_route_rejects_fetched_only_mysterious_treasure_target();
  test_payload_route_rejects_fetched_only_quick_ball_target();
  test_payload_route_accepts_a_different_mysterious_treasure_target();
  test_payload_route_accepts_a_different_quick_ball_target();
  return 0;
}
