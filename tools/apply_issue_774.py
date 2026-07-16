from pathlib import Path

path = Path("src/regidrago_sim.cpp")
text = path.read_text(encoding="utf-8")
old = '''#define fss_target_after_search_started fss_target_after_search_started_original
#define attach_fss attach_fss_original
#include "trace_engine_v2/part_010.inc"
#undef attach_fss
#undef fss_target_after_search_started
'''
new = '''#define fss_target_after_search_started fss_target_after_search_started_original
#define attach_fss attach_fss_original
#define should_play_steven should_play_steven_original
#include "trace_engine_v2/part_010.inc"
#undef should_play_steven
#undef attach_fss
#undef fss_target_after_search_started
#include "trace_engine_v2/part_010_steven_crispin_override.inc"
'''
if 'part_010_steven_crispin_override.inc' not in text:
    if old not in text:
        raise SystemExit("part_010 composition anchor missing")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")
