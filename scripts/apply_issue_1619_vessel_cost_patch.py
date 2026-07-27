from pathlib import Path
from textwrap import dedent

path = Path("src/trace_engine_v2/part_010.inc")
source = path.read_text(encoding="utf-8")

admission_anchor = (
    "    const bool can_jit_discard = permit_payload &&\n"
    "        (can_play_payload_this_turn() || payload_ready()) &&\n"
    "        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);\n"
)
admission_replacement = (
    "    const bool issue_1619_direct_payload_cost = strict_payload_timing() &&\n"
    "        need_payload() && state_.active &&\n"
    "        state_.active->card == Card::RegidragoV &&\n"
    "        state_.active->grass >= 2 && state_.active->fire == 0 &&\n"
    "        hand_count(Card::RegidragoVstar) > 0 && fire_needed() > 0 &&\n"
    "        might_be_unseen(Card::Fire) &&\n"
    "        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&\n"
    "        count_of(state_.discard, Card::SecretBox) > 0 &&\n"
    "        count_of(state_.discard, Card::Dawn) > 0 &&\n"
    "        count_of(state_.discard, Card::ForretressEx) > 0 &&\n"
    "        count_of(state_.discard, Card::Pineco) > 0;\n"
    "    // The generic item loop suppresses early payload discards for strict-JIT. This\n"
    "    // exact public continuation overrides that suppression because Secret Box has\n"
    "    // already selected Vessel, Dawn has supplied the payload and Forretress ex,\n"
    "    // Exploding Energy has supplied GG, and the held VSTAR plus searchable Fire\n"
    "    // prove that the payload must be discarded before the current turn ends:\n"
    "    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163\n"
    "    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163\n"
    "    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87\n"
    "    // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2\n"
    "    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136\n"
    "    // Core discard, search, attachment, evolution, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules\n"
    "    // Strict-JIT and earliest-completion policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n"
    "    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1619\n"
    "    const bool can_jit_discard = issue_1619_direct_payload_cost ||\n"
    "        (permit_payload &&\n"
    "         (can_play_payload_this_turn() || payload_ready()) &&\n"
    "         std::any_of(state_.hand.begin(), state_.hand.end(), is_payload));\n"
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
