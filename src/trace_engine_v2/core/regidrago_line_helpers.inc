#ifndef SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED
#define SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED

static constexpr bool is_regidrago_v_line(const Card card) {
  return card == Card::RegidragoV || card == Card::RegidragoVstar;  // Card identities: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
}

#endif  // SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED
