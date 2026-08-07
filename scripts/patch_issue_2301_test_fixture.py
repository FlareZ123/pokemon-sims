from pathlib import Path
import fcntl
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lock_path = path.with_name(path.name + ".lock")
    with lock_path.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=path.parent, delete=False
        ) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_name = tmp.name
        os.replace(tmp_name, path)
        fcntl.flock(lock.fileno(), fcntl.LOCK_UN)
    lock_path.unlink(missing_ok=True)


path = ROOT / "tests/issue_2301_pineco_quick_ball_vessel_timer_tests.cpp"
text = path.read_text(encoding="utf-8")
old = '''  state.hand = {sim::Card::QuickBall, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::SecretBox,
                sim::Card::GoodraVstar, sim::Card::MegaDragonite};
  return state;
}
'''
new = '''  state.hand = {sim::Card::QuickBall, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::SecretBox,
                sim::Card::GoodraVstar, sim::Card::MegaDragonite};
  // K0 means the policy cannot inspect the hidden deck. The simulator still needs
  // a physical hidden zone so might_be_unseen() can represent fixed-list
  // plausibility without revealing exact Prize identities to the decision policy.
  // K0/K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  state.deck = {sim::Card::RegidragoV, sim::Card::ForestSealStone,
                sim::Card::Dawn, sim::Card::Pineco, sim::Card::ForretressEx,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  return state;
}
'''
if text.count(old) != 1:
    raise RuntimeError("#2301 K0 fixture anchor mismatch")
text = text.replace(old, new, 1)
atomic_write(path, text)
