// Release the late-turn textual-composition aliases after part_016 closes the
// historical execution/reporting chain. Keep the leak guards beside the undefs.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
#undef scenario_by_label
#undef all_scenarios
#if defined(all_scenarios) || defined(scenario_by_label)
#error "Scenario-registry composition leaked an alias"
#endif
#undef run_turn
#undef play_field_blower
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count)
#error "Late-turn composition leaked an execution alias"
#endif
