# Issue 1645 refinement evidence

Source-bound witness: `crobat1-heavy-ball`, `matchup-flex-jit/go-second`, seed `218`.

The fixed recipe contains three Fire Energy, and the K1 Prize set contains two of them. Earthen Vessel therefore takes the only remaining deck Fire on T2. The corrected deterministic route is:

1. Steven's Resolve searches Latias ex, Gladion, and Grass Energy on T1.
2. On T2, Bench Latias ex, evolve the prior-turn Regidrago V, use Earthen Vessel to take Grass and the only deck Fire, manually attach Grass, and use Gladion to recover a prized Crispin.
3. On T3, Crispin searches only one Grass Energy under its printed “up to 2” text and attaches that Grass. Manually attach the held Fire, discard a permitted Dragon with Brilliant Blender, and retreat the Basic Active for free through Skyliner.

Crispin may search for up to two different Basic Energy, so a one-Grass resolution is legal: https://api.pokemontcg.io/v2/cards/sv7-133

Earthen Vessel searches up to two Basic Energy after one discard: https://api.pokemontcg.io/v2/cards/sv4-163

Latias ex gives Basic Pokémon no Retreat Cost: https://api.pokemontcg.io/v2/cards/sv8-76

Steven's Resolve searches up to three cards: https://api.pokemontcg.io/v2/cards/sm7-145

The fixed recipe contains three Fire Energy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_000.inc#L86-L99

This refinement preserves the reported Steven target-selection defect while correcting the impossible two-type T3 Crispin description.
