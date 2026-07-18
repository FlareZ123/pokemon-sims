from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-852.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-852.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 852 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))

        replace_once(
            Path("src/trace_engine_v2/part_006.inc"),
            """    if (flex_fodder && scenario_.dci != DciProfile::StrictJit) {
      for (const Card card : {Card::MawileGX, Card::Guzma, Card::Channeler, Card::FieldBlower,
                              Card::ChaoticSwell, Card::PathToPeak, Card::TeamYellsCheer,
                              Card::RoseannesBackup, Card::Lusamine, Card::ProfessorTuro,
                              Card::Powerglass}) {
""",
            """    if (flex_fodder && scenario_.dci != DciProfile::StrictJit) {
      // Erika's Invitation replaced Mawile-GX in the canonical deck. Its effect is
      // opponent-dependent and deliberately inert in this goldfish model, so it is
      // the current low-impact matchup-flex discard candidate:
      // https://api.pokemontcg.io/v2/cards/sv3pt5-160
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // https://github.com/FlareZ123/pokemon-sims/issues/852
      for (const Card card : {Card::ErikasInvitation, Card::Guzma, Card::Channeler, Card::FieldBlower,
                              Card::ChaoticSwell, Card::PathToPeak, Card::TeamYellsCheer,
                              Card::RoseannesBackup, Card::Lusamine, Card::ProfessorTuro,
                              Card::Powerglass}) {
""",
        )

        replace_once(
            Path("src/trace_engine_v2/part_012.inc"),
            """        const bool can_cycle = *extra == Card::Dipplin || *extra == Card::MawileGX || *extra == Card::Guzma ||
                               *extra == Card::Channeler || *extra == Card::FieldBlower || *extra == Card::ChaoticSwell || *extra == Card::PathToPeak;
""",
            """        // Serena may discard up to three cards before drawing to five. Erika's
        // Invitation is the canonical inert replacement for retired Mawile-GX in
        // matchup-flex cycling:
        // https://api.pokemontcg.io/v2/cards/swsh12-164
        // https://api.pokemontcg.io/v2/cards/sv3pt5-160
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
        // https://github.com/FlareZ123/pokemon-sims/issues/852
        const bool can_cycle = *extra == Card::Dipplin || *extra == Card::ErikasInvitation || *extra == Card::Guzma ||
                               *extra == Card::Channeler || *extra == Card::FieldBlower || *extra == Card::ChaoticSwell || *extra == Card::PathToPeak;
""",
        )

        replace_once(
            Path("tests/gladion_unpayable_search_tests.cpp"),
            """  state.hand = {sim::Card::Gladion, sim::Card::UltraBall,
                sim::Card::MawileGX, sim::Card::Guzma};
  state.deck = {sim::Card::RegidragoV};
  state.prizes = {sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Matchup-flex policy permits the two visible matchup cards as legal costs, so
  // Ultra Ball can directly search Regidrago V and Gladion should be preserved:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sm4-95
""",
            """  state.hand = {sim::Card::Gladion, sim::Card::UltraBall,
                sim::Card::ErikasInvitation, sim::Card::Guzma};
  state.deck = {sim::Card::RegidragoV};
  state.prizes = {sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Matchup-flex policy permits Erika's Invitation and Guzma as the two visible
  // low-impact costs, so Ultra Ball can search Regidrago V and preserve Gladion:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://github.com/FlareZ123/pokemon-sims/issues/852
""",
        )
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
