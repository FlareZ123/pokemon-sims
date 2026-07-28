# Issue 1702 refinement: strict-JIT Item-cost gate

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1702

The Heavy Ball target-selection defect remains valid only when the complete T2 route can legally pay both held Item costs under the repository's strict-JIT DCI policy.

The proposed route uses Hisuian Heavy Ball for the uniquely prized Latias ex, then Mysterious Treasure for a replaceable Regidrago V, Earthen Vessel for Grass plus Fire Energy, Forest Seal Stone for Regidrago VSTAR, and Steven's Resolve for Crispin plus Brilliant Blender. Its second discard cost is singleton Channeler.

Current strict DCI ordinarily protects singleton matchup cards. The eventual fix must therefore prove that Channeler's remaining modeled setup role is fully replaced by the exact deterministic T2 route before admitting it as Mysterious Treasure or Earthen Vessel fuel. Regidrago V must remain the Heavy Ball target whenever that proof fails or no other policy-admissible second cost exists.

Required negative controls include missing Mysterious Treasure, missing or protected Channeler, incomplete Energy access, unavailable Forest Seal Stone or VSTAR Power, absent Crispin or Brilliant Blender, insufficient Bench space, Ability lock, and any state where Latias ex has another live recovery route.

Sources:

- Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
- Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
- Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
- Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
- Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
- Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
- Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
- Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
- Repository DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#discard-capability-index-dci
- Repository dynamic DCI assumptions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
