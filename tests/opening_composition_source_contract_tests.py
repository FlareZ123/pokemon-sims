from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
LEGACY_STAGE = ROOT / "src" / "trace_engine_v2" / "composition" / "opening_legacy_stage.inc"
COMPLETION_STAGE = ROOT / "src" / "trace_engine_v2" / "composition" / "opening_state_completion_stage.inc"


def test_opening_stage_owns_member_includes_directly() -> None:
    source = LEGACY_STAGE.read_text(encoding="utf-8")

    member_wrapper = """#define REGIDRAGO_ENGINE_MEMBER_WRAPPER
#include \"core/opening/opening_member_impl.inc\"
#undef REGIDRAGO_ENGINE_MEMBER_WRAPPER"""
    member_scope = """#define REGIDRAGO_ENGINE_MEMBER_SCOPE
#include \"core/opening/opening_state_ability_resolution.inc\"
#undef REGIDRAGO_ENGINE_MEMBER_SCOPE"""

    assert member_wrapper in source
    assert member_scope in source
    assert '#include "composition/opening_state_completion_stage.inc"' not in source


def test_forwarding_completion_stage_stays_removed() -> None:
    assert not COMPLETION_STAGE.exists()
