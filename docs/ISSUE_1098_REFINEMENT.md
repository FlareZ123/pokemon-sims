# Issue 1098 refinement

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1098

## Corrected probability boundary

The canonical list contains three Fire Energy: https://github.com/FlareZ123/pokemon-sims/blob/main/data/decklist.json

In the source-bound seed-44 K1 state, one Fire Energy is attached to Regidrago VSTAR and one is in the revealed Prizes. Exactly one Fire Energy remains in the 44-card shuffled deck.

Steven's Resolve may search up to three cards and ends the turn: https://api.pokemontcg.io/v2/cards/sm7-145

Searching exactly Crispin plus Grass Energy leaves 42 cards. A Fire Energy topdeck on T3 removes Crispin's second searchable Basic Energy type, so Crispin cannot attach an Energy. Crispin card text: https://api.pokemontcg.io/v2/cards/sv7-133

The proposed Mysterious Treasure continuation remains legal on every non-Fire topdeck because multiple Psychic or Dragon targets remain. Mysterious Treasure card text: https://api.pokemontcg.io/v2/cards/sm6-113

The corrected route probability is `41 / 42 = 97.61905%`. The original deterministic claim is withdrawn. The refined bug remains that the policy chooses Serena's one-card draw instead of this much stronger visible-state route.

Regidrago VSTAR and Apex Dragon source: https://api.pokemontcg.io/v2/cards/swsh12-136

Official turn, search, shuffle, Supporter, Item, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules

Repository K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

No production logic is changed on this branch until the refined issue receives two new independent approvals.
