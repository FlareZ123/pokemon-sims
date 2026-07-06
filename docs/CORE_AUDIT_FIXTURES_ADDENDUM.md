# Core Audit Fixture Addendum

This addendum extends `OPTIMAL_POLICY_FIXTURES.md` from 35 to **39** core exact-state fixtures.

36. **Steven's Resolve reserves Gladion after K1 reveals a prized VSTAR.** Initial: `T1 going second; A Regidrago V[-]; H Steven's Resolve; D Gladion, Crispin, Brilliant Blender; P Regidrago VSTAR`. Pass: Steven inspects deck, takes Gladion, Crispin, and Blender, then ends the turn. It does not waste a search slot trying to take the prized VSTAR.

37. **A full Bench blocks Basic search connectors.** Initial: `T2; A Regidrago V[GGF]; B Regidrago VSTAR[GGF], Tapu Lele-GX, Oricorio, Mawile-GX, Dialga-GX; H Quick Ball, Mysterious Treasure, Dipplin; X/DTT Mega Dragonite ex`. Pass: Quick Ball and Mysterious Treasure remain in hand because Latias ex cannot be benched.

38. **Strict Serena can use all three legal safe discards.** Initial: `T2; A Regidrago V[GGF]; H Serena, Dipplin, Grass, Grass, Fire, Fire; D VSTAR, Mawile-GX, Guzma; X/DTT Mega Dragonite ex`. Pass: Serena discards Dipplin plus one surplus Grass and one surplus Fire, then draws to five. The printed minimum of one discard and maximum of three is respected.

39. **Heavy Ball chooses Oricorio over Tapu when it preserves Burnet under Item lock.** Initial: `T2; A Regidrago VSTAR[GG]; H Hisuian Heavy Ball, Professor Burnet; D Fire, Mega Dragonite ex, Crispin, Grass; P Tapu Lele-GX, Oricorio; full Item lock`. Pass: Heavy Ball retrieves Oricorio, Vital Dance finds Fire, manual attachment completes GGF, and Burnet remains available to discard Mega Dragonite ex in the same turn.

The invalid interim zero-discard Serena line is documented and superseded in `AUDIT_CORRECTIONS.md`.
