# Issue 1705 refinement: strict-JIT Treasure and Vessel cost gates

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1705

The seed-461 pre-Steven Treasure, Latias ex, and Earthen Vessel route remains valid only with two explicit strict-JIT route-replacement proofs.

First, singleton Field Blower is ordinarily protected by strict DCI. The fix may spend it through Mysterious Treasure only when the lock-free observable state proves that Field Blower has no remaining modeled setup role and the complete T2 route is otherwise deterministic.

Second, Dragapult ex is discarded through Earthen Vessel before the ready turn. Strict JIT may admit that early payload cost only when Brilliant Blender is guaranteed to supply a permitted Dragon payload during the T2 ready turn. The payload must remain protected when Blender, a legal Blender payload, the VSTAR and Energy package, or the Latias promotion route is incomplete.

The positive gate must prove legal and payable Mysterious Treasure and Earthen Vessel actions, Bench space for Latias ex, searchable Grass and Fire Energy, an unused T1 manual attachment, an evolution-eligible Regidrago V, held or searchable Regidrago VSTAR, Crispin, Brilliant Blender, a legal current-turn Blender payload, Skyliner availability, and no relevant Item or Rule Box Ability lock. Negative controls must remove each prerequisite separately.

Sources:

- Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
- Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
- Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
- Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
- Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
- Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
- Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
- Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
- Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Repository DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#discard-capability-index-dci
- Repository dynamic DCI assumptions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
