
void test_item_lock_blocks_item_cards() {
  State s = state(2);
  s.hand = {Card::EarthenVessel, Card::Dipplin};
  s.deck = {Card::Grass, Card::Fire};
  Fixture f(scenario("item-lock", DciProfile::StrictJit, LockMode::FullItem), s);
  expect(EngineTestAccess::item_locked(f.engine), "The fixture should have Item lock.");
  expect(!EngineTestAccess::play_earthen_vessel(f.engine, false), "Item lock must prevent Earthen Vessel.");
}

void test_ultra_ball_discards_payload_in_jit_window() {
  State s = state(2, Card::RegidragoVstar, 0, 2, 1);
  s.hand = {Card::UltraBall, Card::MegaDragonite, Card::Dipplin};
  s.deck = {Card::MawileGX};
  Fixture f(scenario("ultra-ball-jit-payload"), s);
  expect(EngineTestAccess::run_search_items_one_step(f.engine, true),
         "Ultra Ball should be available as a current-turn payload discard outlet.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.discard, Card::MegaDragonite) && contains(after.discarded_this_turn, Card::MegaDragonite),
         "Ultra Ball must place the selected Dragon payload in discard during this turn.");
  expect(contains(after.hand, Card::MawileGX),
         "Ultra Ball must complete its required Pokémon search after paying the discard cost.");
}

void test_tate_refreshes_a_five_card_dead_hand() {
  State s = state(2);
  s.hand = {Card::TateLiza, Card::Dipplin, Card::MawileGX, Card::Guzma, Card::Channeler};
  s.deck = {Card::RegidragoVstar, Card::Grass};
  // Tate & Liza shuffles the whole hand into the deck, then draws 5: https://api.pokemontcg.io/v2/cards/sm7-148
  Fixture f(scenario("tate-five-card-refresh"), s);
  EngineTestAccess::choose_supporter(f.engine);
  const State& after = EngineTestAccess::state(f.engine);
  expect(after.supporter_used && contains(after.discard, Card::TateLiza),
         "Tate & Liza must be usable from a five-card dead hand.");
  expect(after.hand.size() == 5U, "Tate & Liza draw mode must redraw five cards.");
}

void test_fss_fetches_missing_basic_energy_for_manual_attachment() {
  State s = state(2, Card::RegidragoV, 0, 1, 0);
  s.active->tool = Tool::ForestSealStone;
  s.prizes = {Card::RegidragoVstar};
  s.deck = {Card::Fire};
  // Star Alchemy searches the deck for any card: https://api.pokemontcg.io/v2/cards/swsh12-156
  Fixture f(scenario("fss-direct-fire"), s);
  expect(EngineTestAccess::use_fss(f.engine), "Forest Seal Stone should remain usable when Fire is the only live missing axis.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::Fire),
         "Star Alchemy should fetch the missing Basic Fire Energy for this turn's manual attachment.");
}

using Test = std::pair<const char*, std::function<void()>>;

}  // namespace

int main() {
  const std::vector<Test> tests{
      {"gladion_fetches_prized_vstar_only_when_no_route_exists", test_gladion_fetches_prized_vstar_only_when_no_route_exists},
      {"gladion_is_not_burned_when_vstar_is_already_in_hand", test_gladion_is_not_burned_when_vstar_is_already_in_hand},
      {"steven_fetches_burnet_before_turn_two_item_lock", test_steven_fetches_burnet_before_turn_two_item_lock},
      {"steven_uses_legal_fallback_when_preferred_card_is_prized", test_steven_uses_legal_fallback_when_preferred_card_is_prized},
      {"steven_resolve_reserves_gladion_for_prized_vstar", test_steven_resolve_reserves_gladion_for_prized_vstar},
      {"steven_skips_known_dead_crispin", test_steven_skips_known_dead_crispin},
      {"fss_finds_steven_on_turn_one_when_three_axes_are_missing", test_fss_finds_steven_on_turn_one_when_three_axes_are_missing},
      {"fss_uses_a_fallback_when_vstar_is_not_in_deck", test_fss_uses_a_fallback_when_vstar_is_not_in_deck},
      {"fss_fetches_missing_basic_energy_for_manual_attachment", test_fss_fetches_missing_basic_energy_for_manual_attachment},
      {"fss_cannot_be_used_from_a_vstar_holder", test_fss_cannot_be_used_from_a_vstar_holder},
      {"existing_fss_is_usable_under_item_lock", test_existing_fss_is_usable_under_item_lock},
      {"fss_is_blocked_by_rule_box_lock", test_fss_is_blocked_by_rule_box_lock},
      {"crispin_attaches_only_with_two_different_energy_types", test_crispin_attaches_only_with_two_different_energy_types},
      {"crispin_single_type_only_puts_energy_in_hand", test_crispin_single_type_only_puts_energy_in_hand},
      {"earthen_vessel_can_take_two_grass", test_earthen_vessel_can_take_two_grass},
      {"strict_jit_holds_payload_before_turn_two", test_strict_jit_holds_payload_before_turn_two},
      {"no_control_allows_early_payload_banking", test_no_control_allows_early_payload_banking},
      {"blender_dominates_mysterious_treasure_when_no_search_axis_exists", test_blender_dominates_mysterious_treasure_when_no_search_axis_exists},
      {"searches_use_legal_fallback_targets_after_deck_inspection", test_searches_use_legal_fallback_targets_after_deck_inspection},
      {"heavy_ball_inspects_prizes_and_chains_tapu", test_heavy_ball_inspects_prizes_and_chains_tapu},
      {"known_no_basic_prize_holds_heavy_ball_for_discard_cost", test_known_no_basic_prize_holds_heavy_ball_for_discard_cost},
      {"k1_gladion_recovers_final_prized_energy", test_k1_gladion_recovers_final_prized_energy},
      {"heavy_ball_reveal_marks_crispin_dead", test_heavy_ball_reveal_marks_crispin_dead},
      {"fss_reveal_preserves_burnet_over_prized_energy", test_fss_reveal_preserves_burnet_over_prized_energy},
      {"celestial_roar_attached_energy_leaves_discard", test_celestial_roar_attached_energy_leaves_discard},
      {"tate_draws_from_a_four_card_dead_hand", test_tate_draws_from_a_four_card_dead_hand},
      {"tate_refreshes_a_five_card_dead_hand", test_tate_refreshes_a_five_card_dead_hand},
      {"fss_fetches_gladion_for_final_prized_energy", test_fss_fetches_gladion_for_final_prized_energy},
      {"full_bench_blocks_basic_search_connectors", test_full_bench_blocks_basic_search_connectors},
      {"strict_serena_uses_three_safe_discards", test_strict_serena_uses_three_safe_discards},
      {"heavy_ball_oricorio_preserves_burnet_supporter", test_heavy_ball_oricorio_preserves_burnet_supporter},
      {"legacy_star_recovered_blender_is_played_same_turn", test_legacy_star_recovered_blender_is_played_same_turn},
      {"lusamine_recovers_only_live_axis_connectors", test_lusamine_recovers_only_live_axis_connectors},
      {"wonder_tag_skips_known_dead_crispin", test_wonder_tag_skips_known_dead_crispin},
      {"payload_selectors_fetch_burnet_when_blender_is_unavailable", test_payload_selectors_fetch_burnet_when_blender_is_unavailable},
      {"live_burnet_preempts_arven_when_blender_is_absent", test_live_burnet_preempts_arven_when_blender_is_absent},
      {"tapu_fetches_burnet_without_item_lock_when_blender_is_not_live", test_tapu_fetches_burnet_without_item_lock_when_blender_is_not_live},
      {"lusamine_recovers_current_axis_connectors", test_lusamine_recovers_current_axis_connectors},
      {"arven_requires_a_live_item_or_tool_route", test_arven_requires_a_live_item_or_tool_route},
      {"oricorio_is_held_when_earthen_vessel_already_solves_energy", test_oricorio_is_held_when_earthen_vessel_already_solves_energy},
      {"oricorio_works_under_rule_box_lock", test_oricorio_works_under_rule_box_lock},
      {"tapu_is_blocked_by_rule_box_lock", test_tapu_is_blocked_by_rule_box_lock},
      {"tapu_fetches_steven_when_it_is_the_live_turn_one_connector", test_tapu_fetches_steven_when_it_is_the_live_turn_one_connector},
      {"latias_retreat_is_basic_only", test_latias_retreat_is_basic_only},
      {"tate_switch_is_selected_when_it_completes_active_axis", test_tate_switch_is_selected_when_it_completes_active_axis},
      {"strict_jit_requires_payload_to_be_in_discard_now_and_this_turn", test_strict_jit_requires_payload_to_be_in_discard_now_and_this_turn},
      {"finished_setup_does_not_burn_serena", test_finished_setup_does_not_burn_serena},
      {"arven_is_not_played_during_item_lock", test_arven_is_not_played_during_item_lock},
      {"first_turn_restrictions_and_celestial_cost", test_first_turn_restrictions_and_celestial_cost},
      {"evolution_timing_and_single_manual_attachment", test_evolution_timing_and_single_manual_attachment},
      {"item_lock_blocks_item_cards", test_item_lock_blocks_item_cards},
      {"ultra_ball_discards_payload_in_jit_window", test_ultra_ball_discards_payload_in_jit_window},
  };

  int failures = 0;
  for (const auto& [name, run] : tests) {
    try {
      run();
      std::cout << "PASS " << name << '\n';
    } catch (const std::exception& error) {
      ++failures;
      std::cerr << "FAIL " << name << ": " << error.what() << '\n';
    }
  }
  std::cout << "Policy fixtures: " << (tests.size() - static_cast<std::size_t>(failures))
            << '/' << tests.size() << " passed\n";
  return failures == 0 ? 0 : 1;
}