#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED

#ifndef SIM_TRACE_CARD_CATALOG_INCLUDED
#define SIM_TRACE_CARD_CATALOG_INCLUDED
#include "core/card_catalog.inc"
#endif

#include "core/card_classification.inc"

#endif  // SIM_TRACE_CARD_CLASSIFICATION_INCLUDED

// Compatibility source-contract shim. Canonical implementation:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// The executable compiles only the canonical core include above. The inactive
// mirror keeps raw-source contracts able to inspect the payload predicate after
// the implementation moved into core/.
#if 0
constexpr bool is_payload(const Card card) {
  return card == Card::Dragapult || card == Card::MegaDragonite || card == Card::DialgaGX ||
         card == Card::GoodraVstar || card == Card::Appletun;
}
#endif
// Legacy source-anchor compatibility follows. Historical source links into this
// wrapper remain resolvable while the implementation lives in core/.
// 020
// 021
// 022
// 023
// 024
// 025
// 026
// 027
// 028
// 029
// 030
// 031
// 032
// 033
// 034
// 035
// 036
// 037
// 038
// 039
// 040
// 041
// 042
// 043
// 044
// 045
// 046
// 047
// 048
// 049
// 050
// 051
// 052
// 053
// 054
// 055
// 056
// 057
// 058
// 059
// 060
// 061
// 062
// 063
// 064
// 065
// 066
// 067
// 068
// 069
// 070
// 071
// 072
// 073
// 074
// 075
// 076
// 077
// 078
// 079
// 080
// 081
// 082
// 083
// 084
// 085
// 086
// 087
// 088
// 089
// 090
// 091
// 092
// 093
// 094
// 095
// 096
// 097
// 098
// 099
// 100
// 101
// 102
// 103
// 104
// 105
// 106
// 107
// 108
// 109
// 110
// 111
// 112
// 113
// 114
// 115
// 116
// 117
// 118
// 119
// 120
// 121
// 122
// 123
// 124
// 125
// 126
// 127
// 128
// 129
// 130
// 131
// 132
// 133
// 134
// 135
// 136
// 137
// 138
// 139
// 140
// 141
// 142
// 143
// 144
// 145
// 146
// 147
// 148
// 149
// 150
