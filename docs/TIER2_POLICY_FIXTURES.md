# Tier Two Policy Fixture Index

## Canonical surface

`regidrago_tier2_tests` executes **31** deterministic choice-differentiation, fast-compression, K1, and lock-aware fixtures. The runner table is the canonical inventory and order: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_003b.inc#L40-L73

Each listed function constructs its exact state in the split source files under `tests/tier2_parts/`. The executable assertions and their direct rule or card URLs are authoritative.

## Executed fixtures

1. `crispin_beats_arven`
2. `steven_beats_arven`
3. `search_then_known_gladion_beats_supporters`
4. `k0_does_not_peek_prizes`
5. `blind_gladion_is_critical_only`
6. `fss_uses_oricorio_compression`
7. `fss_uses_vessel_over_arven`
8. `fss_uses_direct_arven_not_tapu_chain`
9. `fss_uses_blender_for_payload`
10. `known_lusamine_is_delayed_not_direct`
11. `search_effects_persist_knowledge`
12. `zero_target_crispin_rejects_no_effect_play`
13. `k1_singleton_crispin_supplies_manual_energy`
14. `tapu_avoids_redundant_arven`
15. `tapu_uses_gladion_for_known_prized_vstar`
16. `crispin_beats_known_prized_oricorio`
17. `fss_holds_for_tate_switch`
18. `fss_uses_gladion_for_known_prized_vstar`
19. `heavy_ball_oricorio_compresses_energy`
20. `ability_lock_blocks_latias_quick_ball_connector`
21. `fss_is_blocked_by_rulebox_lock`
22. `k1_heavy_ball_beats_still_live_quick_ball`
23. `tate_draw_recovers_missing_vstar`
24. `arven_uses_available_vstar_item_fallback`
25. `fss_holds_for_live_crispin`
26. `ultra_ball_holds_for_live_crispin`
27. `costed_search_accepts_distinct_ultra_second_cost`
28. `tapu_holds_for_three_ultra_vstar_route`
29. `legacy_star_recovers_payable_incense_ultra_bridge`
30. `legacy_star_rejects_final_card_incense_ultra_bridge`
31. `legacy_star_rejects_unpayable_incense_ultra_bridge`

## Behavior notes

- `zero_target_crispin_rejects_no_effect_play` rejects Crispin when the publicly empty deck makes every effect known to fail: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_003b.inc#L1-L20 https://api.pokemontcg.io/v2/cards/sv7-133 https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
- `fss_is_blocked_by_rulebox_lock` is a historical identifier. The current fixture follows the Forest Seal Stone ruling that Star Alchemy is the Tool's Ability and remains usable through Path to the Peak: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
- The three Legacy Star plus Evolution Incense and Ultra Ball fixtures distinguish a payable bridge, a final-card search failure, and an unpayable two-card cost: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/swsh1-163 https://api.pokemontcg.io/v2/cards/swsh12pt5-146

## Run

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/regidrago_tier2_tests
```
