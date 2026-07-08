# Exact-State Policy Fixture Catalog

## What these tests prove

These are **policy-oracle fixtures**, not a mathematical proof over every possible hidden deck order. Every fixture builds an exact player-visible state, calls the same policy method used by the simulator, and asserts the selected action and resulting state. The objective is the earliest legal setup-ready state: Active Regidrago VSTAR with GGF and a payload that meets the selected DCI/JIT profile.

Notation: `T` is the player turn; `A` is Active; `B` is Bench; `H` is hand; `D` is deck; `P` is Prizes; `X` is discard; `DTT` is cards discarded this turn. Omitted zones are empty. `G` and `F` are Energy attached to the named Pokémon.

## Fixture list

1. **Selects Gladion only when it is the sole VSTAR route.** Initial: `T1; A Regidrago V[-]; H Gladion; P Regidrago VSTAR, Grass, Fire, Dipplin, Mawile-GX, Guzma`. Pass: Gladion is played, VSTAR moves to hand, Gladion is shuffled into Prizes, and Gladion is not discarded.
2. **Holds Gladion when VSTAR is already in hand.** Initial: `T2; A Regidrago V[G]; H Gladion, Regidrago VSTAR; P Regidrago VSTAR`. Pass: Gladion remains in hand and is not spent.
3. **Uses Steven's Resolve as the correct turn-one Item-lock bridge.** Initial: `T1 going second; A Regidrago V[-]; H Steven's Resolve; D Regidrago VSTAR, Crispin, Professor Burnet, Brilliant Blender; turn-two Item lock`. Pass: searches VSTAR, Crispin, Burnet, ends turn, and does not take Blender.
4. **Uses a useful legal Steven fallback after a preferred card is prized.** Initial: `T1 going second; A Regidrago V[-]; H Steven's Resolve; P Regidrago VSTAR; D Crispin, Professor Burnet, Earthen Vessel; turn-two Item lock`. Pass: finds Crispin, Burnet, Earthen Vessel rather than leaving an unused search slot.
5. **Uses Forest Seal Stone for Steven when three axes are missing.** Initial: `T1 going second; A Regidrago V[-] with Forest Seal Stone; D Steven's Resolve`. Pass: Star Alchemy finds Steven and consumes the single VSTAR Power.
6. **Uses Star Alchemy fallback when VSTAR is not in deck.** Initial: `T2; A Regidrago V[G] with Forest Seal Stone; P Regidrago VSTAR; D Crispin, Professor Burnet`. Pass: Star Alchemy finds Crispin instead of failing.
7. **Does not use Forest Seal Stone from an evolved holder.** Initial: `T2; A Regidrago VSTAR[GGF] with Forest Seal Stone; D Arven`. Pass: Star Alchemy is rejected.
8. **Uses already attached Forest Seal Stone through Item lock.** Initial: `T2; A Regidrago V[G] with Forest Seal Stone; D Regidrago VSTAR; full Item lock`. Pass: Star Alchemy finds VSTAR because no Item is being played from hand.
9. **Crispin attaches only after finding two different Basic Energy types.** Initial: `T2; A Regidrago V[G]; H Crispin; D Grass, Fire`. Pass: attaches Grass and puts Fire into hand.
10. **Crispin does not illegally attach with one Energy type.** Initial: `T2; A Regidrago V[G]; H Crispin; D Fire`. Pass: Fire enters hand and no Energy attaches.
11. **Earthen Vessel can take two Grass.** Initial: `T2; A Regidrago V[F]; H Earthen Vessel, Dipplin; D Grass, Grass`. Pass: Dipplin pays the cost and both Grass cards enter hand.
12. **Strict JIT protects the only payload before turn two.** Initial A: `T1; A Regidrago V[-]; H Mysterious Treasure, Mega Dragonite ex; D Regidrago VSTAR`. Pass A: Treasure is not played. Initial B is identical except `T2`. Pass B: Treasure discards Mega Dragonite ex.
13. **No-control permits early payload banking.** Initial: `T1; A Latias ex[-]; H Quick Ball, Mega Dragonite ex; D Regidrago V`. Pass: strict JIT declines; no-discard-control plays Quick Ball, discards Mega Dragonite ex, and searches Regidrago V.
14. **Brilliant Blender dominates a redundant Mysterious Treasure payload line.** Initial: `T2; A Regidrago VSTAR[GGF]; H Brilliant Blender, Mysterious Treasure, Dipplin; D Mega Dragonite ex`. Pass: Blender is played and discards Mega Dragonite ex; Treasure remains in hand.
15. **Searches choose a legal fallback after inspection.** Initial: `T2; A Latias ex[-]; H Quick Ball, Dipplin; D Tapu Lele-GX`. Pass: Quick Ball is played, Dipplin pays the cost, and Tapu Lele-GX is found instead of treating the search as a failure.
16. **Heavy Ball is a K0 Prize-information action and chains through Tapu.** Initial: `T2; A Regidrago V[-]; H Heavy Ball, Quick Ball, Dipplin; P Tapu Lele-GX; D Crispin, Grass, Fire`. Pass: Heavy Ball is played before Quick Ball, exchanges itself for Tapu Lele-GX, then the next stabilization pass Benches Tapu and Wonder Tag finds Crispin. Crispin is selected as the Supporter.
17. **Known no-Basic Prizes preserve Heavy Ball as discard fodder.** Initial: `T2; A Regidrago V[-]; H Mysterious Treasure, Dipplin, Heavy Ball, Quick Ball; D VSTAR, Oricorio, Grass, Fire; P Grass, Fire, Dipplin`. Pass: Treasure establishes K1 and gets VSTAR. Heavy Ball is held because no Prize Basic exists. Quick Ball then discards Heavy Ball and gets Oricorio.
18. **Holds Oricorio when Earthen Vessel already covers Energy.** Initial: `T2; A Regidrago V[-]; H Oricorio, Earthen Vessel, Dipplin; D Grass, Fire`. Pass: Oricorio remains in hand; Vessel is the chosen direct connector and finds G plus F.
19. **Uses Oricorio through Rule Box Ability lock.** Initial: `T2; A Regidrago V[-]; H Oricorio; D Grass, Fire; full Rule Box lock`. Pass: Oricorio is Benched and Vital Dance adds Grass and Fire to hand.
20. **Blocks Tapu Lele-GX Wonder Tag through Rule Box lock.** Initial: `T1; A Regidrago V[-]; H Tapu Lele-GX; D Crispin; full Rule Box lock`. Pass: Tapu is Benched, but Crispin is not searched.
21. **Uses Tapu Lele-GX for Steven when Steven is the live turn-one bridge.** Initial: `T1 going second; A Regidrago V[-]; H Tapu Lele-GX; D Steven's Resolve`. Pass: Tapu is Benched and Wonder Tag finds Steven.
22. **Uses Latias only for a Basic Active's free retreat.** Initial A: `T2; A Tapu Lele-GX[-]; B Latias ex[-], Regidrago VSTAR[GGF]`. Pass A: free retreat promotes VSTAR. Initial B: `T2; A Regidrago VSTAR[GGF]; B Latias ex[-]`. Pass B: free retreat is unavailable.
23. **Uses Tate & Liza switch mode when it completes the Active axis.** Initial: `T2; A Tapu Lele-GX[-]; B Regidrago VSTAR[GGF]; H Tate & Liza; X Mega Dragonite ex; DTT Mega Dragonite ex`. Pass: switch mode promotes VSTAR.
24. **Strict JIT requires the payload in discard now and from this turn.** Initial A: `T3; A Regidrago VSTAR[GGF]; X Mega Dragonite ex; DTT empty`. Pass A: not ready. Initial B adds `DTT Mega Dragonite ex`. Pass B: payload is valid. Then the fixture recovers Mega Dragonite ex to hand. Pass C: payload is no longer valid. No-control accepts the prior-turn discard state.
25. **Does not burn Serena after all setup axes are complete.** Initial: `T3; A Regidrago VSTAR[GGF]; H Serena, Dipplin; X Mega Dragonite ex; DTT Mega Dragonite ex`. Pass: Serena remains in hand and the Supporter slot remains unused.
26. **Uses Arven's Tool-only route through current Item lock.** Initial: `T2; A Regidrago V[-]; H Arven; D Evolution Incense, Forest Seal Stone, Regidrago VSTAR; full Item lock`. Pass: Arven is played as the Supporter, does not take Evolution Incense, and takes Forest Seal Stone as its Pokémon Tool target. Forest Seal Stone then attaches to Regidrago V and Star Alchemy finds VSTAR. Arven's printed search supports an Item card and a Pokémon Tool card: https://api.pokemontcg.io/v2/cards/sv1-166. Pokémon Tools are separate from Item cards after the official Tool errata: https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata. Forest Seal Stone supplies Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156.
27. **Enforces first-turn restrictions and Celestial Roar's cost.** Initial A: `T1 going first; A Regidrago V[G]; H Arven`. Pass A: no Supporter and no attack. Initial B: `T1 going second; A Regidrago V[-]; D Grass, Fire, Grass`. Pass B: Celestial Roar fails for no Energy. Initial C: same, except `A Regidrago V[G]`. Pass C: Celestial Roar discards the three cards and attaches both Grass plus Fire.
28. **Enforces evolution timing and one manual attachment.** Initial A: `T2; A Regidrago V[-] entered T2; H VSTAR, Grass, Fire`. Pass A: evolution fails. Initial B: same but Regidrago entered T1. Pass B: evolution succeeds; one manual attachment succeeds; the second fails.
29. **Full Item lock prevents Item cards.** Initial: `T2; A Regidrago V[-]; H Earthen Vessel, Dipplin; D Grass, Fire; full Item lock`. Pass: Earthen Vessel cannot be played.
30. **K1 Gladion retrieves final prized Fire rather than using dead Crispin.** Initial: `T2; A Regidrago V[GG]; H Mysterious Treasure, Dipplin, Gladion, Crispin; D VSTAR; P Fire; X/DTT Mega Dragonite ex`. Pass: Treasure legally takes VSTAR and establishes K1; Gladion takes Fire because no Fire remains in deck; manual attachment completes GGF.
31. **Heavy Ball prize revelation also marks dead Crispin.** Initial: `T2; A Regidrago V[GG]; H Heavy Ball, Crispin, Gladion; D VSTAR; P Fire; X/DTT Mega Dragonite ex`. Pass: Heavy Ball reveals the non-Basic Prize set and is discarded; fixed-list deduction proves Fire absent from deck; Gladion takes Fire rather than spending Crispin.
32. **Prized Energy does not preempt the Item-lock payload bridge.** Initial: `T2; A Regidrago V[GG]; H Heavy Ball, Gladion, Professor Burnet, VSTAR; D Mega Dragonite ex; P Fire; full Item lock`. Pass: Heavy Ball reveals Fire; Burnet is selected and discards Mega Dragonite ex; Gladion stays in hand because Fire alone cannot create readiness.
33. **Celestial Roar transfers Energy out of discard.** Initial: `T1 going second; A Regidrago V[G]; D Dipplin, Fire, Grass`. Pass: the top-three Grass and Fire become attached, neither remains in discard, and only Dipplin remains there.
34. **Tate & Liza draws from a four-card dead hand.** Initial: `T2; A Regidrago V[GGF]; H Tate & Liza, Dipplin, Mawile-GX, Guzma; D VSTAR; X/DTT Mega Dragonite ex`. Pass: no direct Supporter line exists; Tate shuffles the three remaining cards and draws five, finding VSTAR.
35. **Star Alchemy establishes K1 before final-Energy Gladion choice.** Initial: `T2; A Regidrago V[GG] with Forest Seal Stone; H VSTAR; D Gladion; P Fire; X/DTT Mega Dragonite ex`. Pass: Star Alchemy records legal deck knowledge, fetches Gladion, and Gladion takes Fire.

## Test command

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

`regidrago_policy_fixtures` runs all 35 exact-state cases. The existing six seeded trace regressions and the aggregate smoke test remain separate CTest targets.
