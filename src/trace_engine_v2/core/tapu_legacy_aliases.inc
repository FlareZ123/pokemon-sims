#pragma once

// Preserve the legacy Crobat selector under a private name so the confirmed
// Forest Seal Stone hand-compression policy can replace it after part_006.inc:
// https://api.pokemontcg.io/v2/cards/swsh12-156
// https://api.pokemontcg.io/v2/cards/swsh3-104
// https://github.com/FlareZ123/pokemon-sims/issues/1726
#define bench_crobat_if_useful bench_crobat_if_useful_issue_1726_original

// Preserve the completed #1605 implementation as the historical baseline while
// #2645 installs the state-driven route after part_006.inc is parsed:
// https://github.com/FlareZ123/pokemon-sims/issues/1605
// https://github.com/FlareZ123/pokemon-sims/issues/2645
#define issue_1605_arven_crobat_route_available \
  issue_1605_arven_crobat_route_available_issue2645_original
