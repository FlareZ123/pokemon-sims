from __future__ import annotations

import argparse
import os
import tempfile
from pathlib import Path


def atomic_write(path: Path, text: str) -> None:
    fd, temporary = tempfile.mkstemp(prefix=path.name + ".", dir=path.parent, text=True)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary, path)
    except BaseException:
        try:
            os.unlink(temporary)
        except FileNotFoundError:
            pass
        raise


def stage_one(text: str) -> str:
    anchor = "\ndef report_markdown(rows: list[dict[str, str]], fieldnames: list[str], manifest: dict[str, object]) -> str:\n"
    helper = '''\ndef partition_report_rows(\n    rows: list[dict[str, str]],\n    scenario_column: str,\n    t2_column: str,\n    t3_column: str,\n    t4_column: str,\n) -> tuple[list[tuple[str, str, str, str]], list[tuple[str, str, str, str]]]:\n    baseline: list[tuple[str, str, str, str]] = []\n    locks: list[tuple[str, str, str, str]] = []\n    for row in rows:\n        scenario = row[scenario_column]\n        target = locks if "lock" in scenario else baseline\n        target.append(\n            (\n                SCENARIO_LABELS.get(scenario, scenario),\n                row[t2_column],\n                row[t3_column],\n                row[t4_column],\n            )\n        )\n    return baseline, locks\n\n'''
    if "def partition_report_rows(" not in text:
        if text.count(anchor) != 1:
            raise RuntimeError("report_markdown anchor mismatch")
        text = text.replace(anchor, helper + anchor, 1)
    old = '''    baseline = []\n    locks = []\n    for row in rows:\n        scenario = row[scenario_column]\n        target = locks if "lock" in scenario else baseline\n        target.append(\n            (\n                SCENARIO_LABELS.get(scenario, scenario),\n                row[t2_column],\n                row[t3_column],\n                row[t4_column],\n            )\n        )\n\n'''
    new = '''    baseline, locks = partition_report_rows(\n        rows, scenario_column, t2_column, t3_column, t4_column\n    )\n\n'''
    if new not in text:
        if text.count(old) != 1:
            raise RuntimeError("report row partition block mismatch")
        text = text.replace(old, new, 1)
    return text


def stage_two(text: str) -> str:
    anchor = "\ndef main() -> int:\n"
    helper = '''\ndef load_setup_inputs(\n    repo_root: Path,\n) -> tuple[dict[str, object], list[dict[str, str]], list[str]]:\n    manifest_path = repo_root / "results" / "baseline_manifest.json"\n    csv_path = repo_root / "results" / "simulation_results.csv"\n    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))\n    with csv_path.open(newline="", encoding="utf-8") as handle:\n        reader = csv.DictReader(handle)\n        rows = list(reader)\n        fieldnames = list(reader.fieldnames or [])\n    if not rows or not fieldnames:\n        raise ValueError("simulation_results.csv is empty")\n    return manifest, rows, fieldnames\n\n'''
    if "def load_setup_inputs(" not in text:
        if text.count(anchor) != 1:
            raise RuntimeError("main anchor mismatch")
        text = text.replace(anchor, helper + anchor, 1)
    old = '''    manifest_path = repo_root / "results" / "baseline_manifest.json"\n    csv_path = repo_root / "results" / "simulation_results.csv"\n    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))\n    with csv_path.open(newline="", encoding="utf-8") as handle:\n        reader = csv.DictReader(handle)\n        rows = list(reader)\n        fieldnames = list(reader.fieldnames or [])\n    if not rows or not fieldnames:\n        raise ValueError("simulation_results.csv is empty")\n\n'''
    new = '''    manifest, rows, fieldnames = load_setup_inputs(repo_root)\n\n'''
    if new not in text:
        if text.count(old) != 1:
            raise RuntimeError("setup input block mismatch")
        text = text.replace(old, new, 1)
    return text


def stage_three(text: str) -> str:
    anchor = "\ndef main() -> int:\n"
    helper = '''\ndef write_setup_documents(\n    repo_root: Path,\n    rows: list[dict[str, str]],\n    fieldnames: list[str],\n    manifest: dict[str, object],\n) -> None:\n    with exclusive_lock(repo_root / ".update-setup-docs.lock"):\n        atomic_write(\n            repo_root / "docs" / "REPORT.md",\n            report_markdown(rows, fieldnames, manifest),\n        )\n        atomic_write(\n            repo_root / "docs" / "TRACE_AUDIT.md",\n            trace_audit_markdown(repo_root, manifest),\n        )\n        readme_path = repo_root / "README.md"\n        atomic_write(\n            readme_path,\n            update_readme(readme_path.read_text(encoding="utf-8"), manifest),\n        )\n\n'''
    if "def write_setup_documents(" not in text:
        if text.count(anchor) != 1:
            raise RuntimeError("main anchor mismatch")
        text = text.replace(anchor, helper + anchor, 1)
    old = '''    with exclusive_lock(repo_root / ".update-setup-docs.lock"):\n        atomic_write(repo_root / "docs" / "REPORT.md", report_markdown(rows, fieldnames, manifest))\n        atomic_write(repo_root / "docs" / "TRACE_AUDIT.md", trace_audit_markdown(repo_root, manifest))\n        readme_path = repo_root / "README.md"\n        atomic_write(readme_path, update_readme(readme_path.read_text(encoding="utf-8"), manifest))\n'''
    new = '''    write_setup_documents(repo_root, rows, fieldnames, manifest)\n'''
    if new not in text:
        if text.count(old) != 1:
            raise RuntimeError("document write block mismatch")
        text = text.replace(old, new, 1)
    return text


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo-root", type=Path, required=True)
    parser.add_argument("--stage", type=int, choices=(1, 2, 3), required=True)
    args = parser.parse_args()
    path = args.repo_root / "scripts" / "update_setup_docs.py"
    text = path.read_text(encoding="utf-8")
    transform = {1: stage_one, 2: stage_two, 3: stage_three}[args.stage]
    atomic_write(path, transform(text))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
