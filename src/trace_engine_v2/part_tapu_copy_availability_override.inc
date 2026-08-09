#include "core/tapu_connector_policy.inc"
// Preserve the legacy Crobat selector under a private name so the confirmed
// Forest Seal Stone hand-compression policy can replace it after part_006.inc:
// https://api.pokemontcg.io/v2/cards/swsh12-156
// https://api.pokemontcg.io/v2/cards/swsh3-104
// https://github.com/FlareZ123/pokemon-sims/issues/1726
#define bench_crobat_if_useful bench_crobat_if_useful_issue_1726_original
