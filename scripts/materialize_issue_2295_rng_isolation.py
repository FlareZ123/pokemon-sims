from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"


def atomic_write_locked(path: Path, text: str) -> None:
    lock_path = path.with_name(f"{path.name}.lock")
    lock_fd = os.open(lock_path, os.O_CREAT | os.O_EXCL | os.O_RDWR)
    try:
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
        ) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_path = Path(tmp.name)
        os.replace(tmp_path, path)
    finally:
        os.close(lock_fd)
        lock_path.unlink(missing_ok=True)


source = SOURCE.read_text(encoding="utf-8")
old = '''    projected.state_.discard.push_back(payment);
    std::swap(*projected.state_.active, projected.state_.bench[target_index]);
    projected.state_.retreat_used = true;
    if (!projected.play_brilliant_blender()) return false;
'''
new = '''    projected.state_.discard.push_back(payment);
    std::swap(*projected.state_.active, projected.state_.bench[target_index]);
    projected.state_.retreat_used = true;
    // Engine copies share the live RNG reference. Preserve the generator around
    // the policy-only Blender projection so its deck shuffle cannot perturb the
    // real simulation stream. This follows the established projection contract:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1873_blender_crispin_supporter_override.inc
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Confirmed route bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
    const std::mt19937_64 live_rng = rng_;
    const bool projected_blender_available = projected.play_brilliant_blender();
    rng_ = live_rng;
    if (!projected_blender_available) return false;
'''
if source.count(old) != 1:
    raise RuntimeError(f"projection anchor count {source.count(old)}")
source = source.replace(old, new, 1)
atomic_write_locked(SOURCE, source)
