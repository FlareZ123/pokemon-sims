from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LOCK_PATH = ROOT / ".issue-1185-materializer.lock"


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
    ) as handle:
        handle.write(text)
        temporary = Path(handle.name)
    os.replace(temporary, path)


def patch_source() -> None:
    path = ROOT / "src/trace_engine_v2/part_009b1.inc"
    text = path.read_text(encoding="utf-8")
    old_gate = """    if (scenario_.dci != DciProfile::MatchupFlexJit || item_locked() ||
"""
    new_gate = """    const bool route_allows_surplus_energy =
        scenario_.dci == DciProfile::MatchupFlexJit ||
        scenario_.dci == DciProfile::StrictJit;  // Confirmed strict-JIT route: https://github.com/FlareZ123/pokemon-sims/issues/1185
    if (!route_allows_surplus_energy || item_locked() ||
"""
    if old_gate in text:
        text = text.replace(old_gate, new_gate, 1)
    elif new_gate not in text:
        raise RuntimeError("Issue 1185 Quick Ball profile gate was not found.")

    old_sources = """    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1146 https://github.com/FlareZ123/pokemon-sims/issues/1154 https://github.com/FlareZ123/pokemon-sims/issues/1155
"""
    new_sources = """    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1146 https://github.com/FlareZ123/pokemon-sims/issues/1154 https://github.com/FlareZ123/pokemon-sims/issues/1155 https://github.com/FlareZ123/pokemon-sims/issues/1185
"""
    if old_sources in text:
        text = text.replace(old_sources, new_sources, 1)
    elif new_sources not in text:
        raise RuntimeError("Issue 1185 source-bound bug URL line was not found.")
    atomic_write(path, text)


def patch_prior_regression() -> None:
    path = ROOT / "tests/issue_1146_quick_ball_surplus_energy_tests.cpp"
    text = path.read_text(encoding="utf-8")
    old = """  const sim::Scenario strict{\"issue-1146-strict\", sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 5};
  blocked(route_state(), strict, 114606,
          \"Strict DCI must keep the singleton Energy protected.\");
"""
    new = """  const sim::Scenario strict{\"issue-1146-strict\", sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 5};
  std::mt19937_64 strict_rng{114606};
  sim::Engine strict_engine = make_engine(strict, strict_rng, route_state());
  // Strict JIT may spend the final Energy once GGF is complete and the held
  // Blender plus Latias ex route deterministically completes the ready turn:
  // https://github.com/FlareZ123/pokemon-sims/issues/1185
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  expect(sim::EngineTestAccess::final_surplus_cost(strict_engine) ==
             sim::Card::Grass,
         \"Strict JIT must admit the complete route-conditioned surplus Energy.\");

  sim::State strict_no_blender = route_state();
  strict_no_blender.hand.erase(
      std::find(strict_no_blender.hand.begin(), strict_no_blender.hand.end(),
                sim::Card::BrilliantBlender));
  blocked(strict_no_blender, strict, 114609,
          \"Strict JIT must protect Energy without a ready-turn payload route.\");
"""
    if old in text:
        text = text.replace(old, new, 1)
    elif new not in text:
        raise RuntimeError("Issue 1146 strict-JIT regression block was not found.")
    atomic_write(path, text)


def main() -> None:
    with LOCK_PATH.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        patch_source()
        patch_prior_regression()


if __name__ == "__main__":
    main()
