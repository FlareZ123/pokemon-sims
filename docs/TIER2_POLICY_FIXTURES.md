# Tier Two Policy Fixture Catalog

## Purpose

These deterministic fixtures test choice differentiation and fast-compression. Each starts from an exact state and calls the same engine policy used by a trace run. `T` is the player turn; `A` is Active; `B` is Bench; `H` is hand; `D` is deck; `P` is Prizes; `X` is discard; `DTT` is the cards discarded this turn. `G` and `F` are Energy attached to a Pokémon. Omitted zones are empty.

The goal is the earliest legal modeled ready state: Active Regidrago VSTAR with `GGF` and a permitted payload in discard under the selected DCI profile. These fixtures constrain modeled policy. They are not a proof against every deck order or opponent action.

## Choice-differentiation and compression fixtures

1. **Crispin beats Arven.** `T2; A Regidrago V[GG]; H Crispin, Arven, VSTAR; D Grass, Fire, Earthen Vessel, FSS`. Required: play Crispin, attach Fire, preserve Arven, then allow the previously played Regidrago V to evolve.
2. **Steven's Resolve beats Arven on turn one.** `T1 going second; A Regidrago V[-]; H Steven's Resolve, Arven; D VSTAR, Crispin, Brilliant Blender, FSS`. Required: use Steven, take VSTAR plus Crispin plus Blender, preserve Arven, end turn.
3. **A post-search Gladion beats slower Supporters for a prized VSTAR.** `T2; A Regidrago V[GGF]; H Mysterious Treasure, Dipplin, Gladion, Steven, Arven; D Tapu Lele-GX; P VSTAR; X/DTT Mega Dragonite ex`. Required: Treasure creates K1, then Gladion takes VSTAR.
4. **K0 does not peek at Prizes.** `T2; A Regidrago V[GG]; H VSTAR, Crispin, Gladion, Arven; D Grass, Fire, Earthen Vessel; P Blender`. Required: Crispin supplies Fire; unseen Blender does not make the policy spend Gladion.
5. **Blind Gladion is critical-only.** `T2; A Regidrago V[GGF]; H Gladion; P VSTAR, Grass, Fire, Dipplin, Mawile-GX, Guzma; X/DTT Mega Dragonite ex`. Required: Gladion is used because VSTAR is the sole remaining route and reveals the Prize state.
6. **FSS chooses Oricorio compression over one Energy.** `T2; A Regidrago V[-] with FSS; D Oricorio, Grass, Fire`. Required: Star Alchemy gets Oricorio; Vital Dance gets both Energy cards.
7. **FSS chooses Vessel over Arven.** `T2; A Regidrago V[G] with FSS; H Dipplin; D Earthen Vessel, Arven, Grass, Fire`. Required: Star Alchemy gets Earthen Vessel, not Arven; Vessel is immediately usable.
8. **FSS takes direct Arven instead of a Tapu chain.** Exact constructed state in the core fixture has a live Arven and an available Tapu route. Required: FSS gets Arven directly.
9. **FSS takes Blender for current-turn payload.** Exact constructed state has ready VSTAR, a payload still missing, and Blender in deck. Required: FSS gets Blender, then Blender makes the legal JIT discard.
10. **Known Lusamine is delayed, not direct.** Exact constructed state has a recoverable Supporter but a live direct connector. Required: retain Lusamine as delayed recovery and use the current direct line.
11. **Search knowledge persists.** A legal deck search is resolved, then subsequent choices can use the deduced Prize multiset. Required: K1 remains true for later choices.
12. **Zero-target Crispin still establishes K1.** `T2; A Regidrago V[GF]; H Crispin; P VSTAR, Grass, Fire; D empty`. Required: Crispin resolves legally, finds no Energy, and establishes deck/Prize knowledge.
13. **Singleton Crispin preserves manual attachment.** `T2; A Regidrago V[G]; H Mysterious Treasure, Mega Dragonite ex, Crispin, VSTAR; D Tapu Lele-GX, Fire`. Required: Treasure establishes K1; Crispin puts Fire in hand without attaching; manual attachment then supplies Fire.
14. **Tapu does not fetch redundant Arven.** `T2; A Regidrago V[GGF]; H Tapu Lele-GX, VSTAR; X/DTT Mega Dragonite ex`. Required: Tapu stays in hand because the VSTAR card axis is already complete.
15. **Wonder Tag finds Gladion after it sees a prized VSTAR.** `T2; A Regidrago V[GGF]; H Tapu Lele-GX, Arven, Crispin; D Gladion, Arven, Crispin; P VSTAR; X/DTT Mega Dragonite ex`. Required: Bench Tapu, get Gladion through Wonder Tag, then Gladion takes VSTAR and preserves Arven/Crispin.
16. **Crispin beats known prized Oricorio.** `T2; A Regidrago V[GG]; H Mysterious Treasure, Mega Dragonite ex, Gladion, Crispin, VSTAR; D Tapu Lele-GX, Grass, Fire; P Oricorio`. Required: Treasure establishes K1; Crispin attaches Fire now; Gladion remains unused.
17. **FSS holds for Tate & Liza switch.** `T2; A Regidrago V[-] with FSS; B VSTAR[GGF]; H Tate & Liza; X/DTT Mega Dragonite ex`. Required: preserve FSS because Tate & Liza alone fixes Active position.
18. **FSS finds Gladion for a known prized VSTAR.** `T2; A Regidrago V[GGF] with FSS; D Gladion, Arven, Crispin; P VSTAR; X/DTT Mega Dragonite ex`. Required: FSS gets Gladion; Gladion gets VSTAR.
19. **Heavy Ball retrieves prized Oricorio for two-Energy compression.** `T2; A Regidrago V[-]; H Hisuian Heavy Ball; P Oricorio; D Grass, Fire`. Required: Heavy Ball gets Oricorio; Vital Dance gets Grass plus Fire.
20. **Rule Box lock blocks Latias Quick Ball line.** `T2; A Regidrago V[-]; B VSTAR[GGF]; H Quick Ball, Dipplin; X/DTT Mega Dragonite ex; full Rule Box Ability lock`. Required: hold Quick Ball because Latias Skyliner cannot function.
21. **FSS gets Tate & Liza when Latias is locked.** `T2; A Regidrago V[-] with FSS; B VSTAR[GGF]; D Tate & Liza; X/DTT Mega Dragonite ex; full Rule Box Ability lock`. Required: FSS gets Tate & Liza; switch mode promotes VSTAR.
22. **K1 Heavy Ball beats an otherwise live Quick Ball.** `T2; A Regidrago V[-]; H Mysterious Treasure, Mega Dragonite ex, VSTAR, Heavy Ball, Quick Ball; D Tapu Lele-GX, Grass, Fire; P Oricorio`. Required: Treasure creates K1; Heavy Ball now takes known prized Oricorio despite Quick Ball in hand.
23. **Tate & Liza draw mode recovers a dead hand.** `T2; A Regidrago V[GGF]; H Tate & Liza; D VSTAR; X/DTT Mega Dragonite ex`. Required: use draw mode after direct Supporters fail; shuffle the small hand and draw VSTAR.
24. **Arven takes an available same-axis Item fallback.** `T2; A Regidrago V[GGF]; H Arven, Dipplin; D Mysterious Treasure, FSS, VSTAR; X/DTT Mega Dragonite ex`. Required: Evolution Incense is absent, so Arven gets Mysterious Treasure and FSS rather than no Item.
25. **FSS holds for live Crispin.** `T2; A Regidrago V[GG] with FSS; H VSTAR, Crispin; D Grass, Fire, Oricorio; X/DTT Mega Dragonite ex`. Required: FSS remains unused; Crispin attaches Fire directly.
26. **Ultra Ball holds for live Crispin.** `T2; A Regidrago V[GG]; H VSTAR, Ultra Ball, Crispin, Dipplin, Grass, Grass; D Oricorio, Grass, Fire; X/DTT Mega Dragonite ex`. Required: do not discard two cards to fetch Oricorio; preserve Ultra Ball and use Crispin to attach Fire.

## Run

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/regidrago_tier2_tests
```
