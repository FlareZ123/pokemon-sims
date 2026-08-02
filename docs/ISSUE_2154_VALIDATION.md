# Issue 2154 validation

The Tate & Liza switch into next-turn Professor Burnet route is implemented and tested by PR #2157:
https://github.com/FlareZ123/pokemon-sims/pull/2157

The confirmed bug and approval history are recorded in issue #2154:
https://github.com/FlareZ123/pokemon-sims/issues/2154

The exact-source artifact run regenerated and validated the canonical shell matrix, paired deck matrix, manifests, reports, and representative traces:
https://github.com/FlareZ123/pokemon-sims/actions/runs/30728190555

The independent permanent-CI run built the source in Release, strict C++20, and ASan/UBSan configurations and ran eleven reviewed `--simulate-this` audits before reaching the expected stale-baseline comparison on the pre-refresh commit:
https://github.com/FlareZ123/pokemon-sims/actions/runs/30728190595

The focused route uses these printed effects and repository contracts:

- Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
- Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
- Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Official turn and Trainer procedure: https://www.pokemon.com/us/pokemon-tcg/rules
- K1 and decision priorities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

A separate paid-retreat improvement discovered during trace review is tracked independently and remains approval-gated:
https://github.com/FlareZ123/pokemon-sims/issues/2158
