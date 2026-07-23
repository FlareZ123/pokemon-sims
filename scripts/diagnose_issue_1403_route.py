from __future__ import annotations

import os
from pathlib import Path

PATH = Path("src/trace_engine_v2/part_010_fss_override.inc")


def main() -> int:
    text = PATH.read_text(encoding="utf-8")
    marker = "    if (fss_should_take_grass_for_seed23_latias_burnet_route()) return Card::Grass;\n"
    diagnostic = '''    if (state_.turn == 3 && hand_count(Card::Crispin) > 0 &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::ProfessorBurnet) > 0) {
      std::ostringstream issue1403;
      issue1403 << "route=" << fss_should_take_grass_for_seed23_latias_burnet_route()
                << " seen=" << deck_seen_
                << " dci=" << static_cast<int>(scenario_.dci)
                << " locks=" << static_cast<int>(scenario_.locks)
                << " max=" << scenario_.max_turn
                << " vstarUsed=" << state_.vstar_power_used
                << " supporterAllowed=" << supporter_allowed()
                << " manualUsed=" << state_.manual_energy_used
                << " retreatUsed=" << state_.retreat_used
                << " itemLocked=" << item_locked()
                << " active=" << (state_.active ? static_cast<int>(state_.active->card) : -1)
                << " activeBasic=" << (state_.active && is_basic(state_.active->card))
                << " benchSpace=" << bench_space()
                << " latiasAbility=" << ability_available_for_pokemon(Card::LatiasEx)
                << " latiasInPlay=" << in_play(Card::LatiasEx)
                << " latiasHand=" << hand_count(Card::LatiasEx)
                << " latiasDeck=" << std::count(state_.deck.begin(), state_.deck.end(), Card::LatiasEx)
                << " tate=" << hand_count(Card::TateLiza)
                << " vstar=" << hand_count(Card::RegidragoVstar)
                << " burnet=" << hand_count(Card::ProfessorBurnet)
                << " grassDeck=" << std::count(state_.deck.begin(), state_.deck.end(), Card::Grass)
                << " fireDeck=" << std::count(state_.deck.begin(), state_.deck.end(), Card::Fire)
                << " payloadDeck=" << std::count_if(state_.deck.begin(), state_.deck.end(), is_payload)
                << " bench=";
      for (const Pokemon& pokemon : state_.bench) {
        issue1403 << static_cast<int>(pokemon.card) << ':' << pokemon.entered_turn
                  << ':' << pokemon.grass << ':' << pokemon.fire << ',';
      }
      const_cast<Engine*>(this)->trace("ISSUE 1403 DIAGNOSTIC",
                                      "https://github.com/FlareZ123/pokemon-sims/issues/1403",
                                      issue1403.str());
    }
'''
    if diagnostic not in text:
        if text.count(marker) != 1:
            raise RuntimeError(f"Unexpected diagnostic anchor count: {text.count(marker)}")
        text = text.replace(marker, diagnostic + marker, 1)
    temporary = PATH.with_suffix(PATH.suffix + ".tmp")
    temporary.write_text(text, encoding="utf-8")
    os.replace(temporary, PATH)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
