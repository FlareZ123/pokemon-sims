from pathlib import Path
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    fd, temp_name = tempfile.mkstemp(prefix=path.name + ".", dir=path.parent, text=True)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp_name, path)
    except BaseException:
        try:
            os.unlink(temp_name)
        except FileNotFoundError:
            pass
        raise


path = ROOT / "src/trace_engine_v2/part_012.inc"
text = path.read_text(encoding="utf-8")
anchor = "bool issue_1598_bank_prized_treasure_{false};\n\nbool play_gladion() {\n"
helper = r'''bool issue_1598_bank_prized_treasure_{false};

  bool issue_2292_gladion_final_prize_energy_finish(const Card energy) const {
    if (scenario_.dci != DciProfile::StrictJit || !prizes_known() ||
        !supporter_allowed() || state_.manual_energy_used ||
        hand_count(Card::Gladion) == 0 || hand_count(energy) > 0 ||
        !state_.active || state_.active->card != Card::RegidragoVstar ||
        prize_count_after_reveal(energy) == 0 ||
        grass_needed() + fire_needed() != 1 || !payload_ready()) {
      return false;
    }

    // Gladion may take any known Prize. When Regidrago VSTAR is exactly one manual
    // attachment from GGF and this turn's strict-JIT Dragon is already discarded,
    // the known prized missing Energy completes immediately even if another copy
    // of that Energy type remains in the inspected deck.
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Prize, Supporter and manual Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 / strict-JIT / earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2292
    return (energy == Card::Grass && grass_needed() == 1 && fire_needed() == 0) ||
           (energy == Card::Fire && grass_needed() == 0 && fire_needed() == 1);
  }

bool play_gladion() {
'''
if helper not in text:
    if text.count(anchor) != 1:
        raise RuntimeError("#2292 helper anchor mismatch")
    text = text.replace(anchor, helper, 1)

old = '''        const bool same_energy_absent_from_deck = prize_energy.has_value() &&
            deck_count_after_search_started(*prize_energy) == 0;

        if (prize_energy && same_energy_absent_from_deck && !live_crispin) {
          known_target = *prize_energy;
'''
new = '''        const bool same_energy_absent_from_deck = prize_energy.has_value() &&
            deck_count_after_search_started(*prize_energy) == 0;
        const bool direct_prize_energy_finish = prize_energy.has_value() &&
            issue_2292_gladion_final_prize_energy_finish(*prize_energy);

        // A same-type Energy elsewhere in deck does not make that copy accessible
        // after Wonder Tag has already selected Gladion for the unused Supporter play.
        // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
        // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // One Supporter and one manual attachment per turn: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
        // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2292
        if (prize_energy && (same_energy_absent_from_deck || direct_prize_energy_finish) &&
            !live_crispin) {
          known_target = *prize_energy;
'''
if new not in text:
    if text.count(old) != 1:
        raise RuntimeError("#2292 Gladion gate anchor mismatch")
    text = text.replace(old, new, 1)
atomic_write(path, text)

cmake = ROOT / "CMakeLists.txt"
c = cmake.read_text(encoding="utf-8")
if "issue_2292_gladion_final_prize_energy_tests" not in c:
    c += r'''

# Known Prize Grass is an immediate Gladion connector even when another Grass
# remains in deck, provided Supporter/manual attachment remain and JIT payload is live.
# Gladion / Tapu Lele-GX / Quick Ball: https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/sm2-60 https://api.pokemontcg.io/v2/cards/swsh1-179
# Mega Dragonite ex / Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/me2pt5-152 https://api.pokemontcg.io/v2/cards/swsh12-136
# Core rules: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2292
add_executable(issue_2292_gladion_final_prize_energy_tests
  tests/issue_2292_gladion_final_prize_energy_tests.cpp)
add_test(NAME issue_2292_gladion_final_prize_energy_tests
  COMMAND issue_2292_gladion_final_prize_energy_tests)
add_test(NAME trace_issue_2292_strict_first_seed692
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 692 --require-ready-by 4)
'''
    atomic_write(cmake, c)
