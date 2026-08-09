from __future__ import annotations

import re
import unicodedata
from pathlib import Path, PurePosixPath
from urllib.parse import unquote


ROOT = Path(__file__).resolve().parents[1]
README_PATH = ROOT / "README.md"
CROBAT_REPORT_PATH = ROOT / "docs" / "CROBAT_MODEL_REPORT.md"
SCAN_ROOTS = ("src", "tests", "scripts", "docs")
ROOT_FILES = ("README.md", "CMakeLists.txt")
TEXT_SUFFIXES = {
    ".c", ".cc", ".cmake", ".cpp", ".h", ".hpp", ".inc", ".json",
    ".md", ".py", ".txt", ".yaml", ".yml",
}
# GitHub section-anchor rules: https://docs.github.com/en/enterprise-cloud@latest/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax#section-links
# Confirmed repository traceability defect: https://github.com/FlareZ123/pokemon-sims/issues/1542
INTERNAL_ANCHOR_RE = re.compile(
    r"https://github\.com/FlareZ123/pokemon-sims/blob/main/"
    r"(?P<path>[^#\s)>\"']+\.md)#(?P<anchor>[A-Za-z0-9_~%\-]+)"
)
LINE_ANCHOR_RE = re.compile(r"L\d+(?:-L\d+)?\Z")
HEADING_RE = re.compile(r"^#{1,6}\s+(.+?)\s*#*\s*$")
CUSTOM_ANCHOR_RE = re.compile(
    r"<a\s+(?:name|id)=[\"'](?P<anchor>[^\"']+)[\"'][^>]*>",
    re.IGNORECASE,
)
CROBAT_SCOPE_RE = re.compile(
    r"exactly (?P<variants>\d+) current variants and (?P<scenarios>\d+) registered "
    r"aggregate scenarios, for (?P<conditions>\d+) conditions\. At the canonical "
    r"(?P<trials>[\d,]+) trials per condition, that is "
    r"(?P<millions>\d+(?:\.\d+)?) million simulated games\."
)
CROBAT_FINAL_VALIDATION_RUNS = ("31164362259", "31164362295")


def candidate_files() -> list[Path]:
    paths = [ROOT / name for name in ROOT_FILES]
    for root_name in SCAN_ROOTS:
        root = ROOT / root_name
        paths.extend(
            path
            for path in root.rglob("*")
            if path.is_file() and path.suffix.lower() in TEXT_SUFFIXES
        )
    return sorted(set(paths))


def visible_heading_text(text: str) -> str:
    text = re.sub(r"!\[([^\]]*)\]\([^)]*\)", r"\1", text)
    text = re.sub(r"\[([^\]]+)\]\([^)]*\)", r"\1", text)
    text = re.sub(r"<[^>]+>", "", text)
    return re.sub(r"[`*_~]", "", text)


def github_slug(text: str) -> str:
    # GitHub lowercases letters, turns spaces into hyphens, removes other
    # whitespace and punctuation, and preserves duplicate suffixes per file:
    # https://docs.github.com/en/enterprise-cloud@latest/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax#section-links
    pieces: list[str] = []
    for character in visible_heading_text(text).strip().lower():
        if character == " ":
            pieces.append("-")
        elif character.isspace():
            continue
        elif unicodedata.category(character).startswith("P") and character != "-":
            continue
        else:
            pieces.append(character)
    return "".join(pieces).strip("-")


def markdown_anchors(path: Path) -> set[str]:
    text = path.read_text(encoding="utf-8")
    anchors = {match.group("anchor") for match in CUSTOM_ANCHOR_RE.finditer(text)}
    seen: dict[str, int] = {}
    for line in text.splitlines():
        match = HEADING_RE.match(line)
        if not match:
            continue
        base = github_slug(match.group(1))
        if not base:
            continue
        duplicate_index = seen.get(base, 0)
        anchors.add(base if duplicate_index == 0 else f"{base}-{duplicate_index}")
        seen[base] = duplicate_index + 1
    return anchors


def resolve_markdown_target(
    source_path: Path, relative_target: PurePosixPath, errors: list[str]
) -> Path | None:
    target = ROOT.joinpath(*relative_target.parts).resolve()
    try:
        target.relative_to(ROOT.resolve())
    except ValueError:
        errors.append(
            f"{source_path.relative_to(ROOT)}: target escapes repository: "
            f"{relative_target}"
        )
        return None
    if not target.is_file():
        errors.append(
            f"{source_path.relative_to(ROOT)}: missing Markdown target "
            f"{relative_target}"
        )
        return None
    return target


def validate_internal_anchors(errors: list[str]) -> int:
    anchors_by_target: dict[Path, set[str]] = {}
    checked = 0

    for source_path in candidate_files():
        try:
            source = source_path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        for match in INTERNAL_ANCHOR_RE.finditer(source):
            anchor = unquote(match.group("anchor"))
            if LINE_ANCHOR_RE.fullmatch(anchor):
                continue
            checked += 1
            relative_target = PurePosixPath(unquote(match.group("path")))
            target = resolve_markdown_target(source_path, relative_target, errors)
            if target is None:
                continue
            anchors = anchors_by_target.setdefault(target, markdown_anchors(target))
            if anchor not in anchors:
                errors.append(
                    f"{source_path.relative_to(ROOT)}: missing #{anchor} in "
                    f"{relative_target}"
                )

    return checked


def validate_crobat_readme_provenance(errors: list[str]) -> None:
    readme = README_PATH.read_text(encoding="utf-8")
    report = CROBAT_REPORT_PATH.read_text(encoding="utf-8")
    scope = CROBAT_SCOPE_RE.search(report)
    if scope is None:
        errors.append("docs/CROBAT_MODEL_REPORT.md: source-bound Crobat scope is missing")
        return

    variants = int(scope.group("variants"))
    scenarios = int(scope.group("scenarios"))
    conditions = int(scope.group("conditions"))
    trials = int(scope.group("trials").replace(",", ""))
    games = variants * scenarios * trials
    reported_millions = float(scope.group("millions"))
    if variants * scenarios != conditions or games / 1_000_000 != reported_millions:
        errors.append("docs/CROBAT_MODEL_REPORT.md: Crobat scope arithmetic is inconsistent")
        return

    expected_summary = f"the {reported_millions:g}-million-game Crobat matrix"
    if expected_summary not in readme:
        errors.append(
            "README.md: Crobat validation count does not match "
            "docs/CROBAT_MODEL_REPORT.md"
        )
    for run_id in CROBAT_FINAL_VALIDATION_RUNS:
        if f"https://github.com/FlareZ123/pokemon-sims/actions/runs/{run_id}" not in readme:
            errors.append(
                f"README.md: missing final Crobat validation run {run_id} from #2253"
            )

    # The report is source-bound by #2253; this README summary must follow that
    # same 13 x 14 inventory and final current-main evidence instead of reviving
    # pre-binding metadata: https://github.com/FlareZ123/pokemon-sims/issues/2253#issuecomment-5215306149
    # Confirmed documentation/provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2279
    if "the 20.8-million-game Crobat matrix" in readme:
        errors.append("README.md: stale 20.8-million-game Crobat summary remains")


def main() -> int:
    errors: list[str] = []
    checked = validate_internal_anchors(errors)
    validate_crobat_readme_provenance(errors)

    if checked == 0:
        errors.append("no internal blob/main Markdown section links were found")
    if errors:
        print("Internal Markdown source-link contract failed:")
        for error in errors:
            print(f"- {error}")
        return 1

    print(f"Validated {checked} internal Markdown section links.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
