// Guzma & Hala can itself pay the current-turn Dragon discard while compressing
// the Stadium, Tool, and Special Energy axes. Give a fully admitted route the
// Supporter slot before the generic paid-search block spends those same resources.
// The policy function retains its own Supporter, DCI/JIT, and multi-axis gates, so
// decks without Guzma & Hala and incomplete single-axis routes fall through here.
// Guzma & Hala: https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/
// Earliest/compression policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
// Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3386
if (!state_.supporter_used && setup_axis_missing() && play_guzma_hala()) return true;

#define REGIDRAGO_ISSUE_962_SELECT_ELIGIBILITY
#include "composition/issue_962_route_sections.inc"
#undef REGIDRAGO_ISSUE_962_SELECT_ELIGIBILITY
