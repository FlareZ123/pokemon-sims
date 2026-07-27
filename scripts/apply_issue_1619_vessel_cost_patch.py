from pathlib import Path
from textwrap import dedent

path = Path("src/trace_engine_v2/part_010.inc")
source = path.read_text(encoding="utf-8")
anchor = (
    "    const auto cost = choose_discard(can_jit_discard, true, true, Card::EarthenVessel);\n"
)
replacement = dedent(
    """\
        std::optional<Card> cost;
        const bool issue_1619_direct_payload_cost = can_jit_discard &&
            strict_payload_timing() && need_payload() && state_.active &&
            state_.active->card == Card::RegidragoV &&
            state_.active->grass >= 2 && state_.active->fire == 0 &&
            hand_count(Card::RegidragoVstar) > 0 && fire_needed() > 0 &&
            might_be_unseen(Card::Fire) &&
            count_of(state_.discard, Card::SecretBox) > 0 &&
            count_of(state_.discard, Card::Dawn) > 0 &&
            count_of(state_.discard, Card::ForretressEx) > 0 &&
            count_of(state_.discard, Card::Pineco) > 0;
        if (issue_1619_direct_payload_cost) {
          for (const Card card : state_.hand) {
            if (is_payload(card)) {
              cost = card;
              break;
            }
          }
        }
        // The complete public Secret Box-Dawn-Forretress route uses Earthen Vessel as
        // its only same-turn payload outlet and Fire search. Once Exploding Energy has
        // supplied GG and Dawn has put a Dragon in hand, lower-DCI generic fodder must
        // remain secondary because discarding it would strand strict-JIT payload timing:
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
        // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
        // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core discard, search, attachment, evolution, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Strict-JIT and earliest-completion policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1619
        if (!cost) {
          cost = choose_discard(can_jit_discard, true, true, Card::EarthenVessel);
        }
    """
)
if source.count(anchor) != 1:
    raise SystemExit(f"issue-1619 Vessel cost anchor count: {source.count(anchor)}")
path.write_text(source.replace(anchor, replacement, 1), encoding="utf-8")
