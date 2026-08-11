from __future__ import annotations

import json
import os
import subprocess
import tempfile
from pathlib import Path

from baseline_provenance import simulator_policy_source_digest

ROOT = Path(__file__).resolve().parents[1]
PART = ROOT / "src" / "trace_engine_v2" / "part_016.inc"
MODULE = ROOT / "src" / "trace_engine_v2" / "cli" / "crobat_modeling.inc"
BASELINE_MANIFEST = ROOT / "results" / "baseline_manifest.json"
MULTI_MANIFEST = ROOT / "results" / "multi_deck_manifest.json"
WORKFLOW = ROOT / ".github" / "workflows" / "materialize-cleanup-crobat.yml"
SELF = Path(__file__).resolve()


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary_name, path)
    finally:
        if os.path.exists(temporary_name):
            os.unlink(temporary_name)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if text.count(old) != 1:
        raise RuntimeError(f"Expected exactly one cleanup target in {path}: {old[:80]!r}")
    atomic_write(path, text.replace(old, new, 1))


def git(*args: str) -> None:
    subprocess.run(["git", *args], cwd=ROOT, check=True)


def commit(message: str, *paths: Path) -> None:
    git("add", *(str(path.relative_to(ROOT)) for path in paths))
    git("commit", "-m", message)


def extract_crobat_module() -> None:
    text = PART.read_text(encoding="utf-8")
    start_marker = "namespace sim {\n\nstruct CrobatModelingDeck {"
    end_marker = "\n}  // namespace sim\n\n\n#ifndef REGIDRAGO_SIM_NO_MAIN"
    start = text.find(start_marker)
    end = text.find(end_marker, start)
    if start < 0 or end < 0:
        raise RuntimeError("Crobat modeling block boundaries were not found")
    block_end = end + len("\n}  // namespace sim")
    block = text[start:block_end]
    module_text = (
        "#pragma once\n\n"
        "// Crobat modeling is a CLI-only experimental surface kept outside the main\n"
        "// translation-unit tail so part_016 remains focused on CLI parsing/dispatch.\n"
        "// Modeling contract: https://github.com/FlareZ123/pokemon-sims/issues/1394\n\n"
        + block
        + "\n"
    )
    atomic_write(MODULE, module_text)
    replacement = (
        "// Crobat modeling implementation is composed from its dedicated CLI module.\n"
        "// Modeling contract: https://github.com/FlareZ123/pokemon-sims/issues/1394\n"
        "#include \"cli/crobat_modeling.inc\""
    )
    atomic_write(PART, text[:start] + replacement + text[block_end:])
    commit("Cleanup: extract Crobat modeling module", PART, MODULE)


def centralize_crobat_copy_count() -> None:
    insertion = """int crobat_copy_count(const DeckRecipe& recipe) {
  return std::accumulate(recipe.begin(), recipe.end(), 0,
                         [](const int total, const auto& entry) {
                           return total +
                               (entry.first == Card::CrobatV ? entry.second : 0);
                         });
}

void write_crobat_modeling_matrix("""
    replace_once(MODULE, "void write_crobat_modeling_matrix(", insertion)
    old = """          << std::accumulate(deck.recipe.begin(), deck.recipe.end(), 0,
               [](const int total, const auto& entry) {
                 return total + (entry.first == Card::CrobatV ? entry.second : 0);
               }) << ','"""
    replace_once(MODULE, old, "          << crobat_copy_count(deck.recipe) << ','")
    commit("Cleanup: centralize Crobat copy counting", MODULE)


def centralize_seed_slot() -> None:
    insertion = """constexpr std::size_t crobat_model_seed_slot(const std::size_t scenario_index) {
  // Historical slots 4 and 11 belonged to retired full-turn-one Item-lock rows.
  // Preserve the established random stream for every surviving scenario:
  // https://github.com/FlareZ123/pokemon-sims/issues/1118
  return scenario_index + (scenario_index >= 4 ? 1U : 0U) +
         (scenario_index >= 10 ? 1U : 0U);
}

void write_crobat_modeling_matrix("""
    replace_once(MODULE, "void write_crobat_modeling_matrix(", insertion)
    old = """    // Historical slots 4 and 11 belonged to the retired full-turn-one Item-lock
    // rows. Keep every surviving scenario on its old random stream:
    // https://github.com/FlareZ123/pokemon-sims/issues/1118
    const std::size_t seed_slot = scenario_index +
        (scenario_index >= 4 ? 1U : 0U) +
        (scenario_index >= 10 ? 1U : 0U);"""
    replace_once(
        MODULE,
        old,
        "    const std::size_t seed_slot = crobat_model_seed_slot(scenario_index);",
    )
    commit("Cleanup: centralize Crobat scenario seed slots", MODULE)


def refresh_provenance() -> None:
    digest = simulator_policy_source_digest(ROOT)
    for path in (BASELINE_MANIFEST, MULTI_MANIFEST):
        manifest = json.loads(path.read_text(encoding="utf-8"))
        manifest["simulator_policy_source_sha256"] = digest
        atomic_write(path, json.dumps(manifest, indent=2) + "\n")
    subprocess.run(
        ["python", "scripts/update_multi_deck_docs.py", "--repo-root", str(ROOT)],
        cwd=ROOT,
        check=True,
    )
    SELF.unlink()
    WORKFLOW.unlink()
    commit(
        "Cleanup: refresh source-bound cleanup evidence",
        BASELINE_MANIFEST,
        MULTI_MANIFEST,
        ROOT / "docs" / "MULTI_DECK_REPORT.md",
        SELF,
        WORKFLOW,
    )


def main() -> None:
    git("config", "user.name", "github-actions[bot]")
    git("config", "user.email", "41898282+github-actions[bot]@users.noreply.github.com")
    extract_crobat_module()
    centralize_crobat_copy_count()
    centralize_seed_slot()
    refresh_provenance()


if __name__ == "__main__":
    main()
