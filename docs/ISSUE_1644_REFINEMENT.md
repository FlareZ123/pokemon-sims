# Issue 1644 refinement evidence

Source-bound witness: `crobat2-erika-channeler`, `no-discard-control/go-first`, seed `83`.

The K1 Prize set contains Forest Seal Stone. The current selector exchanges Gladion for Grass Energy, although the live same-turn route is:

1. Exchange Gladion for Forest Seal Stone.
2. Attach Forest Seal Stone to the Active Regidrago V.
3. Use Star Alchemy to search Regidrago VSTAR.
4. Evolve the prior-turn Active Regidrago V and reach readiness on T3.

Forest Seal Stone lets the attached Pokémon V use Star Alchemy to search any card: https://api.pokemontcg.io/v2/cards/swsh12-156

Regidrago VSTAR evolves from Regidrago V and Apex Dragon costs `GGF`: https://api.pokemontcg.io/v2/cards/swsh12-136

Gladion exchanges itself for one Prize card: https://api.pokemontcg.io/v2/cards/sm4-95

The repository recognizes Regidrago V as a Pokémon V and permits Forest Seal Stone on it: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_001.inc#L88-L95

This refinement replaces the prior “hold Gladion” proposal. The bug is target selection: Grass Energy is inert while Forest Seal Stone completes the unresolved VSTAR axis.
