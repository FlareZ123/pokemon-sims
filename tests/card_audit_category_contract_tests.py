from __future__ import annotations

import copy
import importlib.util
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT_PATH = REPO_ROOT / "scripts" / "audit_card_data.py"
SIM_PLAN_PATH = REPO_ROOT / "SIM-PLAN.md"


def load_audit_module():
    spec = importlib.util.spec_from_file_location("audit_card_data_categories", SCRIPT_PATH)
    if spec is None or spec.loader is None:
        raise AssertionError("Unable to load scripts/audit_card_data.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def minimal_corpus(audit) -> dict[str, dict[str, object]]:
    expected_by_name, _ = audit.canonical_category_contract()
    cards: dict[str, dict[str, object]] = {}
    for name, (card_id, _) in audit.REQUESTED.items():
        if card_id.startswith("energy-"):
            continue
        expected_category = expected_by_name[name]
        cards[card_id] = {
            "id": card_id,
            "name": name,
            "supertype": "Pokémon" if expected_category == "pokemon" else "Trainer",
        }
    return cards


def expect_system_exit(action, expected_message: str) -> None:
    try:
        action()
    except SystemExit as error:
        if expected_message not in str(error):
            raise AssertionError(f"Expected {expected_message!r} in {error!r}") from error
    else:
        raise AssertionError("Expected SystemExit")


def main() -> int:
    audit = load_audit_module()
    cards = minimal_corpus(audit)

    # Category totals must be derived from each supplied record and the explicit
    # Basic Energy entries, then checked against the repository decklist:
    # https://github.com/PokemonTCG/pokemon-tcg-data
    # https://github.com/FlareZ123/pokemon-sims/blob/main/data/decklist.json
    # https://github.com/FlareZ123/pokemon-sims/issues/691
    payload = audit.build_payload(Path("fixture.zip"), cards)
    if payload["category_totals"] != {"pokemon": 18, "trainers": 33, "energy": 9}:
        raise AssertionError(f"Unexpected derived category totals: {payload['category_totals']}")

    # The validation plan must use the category totals derived from the canonical
    # decklist and corpus-backed audit instead of preserving historical constants:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/data/decklist.json
    # https://github.com/FlareZ123/pokemon-sims/blob/main/scripts/audit_card_data.py
    # https://github.com/FlareZ123/pokemon-sims/issues/929
    totals = payload["category_totals"]
    expected_plan_requirement = (
        f"Assert {totals['pokemon']} Pokémon, {totals['trainers']} Trainers, "
        f"{totals['energy']} Energy, and 60 total cards"
    )
    validation_plan = SIM_PLAN_PATH.read_text(encoding="utf-8")
    if expected_plan_requirement not in validation_plan:
        raise AssertionError(
            "SIM-PLAN.md category requirement does not match the canonical decklist: "
            f"{expected_plan_requirement}"
        )

    mutated = copy.deepcopy(cards)
    mutated["swsh12-135"]["supertype"] = "Trainer"
    expect_system_exit(
        lambda: audit.build_payload(Path("mutated-supertype.zip"), mutated),
        "Unexpected supertype for Regidrago V",
    )

    unsupported = copy.deepcopy(cards)
    unsupported["swsh12-135"]["supertype"] = "Energy"
    expect_system_exit(
        lambda: audit.build_payload(Path("unsupported-supertype.zip"), unsupported),
        "Unsupported supertype for Regidrago V",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
