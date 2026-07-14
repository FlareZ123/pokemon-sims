  s.deck = {Card::StevensResolve};
  Fixture f(scenario("tapu-steven"), s);
  expect(EngineTestAccess::bench_tapu_if_useful(f.engine), "Tapu should be benched when Wonder Tag obtains the needed Steven bridge.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::StevensResolve), "Wonder Tag should retrieve Steven's Resolve.");
}

void test_latias_retreat_is_basic_only() {
  State s = state(2, Card::TapuLeleGX);
  s.bench = {Pokemon{Card::LatiasEx, 1, 0, 0, Tool::None}, Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  Fixture f(scenario("latias-retreat"), s);
  expect(EngineTestAccess::retreat_to_benched_vstar_with_latias(f.engine), "Latias should grant the Basic Active a free retreat into VSTAR.");
  expect(EngineTestAccess::state(f.engine).active->card == Card::RegidragoVstar, "Regidrago VSTAR should become Active after the free retreat.");

  State blocked = state(2, Card::RegidragoVstar, 0, 2, 1);
  blocked.bench = {Pokemon{Card::LatiasEx, 1, 0, 0, Tool::None}};
  Fixture not_basic(scenario("latias-vstar"), blocked);
  expect(!EngineTestAccess::can_free_retreat_with_latias(not_basic.engine), "Latias must not make a VSTAR Active free to retreat.");
}

void test_tate_switch_is_selected_when_it_completes_active_axis() {
  State s = state(2, Card::TapuLeleGX);
  s.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  s.hand = {Card::TateLiza};
  s.discarded_this_turn = {Card::MegaDragonite};
  s.discard = {Card::MegaDragonite};
  Fixture f(scenario("tate-switch"), s);
  expect(EngineTestAccess::play_tate_switch(f.engine), "Tate & Liza should use switch mode when it makes the ready VSTAR Active.");
  expect(EngineTestAccess::state(f.engine).active->card == Card::RegidragoVstar, "Tate & Liza switch mode should promote VSTAR.");
}

void test_strict_jit_requires_payload_to_be_in_discard_now_and_this_turn() {
  State s = state(3, Card::RegidragoVstar, 0, 2, 1);
  s.discard = {Card::MegaDragonite};
  Fixture strict_old(scenario("strict-old-payload"), s);
  expect(!EngineTestAccess::payload_ready(strict_old.engine), "Strict JIT must not count a payload discarded on a prior turn.");

  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture strict_current(scenario("strict-current-payload"), s);
  expect(EngineTestAccess::payload_ready(strict_current.engine), "Strict JIT should count a payload discarded this turn and still in discard.");
  expect(EngineTestAccess::recover_discard_to_hand(strict_current.engine, Card::MegaDragonite), "Fixture should recover the current-turn payload.");
  expect(!EngineTestAccess::payload_ready(strict_current.engine), "A payload recovered from discard must not satisfy strict JIT.");

  Fixture no_control(scenario("no-control-old-payload", DciProfile::NoDiscardControl), state(3, Card::RegidragoVstar, 0, 2, 1));
  EngineTestAccess::state(no_control.engine).discard = {Card::MegaDragonite};
  expect(EngineTestAccess::payload_ready(no_control.engine), "No-control profile may count a prior-turn payload already in discard.");
}

void test_finished_setup_does_not_burn_serena() {
  State s = state(3, Card::RegidragoVstar, 0, 2, 1);
  s.hand = {Card::Serena, Card::Dipplin};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(scenario("hold-serena"), s);
  expect(!EngineTestAccess::setup_axis_missing(f.engine), "The fixture begins fully ready.");
  EngineTestAccess::choose_supporter(f.engine);
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::Serena) && !after.supporter_used, "A ready state must not burn Serena for incidental draw.");
}

void test_arven_fetches_forest_seal_stone_during_item_lock() {
  State s = state(2);
  s.hand = {Card::Arven};
  // Forest Seal Stone may attach to Regidrago V under Item lock, then Star Alchemy
  // may search Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/swsh12-136
  s.deck = {Card::EvolutionIncense, Card::ForestSealStone, Card::RegidragoVstar};
  Fixture f(scenario("arven-item-lock", DciProfile::StrictJit, LockMode::FullItem), s);

  // Arven is a Supporter, so Item lock does not prohibit playing it; Pokémon Tools
  // remain separate from Items after the Tool erratum: https://api.pokemontcg.io/v2/cards/sv1-166 https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  expect(EngineTestAccess::play_arven(f.engine), "Arven should use the live Forest Seal Stone Tool route during Item lock.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::ForestSealStone) && after.supporter_used,
         "Arven should fetch Forest Seal Stone and consume the Supporter play.");
}

void test_first_turn_restrictions_and_celestial_cost() {
  State first = state(1, Card::RegidragoV, 0, 1, 0);
  first.hand = {Card::Arven};
  Fixture going_first(scenario("first-turn", DciProfile::StrictJit, LockMode::None, true), first);
  expect(!EngineTestAccess::supporter_allowed(going_first.engine), "Going first turn one must forbid Supporters.");
  expect(!EngineTestAccess::use_celestial_roar(going_first.engine), "Going first turn one must forbid attacking.");

  State unpowered = state(1, Card::RegidragoV, 0, 0, 0);
  unpowered.deck = {Card::Grass, Card::Fire, Card::Grass};
  Fixture no_cost(scenario("roar-no-cost"), unpowered);
  expect(!EngineTestAccess::use_celestial_roar(no_cost.engine), "Celestial Roar requires one attached Energy.");

  State powered = state(1, Card::RegidragoV, 0, 1, 0);
  powered.deck = {Card::Grass, Card::Fire, Card::Grass};
  Fixture cost_paid(scenario("roar-paid"), powered);
  expect(EngineTestAccess::use_celestial_roar(cost_paid.engine), "Powered Regidrago V should use Celestial Roar going second.");
  expect(EngineTestAccess::state(cost_paid.engine).active->grass == 3 && EngineTestAccess::state(cost_paid.engine).active->fire == 1,
         "Celestial Roar should attach every Energy among the discarded top three cards.");
}

void test_evolution_timing_and_single_manual_attachment() {
  State new_regi = state(2, Card::RegidragoV, 2, 0, 0);
  new_regi.hand = {Card::RegidragoVstar, Card::Grass, Card::Fire};
  Fixture same_turn(scenario("evolve-same-turn"), new_regi);
  expect(!EngineTestAccess::evolve_best_regi(same_turn.engine), "A Regidrago V played this turn cannot evolve.");

  State old_regi = state(2, Card::RegidragoV, 1, 0, 0);
  old_regi.hand = {Card::RegidragoVstar, Card::Grass, Card::Fire};
  Fixture next_turn(scenario("evolve-next-turn"), old_regi);
  expect(EngineTestAccess::evolve_best_regi(next_turn.engine), "A prior-turn Regidrago V may evolve.");
  expect(EngineTestAccess::attach_manual(next_turn.engine), "One manual attachment should advance GGF.");
  expect(!EngineTestAccess::attach_manual(next_turn.engine), "The second manual attachment in the same turn must fail.");
}
