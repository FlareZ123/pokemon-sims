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


part7 = ROOT / "src/trace_engine_v2/part_007.inc"
text = part7.read_text(encoding="utf-8")
anchor = "  Card choose_supporter_after_search_started() const {\n"
helper = r'''  bool issue_2291_wonder_tag_arven_fss_vessel_route() const {
    if (scenario_.dci != DciProfile::StrictJit || !scenario_.going_first ||
        state_.turn != 4 || !prizes_known() || !supporter_allowed() ||
        item_locked() || state_.manual_energy_used || state_.vstar_power_used ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 1 || state_.active->fire != 1 ||
        state_.active->tool != Tool::None ||
        deck_count_after_search_started(Card::Arven) == 0 ||
        deck_count_after_search_started(Card::ForestSealStone) == 0 ||
        deck_count_after_search_started(Card::EarthenVessel) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) != 0 ||
        !in_play(Card::TapuLeleGX) ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // K1 proves Crispin can find only Grass, so its conditional attachment cannot
    // happen. Arven can instead search Forest Seal Stone and Earthen Vessel; Star
    // Alchemy supplies Regidrago VSTAR, Vessel supplies Grass, and the held Dragon
    // pays Vessel's discard cost as the same-turn strict-JIT payload.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Tool, VSTAR Power, Item, discard, search, attachment and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, DCI/AMR and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2291
    return true;
  }

'''
if helper not in text:
    if text.count(anchor) != 1:
        raise RuntimeError("#2291 selector anchor mismatch")
    text = text.replace(anchor, helper + anchor, 1)
old = """  Card choose_supporter_after_search_started() const {\n    if (issue_1797_wonder_tag_steven_route_available()) {\n"""
new = """  Card choose_supporter_after_search_started() const {\n    if (issue_2291_wonder_tag_arven_fss_vessel_route()) {\n      // The complete K1 route is Arven -> FSS/Vessel: https://github.com/FlareZ123/pokemon-sims/issues/2291\n      return Card::Arven;\n    }\n    if (issue_1797_wonder_tag_steven_route_available()) {\n"""
if new not in text:
    if text.count(old) != 1:
        raise RuntimeError("#2291 selector insertion mismatch")
    text = text.replace(old, new, 1)
atomic_write(part7, text)

override = ROOT / "src/trace_engine_v2/part_issue_2291_arven_vessel_fss_override.inc"
override_text = r'''  bool play_arven() {
    if (!issue_2291_wonder_tag_arven_fss_vessel_route() || hand_count(Card::Arven) == 0) {
      return play_arven_issue2291_original();
    }

    // Arven searches one Item and one Pokemon Tool. In this exact K1 state,
    // Earthen Vessel is the Item channel that supplies the missing Grass while
    // Forest Seal Stone supplies Star Alchemy for the missing Regidrago VSTAR.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core search, Supporter, Item, Tool, Ability, Energy attachment and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest-route and connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2291
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven issue-2291 route");

    std::vector<Card> found;
    if (move_deck_to_hand(Card::EarthenVessel)) found.push_back(Card::EarthenVessel);
    if (move_deck_to_hand(Card::ForestSealStone)) found.push_back(Card::ForestSealStone);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01; P-DCI-01",
          "Searched the deterministic K1 FSS/Vessel finish: " + join_cards(found) + ".");
    return true;
  }
'''
atomic_write(override, override_text)

sim = ROOT / "src/regidrago_sim.cpp"
s = sim.read_text(encoding="utf-8")
old_include = '''#define play_arven play_arven_empty_deck_original
#include "trace_engine_v2/part_012_arven_fss_blender_contention_override.inc"
#undef play_arven
#define choose_supporter choose_supporter_original
'''
new_include = '''#define play_arven play_arven_issue2291_original
#include "trace_engine_v2/part_012_arven_fss_blender_contention_override.inc"
#undef play_arven
#include "trace_engine_v2/part_issue_2291_arven_vessel_fss_override.inc"
#define choose_supporter choose_supporter_original
'''
if new_include not in s:
    if s.count(old_include) != 1:
        raise RuntimeError("#2291 play_arven include anchor mismatch")
    s = s.replace(old_include, new_include, 1)
atomic_write(sim, s)

cmake = ROOT / "CMakeLists.txt"
c = cmake.read_text(encoding="utf-8")
if "trace_issue_2291_strict_first_seed275" not in c:
    c += r'''

# K1 Wonder Tag -> Arven -> FSS/Vessel is the deterministic strict-JIT T4 finish.
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
# Crispin and ruling: https://api.pokemontcg.io/v2/cards/sv7-133 https://compendium.pokegym.net/category/5-trainers/crispin/
# Arven / FSS / Vessel / Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/sv1-166 https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sv4-163 https://api.pokemontcg.io/v2/cards/swsh12-136
# Core rules: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2291
add_test(NAME trace_issue_2291_strict_first_seed275
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 275 --require-ready-by 4)
'''
    atomic_write(cmake, c)
