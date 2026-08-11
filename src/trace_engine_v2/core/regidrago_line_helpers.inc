#ifndef SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED
#define SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED

static constexpr bool is_regidrago_v_line(const Card card) {
  // Keep the two modeled Regidrago evolution identities in one reusable predicate:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  return card == Card::RegidragoV || card == Card::RegidragoVstar;
}

#endif  // SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED
