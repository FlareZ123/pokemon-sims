#pragma once

// Compose the Legacy Star projection and payload bridge wrappers in one place.
// Each temporary alias exposes the preceding implementation to the next layer.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#if defined(remove_one) || defined(pokemon_communication_plan) || defined(use_legacy_star)
#error "Legacy Star projection chain requires wrapper aliases to be undefined on entry"
#endif

#include "../part_legacy_star_projection_provenance_override.inc"
#define remove_one remove_one_legacy_star_projection
#define pokemon_communication_plan pokemon_communication_plan_legacy_star_projection
#define use_legacy_star use_legacy_star_issue1016_original
#include "../part_013_legacy_star_override.inc"
#undef use_legacy_star
#undef pokemon_communication_plan
#undef remove_one

#define use_legacy_star use_legacy_star_issue1069_original
#include "../part_issue_1016_legacy_star_quick_ball_override.inc"
#undef use_legacy_star
#include "../part_issue_1069_legacy_star_combined_energy_payload_override.inc"

#if defined(remove_one) || defined(pokemon_communication_plan) || defined(use_legacy_star)
#error "Legacy Star projection chain leaked a wrapper alias"
#endif
