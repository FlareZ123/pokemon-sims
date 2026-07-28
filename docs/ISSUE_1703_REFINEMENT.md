# Issue 1703 refinement: strict-JIT route-replaced Vessel cost

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1703

The seed-38 pre-Steven Earthen Vessel route remains a valid setup defect only when its T1 discard is admitted through an explicit strict-JIT route-replacement proof.

The issue currently names singleton Path to the Peak as setup-inert fuel. Current strict DCI deliberately protects singleton matchup cards, and ordinary `choose_discard` does not admit Path to the Peak. The eventual fix must select a cost whose remaining modeled setup role is completely replaced by the exact T2 Vessel, Steven's Resolve, Crispin, Mysterious Treasure, and Dragon-payload route. Path to the Peak must remain protected whenever its discrete value remains in scope or the complete route is unavailable.

The exact positive gate must prove a payable Earthen Vessel, searchable Grass and Fire Energy, an unused T1 manual attachment, an evolution-eligible Regidrago V, held Crispin, searchable Regidrago VSTAR, searchable Mysterious Treasure, a searchable Dragon payload, a legal Treasure target after the payload cost, and no Item or Supporter lock. Negative controls must remove each prerequisite separately.

Sources:

- Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
- Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
- Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
- Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
- Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
- Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
- Repository DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#discard-capability-index-dci
- Repository dynamic DCI assumptions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
