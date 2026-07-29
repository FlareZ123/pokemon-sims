# Klara swap report

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1773

Klara source: https://api.pokemontcg.io/v2/cards/swsh6-145

Roseanne's Backup comparison source: https://api.pokemontcg.io/v2/cards/swsh9-148

Eri comparison boundary: https://api.pokemontcg.io/v2/cards/sv5-146

Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

## Scope

Only `regidrago-shell` contained Roseanne's Backup at baseline commit `46860424a452121b0747eb048520c3b82a0604cb`. `regidrago-pineco` contained neither Roseanne's Backup nor Klara and is unchanged. The required comparison therefore contains exactly two matrices: shell before and shell after.

## Reproducibility

- Release build for both matrices
- Fixed matrix seed: `20260705`
- Trials per scenario: `100,000`
- Scenarios per matrix: `16`
- Games per matrix: `1,600,000`
- Before CSV: [`../results/klara_swap_shell_before.csv`](../results/klara_swap_shell_before.csv)
- After CSV: [`../results/klara_swap_shell_after.csv`](../results/klara_swap_shell_after.csv)
- Manifest and hashes: [`../results/klara_swap_manifest.json`](../results/klara_swap_manifest.json)

## Before matrix: Roseanne’s Backup

| Scenario | T2 | T3 | T4 | T5 | Failure |
|---|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.933% | 38.500% | 55.367% | 66.078% | 44.633% |
| Matchup-flex JIT, going first | 16.274% | 47.590% | 63.396% | 73.016% | 36.604% |
| No discard control, going first | 20.134% | 56.000% | 72.128% | 81.134% | 27.872% |
| T2 Item lock, going first | 4.596% | 10.206% | 17.699% | 23.115% | 82.301% |
| Full Item lock, going first | 2.823% | 7.749% | 15.061% | 20.616% | 84.939% |
| Rule Box Ability lock, going first | 4.487% | 25.996% | 39.084% | 49.676% | 60.916% |
| Combined lock, going first | 0.293% | 3.279% | 7.268% | 11.095% | 92.732% |
| Strict JIT, going second | 29.492% | 52.693% | 63.936% | 71.848% | 36.064% |
| Matchup-flex JIT, going second | 37.223% | 60.744% | 71.022% | 78.016% | 28.978% |
| No discard control, going second | 39.830% | 66.914% | 77.999% | 84.627% | 22.001% |
| T2 Item lock, going second | 14.085% | 27.964% | 35.542% | 39.747% | 64.458% |
| Full Item lock, going second | 10.556% | 22.936% | 30.105% | 34.319% | 69.895% |
| Rule Box Ability lock, going second | 18.123% | 34.679% | 44.802% | 53.294% | 55.198% |
| Combined lock, going second | 2.373% | 11.415% | 15.519% | 19.083% | 84.481% |
| Supporter lock, going first | 0.002% | 15.385% | 21.677% | 28.412% | 78.323% |
| Supporter lock, going second | 8.181% | 19.626% | 25.447% | 31.621% | 74.553% |

## After matrix: Klara

| Scenario | T2 | T3 | T4 | T5 | Failure |
|---|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.051% | 39.286% | 56.515% | 67.206% | 43.485% |
| Matchup-flex JIT, going first | 16.319% | 47.568% | 63.420% | 73.093% | 36.580% |
| No discard control, going first | 20.013% | 56.018% | 72.215% | 81.214% | 27.785% |
| T2 Item lock, going first | 4.632% | 10.380% | 18.077% | 23.574% | 81.923% |
| Full Item lock, going first | 2.752% | 7.683% | 15.067% | 20.871% | 84.933% |
| Rule Box Ability lock, going first | 4.403% | 26.487% | 39.760% | 50.226% | 60.240% |
| Combined lock, going first | 0.293% | 3.309% | 7.394% | 11.347% | 92.606% |
| Strict JIT, going second | 29.777% | 53.497% | 64.747% | 72.577% | 35.253% |
| Matchup-flex JIT, going second | 37.377% | 61.088% | 71.458% | 78.417% | 28.542% |
| No discard control, going second | 39.944% | 67.090% | 78.237% | 84.787% | 21.763% |
| T2 Item lock, going second | 14.297% | 28.220% | 35.805% | 40.102% | 64.195% |
| Full Item lock, going second | 10.632% | 23.384% | 30.611% | 34.848% | 69.389% |
| Rule Box Ability lock, going second | 18.055% | 35.108% | 45.504% | 53.913% | 54.496% |
| Combined lock, going second | 2.377% | 11.343% | 15.621% | 19.302% | 84.379% |
| Supporter lock, going first | 0.001% | 15.369% | 21.653% | 28.376% | 78.347% |
| Supporter lock, going second | 8.208% | 19.692% | 25.509% | 31.662% | 74.491% |

## Delta: Klara minus Roseanne’s Backup

| Scenario | Δ T2 | Δ T3 | Δ T4 | Δ T5 | Δ Failure |
|---|---:|---:|---:|---:|---:|
| Strict JIT, going first | +0.118 pp | +0.786 pp | +1.148 pp | +1.128 pp | -1.148 pp |
| Matchup-flex JIT, going first | +0.045 pp | -0.022 pp | +0.024 pp | +0.077 pp | -0.024 pp |
| No discard control, going first | -0.121 pp | +0.018 pp | +0.087 pp | +0.080 pp | -0.087 pp |
| T2 Item lock, going first | +0.036 pp | +0.174 pp | +0.378 pp | +0.459 pp | -0.378 pp |
| Full Item lock, going first | -0.071 pp | -0.066 pp | +0.006 pp | +0.255 pp | -0.006 pp |
| Rule Box Ability lock, going first | -0.084 pp | +0.491 pp | +0.676 pp | +0.550 pp | -0.676 pp |
| Combined lock, going first | +0.000 pp | +0.030 pp | +0.126 pp | +0.252 pp | -0.126 pp |
| Strict JIT, going second | +0.285 pp | +0.804 pp | +0.811 pp | +0.729 pp | -0.811 pp |
| Matchup-flex JIT, going second | +0.154 pp | +0.344 pp | +0.436 pp | +0.401 pp | -0.436 pp |
| No discard control, going second | +0.114 pp | +0.176 pp | +0.238 pp | +0.160 pp | -0.238 pp |
| T2 Item lock, going second | +0.212 pp | +0.256 pp | +0.263 pp | +0.355 pp | -0.263 pp |
| Full Item lock, going second | +0.076 pp | +0.448 pp | +0.506 pp | +0.529 pp | -0.506 pp |
| Rule Box Ability lock, going second | -0.068 pp | +0.429 pp | +0.702 pp | +0.619 pp | -0.702 pp |
| Combined lock, going second | +0.004 pp | -0.072 pp | +0.102 pp | +0.219 pp | -0.102 pp |
| Supporter lock, going first | -0.001 pp | -0.016 pp | -0.024 pp | -0.036 pp | +0.024 pp |
| Supporter lock, going second | +0.027 pp | +0.066 pp | +0.062 pp | +0.041 pp | -0.062 pp |

## Aggregate interpretation

Across the 16 conditions, the unweighted average moved by **+0.045 pp at T2**, **+0.240 pp at T3**, **+0.346 pp at T4**, and **+0.364 pp at T5**. Average setup failure moved by **-0.346 pp**.

The final matrix has a positive mean at every readiness horizon. Every remaining negative readiness cell is smaller than one combined standard error. The largest negative standardized movement is below `1.0` combined SE, so none provides evidence of a setup regression at 100,000 trials per condition. Supporter-lock rows are a useful control because Klara cannot be played there; their tiny positive or negative differences are Monte Carlo variation after the two binaries’ continuous RNG streams diverge.

## Negative-delta investigation and correction

The first after-swap matrix was positive overall but exact direct-seed comparison found two genuine strict-JIT losses:

- Seed `301`: Klara recovered Dragapult before Quick Ball could find Tapu Lele-GX and Wonder Tag for Crispin, delaying GGF from T3 to T4.
- Seed `759`: Wonder Tag had already found Tate & Liza, but Klara consumed the Supporter action first and delayed the VSTAR draw route from T3 to T4.

Those were selector bugs, not an inherent Klara tradeoff. The final implementation performs a side-effect-free continuation projection with Klara disabled but still present in hand. When the existing non-Klara connector chain already completes every setup axis this turn, Klara is held. The shared RNG is restored after the projection, so the check cannot alter the real trial. Seeds `301` and `759` are permanent tests and both finish on T3 in the final Release binary. Positive exact seeds such as `243` and `948` still play Klara and finish on T3.

## Modeled strategic value

Klara returns up to two Pokémon and up to two Basic Energy directly from discard to hand. The setup policy preserves the sole current-turn strict-JIT Dragon, can recover an older Dragon only when a legal Item can discard it again that turn, and can use an optional second Pokémon target for discard-control denial without breaking readiness. Because the recovery is a Supporter action rather than an Item, Eri cannot remove Klara from hand as an Item. Team Yell’s Cheer and Lusamine remain modeled recovery-chain partners, while full post-readiness combat and deck-out valuation remain outside this setup simulator.

## Final current-main validation boundary

The merge candidate includes the current `main` selector stack and its generated-output contracts. Final validation also covers temporary-recipe ownership under AddressSanitizer, delegation to existing Professor Turo and Oricorio routes, Klara as the registered issue-1236 discard cost, and the legal earlier Klara alternative in the issue-1109 integration seed. The Release and sanitizer workflows after this commit are the merge gate for those combined behaviors.
