// Preserve the full route-conditioned DCI implementation, then layer only the
// confirmed #2271 surplus-Regidrago-V Treasure completion after every existing
// ordinary and route-specific discard choice.
// Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2271
#define choose_discard choose_discard_issue2271_original
#include "part_issue_1436_full_supporter_dci_issue2271_original.inc"
#undef choose_discard
#include "part_issue_2271_surplus_regidrago_v_dci_override.inc"
