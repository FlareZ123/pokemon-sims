from pathlib import Path
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


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


path = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"
text = path.read_text(encoding="utf-8")
old = '''        state_.active->card != Card::Oricorio ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        !need_active_vstar() || need_energy() || need_payload()) {
'''
new = '''        state_.active->card != Card::Oricorio ||
        // Skyliner is a Rule Box Pokemon Ability, so these modeled lock modes block it:
        // https://api.pokemontcg.io/v2/cards/sv8-76
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
        scenario_.locks == LockMode::FullRuleBoxAbility ||
        scenario_.locks == LockMode::FullCombined ||
        // Strict-JIT readiness requires a Dragon payload to enter discard this turn:
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
        !std::any_of(state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(),
                     [this](const Card card) { return is_payload(card); })) {
'''
if text.count(old) != 1:
    raise RuntimeError("#2293 target-local readiness guard anchor mismatch")
text = text.replace(old, new, 1)
atomic_write(path, text)
