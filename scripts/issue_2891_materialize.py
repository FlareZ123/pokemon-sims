import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "src/trace_engine_v2/part_010_fss_override.inc"

text = PATH.read_text(encoding="utf-8")
start_marker = "  bool fss_should_take_grass_for_seed23_latias_burnet_route() const {\n"
end_marker = "\n  Card fss_target_after_search_started() const {"
start = text.index(start_marker)
end = text.index(end_marker, start)

replacement = '''  bool fss_should_take_grass_for_seed23_latias_burnet_route() const {
    // Historical name retained for regression compatibility. The route is
    // state-semantic: Star Alchemy searches one Grass, Crispin searches a second
    // Grass plus Fire and attaches one, and the unused manual attachment supplies
    // the other current-turn Grass. Quick Ball replaces Tate & Liza with Latias ex.
    // On the projected next turn the established Regidrago V can evolve, the held
    // Fire attachment completes GGF, and Professor Burnet supplies the JIT payload.
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced evolution, Item, Supporter, attachment, and Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1, shared JIT timing, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI/AMR approximation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Original route and state-generic paired Quick Ball route: https://github.com/FlareZ123/pokemon-sims/issues/1403 https://github.com/FlareZ123/pokemon-sims/issues/2704
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2891
    const int projected_ready_turn = state_.turn + 1;
    if (!prizes_known() || !strict_payload_timing() ||
        scenario_.max_turn < projected_ready_turn ||
        !supporter_allowed() || state_.manual_energy_used ||
        state_.retreat_used || item_locked() || !state_.active ||
        !is_basic(state_.active->card) || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        std::count(state_.deck.begin(), state_.deck.end(), Card::LatiasEx) == 0 ||
        hand_count(Card::Crispin) == 0 || hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        std::count(state_.deck.begin(), state_.deck.end(), Card::Grass) < 2 ||
        std::count(state_.deck.begin(), state_.deck.end(), Card::Fire) == 0 ||
        !std::any_of(state_.deck.begin(), state_.deck.end(), is_payload)) {
      return false;
    }

    if (state_.active->card == Card::RegidragoV ||
        state_.active->card == Card::RegidragoVstar) {
      return false;
    }

    return std::any_of(
        state_.bench.begin(), state_.bench.end(),
        [projected_ready_turn](const Pokemon& pokemon) {
          // A Regidrago V already in play before the projected ready turn may
          // legally evolve on that next turn. Literal seed turn numbers do not
          // create an evolution requirement:
          // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
          // Advanced evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2891
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < projected_ready_turn &&
              pokemon.grass == 0 && pokemon.fire == 0;
        });
  }
'''

updated = text[:start] + replacement + text[end:]
if updated == text:
    raise RuntimeError("#2891 materializer made no source change")

fd, temp_name = tempfile.mkstemp(prefix=f".{PATH.name}.", dir=PATH.parent)
try:
    with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as handle:
        handle.write(updated)
        handle.flush()
        os.fsync(handle.fileno())
    os.replace(temp_name, PATH)
except BaseException:
    try:
        os.unlink(temp_name)
    except FileNotFoundError:
        pass
    raise
