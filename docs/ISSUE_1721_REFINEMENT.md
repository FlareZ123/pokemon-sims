# Issue 1721 observable-state refinement

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1721

This document records the refinement that removes future-draw oracle reasoning from issue 1721. It is evidence only. It does not implement or claim the bug.

## Source-bound witness

Current merged source at refinement base:

```text
main@fa0422b0db6215e063b236357d4b8396dd54c4ca
```

Successful source-equivalent PR CI:

- PR: https://github.com/FlareZ123/pokemon-sims/pull/1718
- CI run: https://github.com/FlareZ123/pokemon-sims/actions/runs/30367085740
- Release artifact ID: `8691571495`
- Artifact digest: `sha256:8bdab931d4d0cde2c6d8a8a0a6f990014428e5bac855d38716eb4282a6db27d2`

Original debug witness:

- Audit PR: https://github.com/FlareZ123/pokemon-sims/pull/1720
- Audit run: https://github.com/FlareZ123/pokemon-sims/actions/runs/30366078554
- Artifact ID: `8690842765`
- Artifact digest: `sha256:3deda897fe614b3f3365c1bb2fe4de5ecaf5c7603df127cef34a67b2b42d4b21`

```text
./build/regidrago_sim --simulate-this \
  --model-variant crobat2-erika-channeler \
  --scenario strict-jit/go-second \
  --seed 31415
```

The current trace establishes K1 on T1, then takes no further action and reaches readiness on T3.

## Observable T1 route

After the legal Mysterious Treasure search, the public state has Active Regidrago V, open Bench space, an unused manual attachment, no lock, and a hand containing Field Blower, Quick Ball, Arven, Regidrago VSTAR, Hisuian Goodra VSTAR, and Mega Dragonite ex.

K1 proves one Earthen Vessel, Powerglass, Oricorio, sufficient Grass and Fire Energy, two additional Quick Ball, two additional Mysterious Treasure, Brilliant Blender, Professor Burnet, Serena, two Dragapult ex, one Mega Dragonite ex, and Dialga-GX remain in the deck.

The legal route is:

1. Play Arven for Earthen Vessel and Powerglass.
2. Play Earthen Vessel, discarding setup-dead Field Blower, and search Grass plus Fire.
3. Attach Powerglass to Active Regidrago V.
4. Play Quick Ball, discarding Fire, and search Oricorio.
5. Bench Oricorio and use Vital Dance for two Grass Energy.
6. Manually attach one Grass to Active Regidrago V.
7. Hold Celestial Roar.
8. Resolve Powerglass to attach the discarded Fire at end of turn.

Relevant card data:

- Arven: https://api.pokemontcg.io/v2/cards/sv1-166
- Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
- Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
- Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
- Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
- Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
- Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
- Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
- Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
- Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164

Official turn procedure:
https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf

Repository knowledge, DCI, earliest-route, and future-oracle policy:

- https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
- https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
- https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
- https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle

## Conservative T2 probability

After the route, the known deck has 38 cards before the T2 draw.

Seven draw identities directly provide a current-turn strict-JIT payload outlet:

- Quick Ball x2
- Mysterious Treasure x2
- Brilliant Blender x1
- Professor Burnet x1
- Serena x1

When the draw is one of the other 31 cards, Legacy Star samples seven of the remaining 37 cards. Eleven cards guarantee success:

- the seven outlets above
- Dragapult ex x2
- Mega Dragonite ex x1
- Dialga-GX x1

The lower bound is:

```text
7/38 + (31/38) * (1 - C(26,7) / C(37,7))
= 0.9478774400
= 94.787744%
```

This lower bound excludes additional lines through Arven, Tapu Lele-GX, Hisuian Heavy Ball, Crobat V, and Tate & Liza. The T1 decision is justified by the public K1 multiset and current board. The fixed T2 Professor Burnet draw is regression evidence only.

## Approval state

The refinement resets the approval count to zero. Two independent approvals must apply to the refined issue wording before claim or implementation.
