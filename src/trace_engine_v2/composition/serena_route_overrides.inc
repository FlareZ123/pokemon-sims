#pragma once

// Keep the late Serena Quick Ball route behind a named composition boundary so
// regidrago_sim.cpp does not carry another issue-specific implementation include.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179

#include "../part_issue_1683_serena_dead_quick_ball_override.inc"
