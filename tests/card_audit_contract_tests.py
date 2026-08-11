from __future__ import annotations

import importlib.util
import re
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DOC_PATH = REPO_ROOT / "docs" / "CARD_AUDIT.md"
RULE_SOURCES_PATH = REPO_ROOT / "docs" / "RULE_SOURCES.md"
SOURCE_WRAPPER_PATH = REPO_ROOT / "src" / "regidrago_sim.cpp"
AUDIT_STATUS_PATH = REPO_ROOT / "docs" / "AUDIT_STATUS.md"
CORE_INDEX_PATH = REPO_ROOT / "docs" / "OPTIMAL_POLICY_FIXTURES.md"
TIER2_INDEX_PATH = REPO_ROOT / "docs" / "TIER2_POLICY_FIXTURES.md"
CORE_RUNNER_PATH = REPO_ROOT / "tests" / "policy_fixture_v2" / "part_004a.inc"
TIER2_RUNNER_PATH = REPO_ROOT / "tests" / "tier2_parts" / "part_003b.inc"
TRACE_REGISTER_PATH = REPO_ROOT / "docs" / "RULES_TRACEABILITY.md"
TRACE_SOURCE_DIR = REPO_ROOT / "src" / "trace_engine_v2"
SOURCE_ROOTS = ("src", "tests", "docs", "scripts", ".github")
CARD_SOURCE_URL = re.compile(r"https://api\.pokemontcg\.io/v2/cards/([^\s<>\"\')\]]+)")
CARD_SOURCE_TRAILING_PUNCTUATION = ".,;:!?`"
# This exact card-ID contract was verified against the supplied English corpus:
# https://github.com/PokemonTCG/pokemon-tcg-data
# Correct Mega Dragonite ex record: https://api.pokemontcg.io/v2/cards/me2pt5-152
# Professor's Letter supplied-corpus record: https://api.pokemontcg.io/v2/cards/xy1-123
# Confirmed source-traceability bug: https://github.com/FlareZ123/pokemon-sims/issues/1696
CANONICAL_SOURCE_CARD_IDS = {
    "base1-99", "me1-117", "me2-87", "me2pt5-16", "me2pt5-152",
    "sm11-141", "sm11-190", "sm12-187", "sm2-55", "sm2-60", "sm2-125",
    "sm3-115", "sm3-128", "sm4-95", "sm4-96", "sm5-100", "sm6-113",
    "sm7-145", "sm7-148", "sm9-152", "sv1-166", "sv3pt5-160",
    "sv4-163", "sv4-171", "sv5-146", "sv4pt5-1", "sv4pt5-2", "sv6-127",
    "sv6-130", "sv6-163", "sv6pt5-63", "sv7-133", "sv8-76",
    "sv8-140", "sv8-164", "swsh1-163", "swsh1-179", "swsh3-104",
    "swsh6-145", "swsh6-148", "swsh7-142",  # Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142 ; https://github.com/FlareZ123/pokemon-sims/issues/2931
    "swsh8-225", "swsh9-148", "swsh9-149", "swsh10-144",
    "swsh10-146", "swsh11-136", "swsh12-135", "swsh12-136",
    "swsh12-156", "swsh12-164", "swsh12pt5-146", "swsh12tg-TG26",
    "xy1-123", "xy9-57",  # Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 ; https://github.com/FlareZ123/pokemon-sims/issues/2931
}
SCRIPT_PATH = REPO_ROOT / "scripts" / "audit_card_data.py"
UPSTREAM_COMMIT_URL = (
    "https://github.com/PokemonTCG/pokemon-tcg-data/commit/"
    "0af6250a22495e4a3e9f60ff45fc3fedc2e0563d"
)
ARCHIVE_SHA256 = "3444c74e47cdb92d83ba760e9eeefa8bbaedd9d7f396068c0e1ed390a686af08"
ERIKA_SOURCE_URL = "https://api.pokemontcg.io/v2/cards/sv3pt5-160"
TRACE_CALL = re.compile(r'\btrace\s*\(\s*"[^"]*"\s*,\s*"([^"]*)"', re.DOTALL)
RULE_ID = re.compile(r"\bR-[A-Z0-9-]+\b")
REGISTERED_RULE_ID = re.compile(r"^\| `(?P<id>R-[A-Z0-9-]+)` \|", re.MULTILINE)


def load_audit_module():
    spec = importlib.util.spec_from_file_location("audit_card_data", SCRIPT_PATH)
    if spec is None or spec.loader is None:
        raise AssertionError("Unable to load scripts/audit_card_data.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def fixture_count(path: Path) -> int:
    source = path.read_text(encoding="utf-8")
    marker = "const std::vector<Test> tests{"
    start = source.find(marker)
    if start < 0:
        raise AssertionError(f"Unable to find fixture runner table in {path}")
    end = source.find("\n  };", start)
    if end < 0:
        raise AssertionError(f"Unable to find fixture runner terminator in {path}")
    table = source[start:end]
    entries = re.findall(r'^\s*\{"[^"]+",\s*[A-Za-z0-9_]+\},?$', table, re.MULTILINE)
    if not entries:
        raise AssertionError(f"Fixture runner table is empty in {path}")
    return len(entries)


def require_documented_count(documented: str, label: str, expected: int) -> None:
    pattern = rf"{re.escape(label)}[^\n]*\*\*{expected}\*\*"
    if re.search(pattern, documented) is None:
        raise AssertionError(f"{label} must document the canonical count {expected}.")


def emitted_rule_ids() -> set[str]:
    emitted: set[str] = set()
    for path in TRACE_SOURCE_DIR.glob("*.inc"):
        for rule_field in TRACE_CALL.findall(path.read_text(encoding="utf-8")):
            emitted.update(RULE_ID.findall(rule_field))
    return emitted


def referenced_card_source_ids() -> dict[str, set[str]]:
    references: dict[str, set[str]] = {}
    for source_root in SOURCE_ROOTS:
        root = REPO_ROOT / source_root
        if not root.exists():
            continue
        for source_path in root.rglob("*"):
            if not source_path.is_file():
                continue
            try:
                source_text = source_path.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                continue
            card_ids = {
                match.group(1).rstrip(CARD_SOURCE_TRAILING_PUNCTUATION)
                for match in CARD_SOURCE_URL.finditer(source_text)
            }
            if card_ids:
                references[str(source_path.relative_to(REPO_ROOT))] = card_ids
    return references


def require_valid_card_source_urls() -> None:
    references = referenced_card_source_ids()
    referenced_ids = set().union(*references.values()) if references else set()
    unknown_ids = sorted(referenced_ids - CANONICAL_SOURCE_CARD_IDS)
    if unknown_ids:
        locations = {
            card_id: sorted(path for path, ids in references.items() if card_id in ids)
            for card_id in unknown_ids
        }
        raise AssertionError(
            "Unknown Pokémon TCG API card IDs: "
            + "; ".join(
                f"{card_id} in {', '.join(locations[card_id])}"
                for card_id in unknown_ids
            )
        )
    stale_ids = sorted(CANONICAL_SOURCE_CARD_IDS - referenced_ids)
    if stale_ids:
        raise AssertionError(
            "Stale canonical Pokémon TCG API card IDs: " + ", ".join(stale_ids)
        )


def main() -> int:
    # Reject nonexistent direct card records before they can be used as executable
    # policy evidence. The contract is corpus-backed and keeps all tracked source
    # URLs independently auditable:
    # https://github.com/PokemonTCG/pokemon-tcg-data
    # https://api.pokemontcg.io/v2/cards/me2pt5-152
    # https://github.com/FlareZ123/pokemon-sims/issues/1696
    require_valid_card_source_urls()

    audit = load_audit_module()
    documented = DOC_PATH.read_text(encoding="utf-8")
    rule_sources = RULE_SOURCES_PATH.read_text(encoding="utf-8")
    source_wrapper = SOURCE_WRAPPER_PATH.read_text(encoding="utf-8")

    # Erika's printed opponent-hand condition is the source for its deliberately inert
    # single-player treatment. Keep that exact print in both declared registries:
    # https://api.pokemontcg.io/v2/cards/sv3pt5-160
    # https://github.com/FlareZ123/pokemon-sims/issues/856
    for registry_name, registry in (("docs/RULE_SOURCES.md", rule_sources),
                                     ("src/regidrago_sim.cpp", source_wrapper)):
        if ERIKA_SOURCE_URL not in registry:
            raise AssertionError(f"{registry_name} must register Erika's Invitation exact print.")

    if sum(copies for _, copies in audit.REQUESTED.values()) != 60:
        raise AssertionError("The audit request no longer represents a 60-card deck.")

    # Steven's Resolve searches for up to three cards. Keep the card-audit table
    # aligned with the supplied print instead of implying a mandatory exact fill:
    # https://api.pokemontcg.io/v2/cards/sm7-145
    # https://github.com/FlareZ123/pokemon-sims/issues/692
    if "Steven’s Resolve | `sm7-145` | 1 | up to three cards, end turn" not in documented:
        raise AssertionError("CARD_AUDIT.md must describe Steven's Resolve as an up-to-three search.")
    if "exact three cards, end turn" in documented:
        raise AssertionError("CARD_AUDIT.md still describes Steven's Resolve as an exact-three search.")

    # The raw JSON is a local reproduction artifact because its source field records the
    # caller-provided path. Documentation must not claim that file is tracked:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/scripts/audit_card_data.py#L135-L166
    if "intentionally untracked" not in documented:
        raise AssertionError("CARD_AUDIT.md must state that data/card_audit.json is untracked.")
    if "The reproducible raw audit is `data/card_audit.json`" in documented:
        raise AssertionError("CARD_AUDIT.md still claims that the raw audit is tracked.")

    # Pin the exact retrievable upstream snapshot and the supplied archive digest so the
    # accepted evidence source cannot silently drift:
    # https://github.com/PokemonTCG/pokemon-tcg-data/commit/0af6250a22495e4a3e9f60ff45fc3fedc2e0563d
    if UPSTREAM_COMMIT_URL not in documented:
        raise AssertionError("CARD_AUDIT.md must pin the accepted upstream commit.")
    if ARCHIVE_SHA256 not in documented:
        raise AssertionError("CARD_AUDIT.md must pin the accepted archive SHA-256.")

    if r"--out data\card_audit.json" not in documented:
        raise AssertionError("CARD_AUDIT.md must retain the local reproduction command.")

    # Keep the status page and both fixture indexes synchronized with the executable
    # runner tables instead of a manually remembered historical count:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/tests/policy_fixture_v2/part_004a.inc#L134-L191
    # https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_003b.inc#L40-L73
    core_count = fixture_count(CORE_RUNNER_PATH)
    tier2_count = fixture_count(TIER2_RUNNER_PATH)
    audit_status = AUDIT_STATUS_PATH.read_text(encoding="utf-8")
    core_index = CORE_INDEX_PATH.read_text(encoding="utf-8")
    tier2_index = TIER2_INDEX_PATH.read_text(encoding="utf-8")

    require_documented_count(audit_status, "Core exact-state policy fixtures:", core_count)
    require_documented_count(audit_status, "Tier Two choice-differentiation fixtures:", tier2_count)
    require_documented_count(core_index, "executes", core_count)
    require_documented_count(tier2_index, "executes", tier2_count)

    # Every production R-* token emitted into a readable trace must have a register
    # row. Guzma's second switch is conditional on its opponent switch occurring, and
    # the repository exposes that prerequisite through an explicit opponent-Bench state:
    # https://api.pokemontcg.io/v2/cards/sm3-115
    # https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#opponent-actions
    # https://github.com/FlareZ123/pokemon-sims/issues/1033
    register = TRACE_REGISTER_PATH.read_text(encoding="utf-8")

    # Steven's Resolve has no turn-one-only clause. Keep the traceability register
    # aligned with the printed effect and the production late-turn route policy:
    # https://api.pokemontcg.io/v2/cards/sm7-145
    # https://www.pokemon.com/us/pokemon-tcg/rules
    # https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010_late_steven_override.inc#L162-L166
    # https://github.com/FlareZ123/pokemon-sims/issues/1181
    if "It is used only going second on turn 1" in register:
        raise AssertionError("R-STEVEN-01 still incorrectly limits Steven's Resolve to turn one.")
    if "On later Supporter-legal turns, the policy also uses source-bounded continuations" not in register:
        raise AssertionError("R-STEVEN-01 must document the supported later-turn Steven routes.")
    if "In turn-two Item-lock scenarios it fetches Burnet rather than Blender" not in register:
        raise AssertionError("R-STEVEN-01 must retain the turn-two Item-lock target rule.")

    emitted = emitted_rule_ids()
    registered = set(REGISTERED_RULE_ID.findall(register))
    missing = sorted(emitted - registered)
    if missing:
        raise AssertionError(f"Unregistered emitted rule IDs: {', '.join(missing)}")
    if "R-GUZMA-01" not in emitted or "R-GUZMA-01" not in registered:
        raise AssertionError("R-GUZMA-01 must be emitted and registered.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
