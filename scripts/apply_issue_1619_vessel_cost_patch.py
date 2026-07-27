from pathlib import Path
from textwrap import dedent

path = Path("src/trace_engine_v2/part_010.inc")
source = path.read_text(encoding="utf-8")

admission_anchor = dedent(
    """\
        const bool can_jit_discard = permit_payload &&
            (can_play_payload_this_turn() || payload_ready()) &&
            std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    """
)
admission_replacement = dedent(
    """\
        const bool issue_1619_direct_payload_cost = strict_payload_timing() &&
            need_payload() && state_.active &&
            state_.active->card == Card::RegidragoV &&
            state_.active->grass >= 2 && state_.active->fire == 0 &&
            hand_count(Card::RegidragoVstar) > 0 && fire_needed() > 0 &&
            might_be_unseen(Card::Fire) &&
            std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
            count_of(state_.discard, Card::SecretBox) > 0 &&
            count_of(state_.discard, Card::Dawn) > 0 &&
            count_of(state_.discard, Card::ForretressEx) > 0 &&
            count_of(state_.discard, Card::Pineco) > 0;
        // The generic item loop suppresses early payload discards for strict-JIT. This
        // exact public continuation overrides that suppression because Secret Box has
        // already selected Vessel, Dawn has supplied the payload and Forretress ex,
        // Exploding Energy has supplied GG, and the held VSTAR plus searchable Fire
        // prove that the payload must be discarded before the current turn ends:
        // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
        // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core discard, search, attachment, evolution, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Strict-JIT and earliest-completion policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1619
        const bool can_jit_discard = issue_1619_direct_payload_cost ||
            (permit_payload &&
             (can_play_payload_this_turn() || payload_ready()) &&
             std::any_of(state_.hand.begin(), state_.hand.end(), is_payload));
    """
)
if source.count(admission_anchor) != 1:
    raise SystemExit(
        f"issue-1619 Vessel admission anchor count: {source.count(admission_anchor)}"
    )
source = source.replace(admission_anchor, admission_replacement, 1)

cost_anchor = (
    "    const auto cost = choose_discard(can_jit_discard, true, true, Card::EarthenVessel);\n"
)
cost_replacement = dedent(
    """\
        std::optional<Card> cost;
        if (issue_1619_direct_payload_cost) {
          for (const Card card : state_.hand) {
            if (is_payload(card)) {
              cost = card;
              break;
            }
          }
        }
        // The proved current-turn payload is the Earthen Vessel cost before generic
        // low-DCI fodder. Spending Wishful Baton or another inert card would leave the
        // Dragon in hand and make the otherwise complete GGF route fail strict-JIT:
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1619
        if (!cost) {
          cost = choose_discard(can_jit_discard, true, true, Card::EarthenVessel);
        }
    """
)
if source.count(cost_anchor) != 1:
    raise SystemExit(
        f"issue-1619 Vessel cost anchor count: {source.count(cost_anchor)}"
    )
path.write_text(source.replace(cost_anchor, cost_replacement, 1), encoding="utf-8")
