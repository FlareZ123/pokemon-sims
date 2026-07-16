# Turbo setup optimization search

This branch evaluates setup-speed swaps with matched seeds and explicit legality filters.

## Search stages

1. Export and build the exact pull-request merge ref.
2. Generate the current 100,000-trial baseline.
3. Measure removal sensitivity for every current deck card.
4. Build a PTCG Live Expanded candidate set from the supplied card corpus.
5. Screen legal one-card swaps with matched seeds.
6. Retest survivors at higher trial counts.
7. Search two-card and three-card combinations around the strongest connectors.
8. Report T2 and T3 probabilities for every modeled lock and turn-order condition.

The objective follows the repository's earliest-ready policy:
https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scope

Candidate graph costs include Supporter use, discard requirements, one-use connectors, Bench use, Item and Ability locks, and Prize collapse:
https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
