
void test_item_lock_blocks_item_cards() {
  State s = state(2);
  s.hand = {Card::EarthenVessel, Card::Dipplin};
  s.deck = {Card::Grass, Card::Fire};
  Fixture f(scenario("item-lock", DciProfile::StrictJit, LockMode::FullItem), s);
  expect(EngineTestAccess::item_locked(f.engine), "The fixture should have Item lock.");
  expect(!EngineTestAccess::play_earthen_vessel(f.engine, false), "Item lock must prevent Earthen Vessel.");
}

using Test = std::pair<const char*, std::function<void()>>;

}  // namespace

int main() {
  const std::vector<Test> tests{
      {"gladion_fetches_prized_vstar_only_when_no_route_exists", test_gladion_fetches_prized_vstar_only_when_no_route_exists},
      {"gladion_is_not_burned_when_vstar_is_already_in_hand", test_gladion_is_not_burned_when_vstar_is_already_in_hand},
      {"steven_fetches_burnet_before_turn_two_item_lock", test_steven_fetches_burnet_before_turn_two_item_lock},
      {"steven_uses_legal_fallback_when_preferred_card_is_prized", test_steven_uses_legal_fallback_when_preferred_card_is_prized},
      {"fss_finds_steven_on_turn_one_when_three_axes_are_missing", test_fss_finds_steven_on_turn_one_when_three_axes_are_missing},
      {"fss_uses_a_fallback_when_vstar_is_not_in_deck", test_fss_uses_a_fallback_when_vstar_is_not_in_deck},
      {"fss_cannot_be_used_from_a_vstar_holder", test_fss_cannot_be_used_from_a_vstar_holder},
      {"existing_fss_is_usable_under_item_lock", test_existing_fss_is_usable_under_item_lock},
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
      {"prized_energy_does_not_preempt_item_lock_payload_bridge", test_prized_energy_does_not_preempt_item_lock_payload_bridge},
      {"celestial_roar_attached_energy_leaves_discard", test_celestial_roar_attached_energy_leaves_discard},
      {"tate_draws_from_a_four_card_dead_hand", test_tate_draws_from_a_four_card_dead_hand},
      {"fss_fetches_gladion_for_final_prized_energy", test_fss_fetches_gladion_for_final_prized_energy},
      {"full_bench_blocks_basic_search_connectors", test_full_bench_blocks_basic_search_connectors},
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
