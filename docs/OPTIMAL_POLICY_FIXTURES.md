# Core Exact-State Policy Fixture Index

## Canonical surface

`regidrago_policy_tests` executes **57** deterministic exact-state fixtures. The runner table is the canonical inventory and order: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/policy_fixture_v2/part_004a.inc#L134-L191

Each listed function constructs its hand, board, deck, Prize, discard, lock, expected action, and expected resulting state in the adjacent split source files under `tests/policy_fixture_v2/`. The executable assertions and their direct rule or card URLs are authoritative when a historical function identifier is awkward or stale.

## Executed fixtures

1. `gladion_fetches_prized_vstar_only_when_no_route_exists`
2. `gladion_is_not_burned_when_vstar_is_already_in_hand`
3. `steven_fetches_burnet_before_turn_two_item_lock`
4. `steven_uses_legal_fallback_when_preferred_card_is_prized`
5. `steven_resolve_reserves_gladion_for_prized_vstar`
6. `steven_skips_known_dead_crispin`
7. `fss_finds_steven_on_turn_one_when_three_axes_are_missing`
8. `fss_uses_a_fallback_when_vstar_is_not_in_deck`
9. `fss_fetches_missing_basic_energy_for_manual_attachment`
10. `fss_cannot_be_used_from_a_vstar_holder`
11. `existing_fss_is_usable_under_item_lock`
12. `fss_is_blocked_by_rule_box_lock`
13. `crispin_attaches_only_with_two_different_energy_types`
14. `crispin_single_type_only_puts_energy_in_hand`
15. `earthen_vessel_can_take_two_grass`
16. `strict_jit_holds_payload_before_turn_two`
17. `no_control_allows_early_payload_banking`
18. `blender_dominates_mysterious_treasure_when_no_search_axis_exists`
19. `searches_use_legal_fallback_targets_after_deck_inspection`
20. `heavy_ball_inspects_prizes_and_chains_tapu`
21. `known_no_basic_prize_holds_heavy_ball_for_discard_cost`
22. `k1_gladion_recovers_final_prized_energy`
23. `heavy_ball_reveal_marks_crispin_dead`
24. `fss_reveal_preserves_burnet_over_prized_energy`
25. `celestial_roar_attached_energy_leaves_discard`
26. `tate_draws_from_a_four_card_dead_hand`
27. `tate_refreshes_a_five_card_dead_hand`
28. `fss_fetches_gladion_for_final_prized_energy`
29. `full_bench_blocks_basic_search_connectors`
30. `strict_serena_uses_three_safe_discards`
31. `heavy_ball_oricorio_preserves_burnet_supporter`
32. `legacy_star_recovered_blender_is_played_same_turn`
33. `legacy_star_recovered_crispin_is_played_same_turn`
34. `legacy_star_recovers_burnet_for_deck_payload`
35. `lusamine_recovers_only_live_axis_connectors`
36. `wonder_tag_skips_known_dead_crispin`
37. `payload_selectors_fetch_burnet_when_blender_is_unavailable`
38. `live_burnet_preempts_arven_when_blender_is_absent`
39. `tapu_fetches_burnet_without_item_lock_when_blender_is_not_live`
40. `lusamine_recovers_current_axis_connectors`
41. `arven_requires_a_live_item_or_tool_route`
42. `oricorio_is_held_when_earthen_vessel_already_solves_energy`
43. `oricorio_works_under_rule_box_lock`
44. `tapu_is_blocked_by_rule_box_lock`
45. `tapu_fetches_steven_when_it_is_the_live_turn_one_connector`
46. `latias_retreat_is_basic_only`
47. `tate_switch_is_selected_when_it_completes_active_axis`
48. `strict_jit_requires_payload_to_be_in_discard_now_and_this_turn`
49. `finished_setup_does_not_burn_serena`
50. `arven_fetches_forest_seal_stone_during_item_lock`
51. `arven_is_not_played_during_item_lock`
52. `first_turn_restrictions_and_celestial_cost`
53. `evolution_timing_and_single_manual_attachment`
54. `item_lock_blocks_item_cards`
55. `ultra_ball_discards_payload_in_jit_window`
56. `dipplin_is_legal_treasure_fallback_but_not_payload_ready`
57. `late_jit_item_search_evolves_in_the_same_turn`

## Historical identifiers with corrected assertions

- `fss_cannot_be_used_from_a_vstar_holder` is a retained historical identifier. Its current assertion requires Star Alchemy to work from a Regidrago VSTAR holder because Pokémon V includes Pokémon VSTAR: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/policy_fixture_v2/part_001.inc#L67-L79 https://compendium.pokegym.net/category/7-gameplay/pokemon-v/ https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
- `fss_is_blocked_by_rule_box_lock` is also a retained historical identifier. Forest Seal Stone's Ability remains usable through Path to the Peak because Star Alchemy is the Tool's Ability: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
- The registered K0 Heavy Ball fixtures follow the repository knowledge policy and play Hisuian Heavy Ball as an information action while Items are legal: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection https://api.pokemontcg.io/v2/cards/swsh10-146

## Run

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/regidrago_policy_tests
```
