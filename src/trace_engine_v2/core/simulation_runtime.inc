// Runtime support is composed in declaration order from focused ownership units.
// Keep state before metrics before trace so downstream Engine formatting helpers see
// the same complete types at the historical boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Trace fixed-point policy: https://github.com/FlareZ123/pokemon-sims/issues/1007
#include "runtime/state.inc"
#include "runtime/metrics.inc"

struct TraceLog {
  bool enabled{false};
  std::vector<std::string> lines;
  std::map<std::string, std::string> last_policy_state_by_key;

  TraceLog() = default;

  // Preserve the existing one-, two-, and three-argument construction forms
  // while initializing every member through a declared constructor:
  // https://eel.is/c++draft/dcl.init.aggr
  // https://github.com/FlareZ123/pokemon-sims/issues/2177
  explicit TraceLog(
      const bool is_enabled, std::vector<std::string> initial_lines = {},
      std::map<std::string, std::string> initial_policy_states = {})
      : enabled(is_enabled),
        lines(std::move(initial_lines)),
        last_policy_state_by_key(std::move(initial_policy_states)) {}

  static std::string policy_state_fingerprint(const State& state) {
    // Fixed-point policy may query the same no-op decision repeatedly. Public trace
    // events are emitted once per equivalent visible state and may reappear after a
    // real state change. Zone order is canonicalized because deck shuffles do not
    // change K1 card availability or any visible policy axis:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
    // https://github.com/FlareZ123/pokemon-sims/issues/1007
    std::ostringstream out;
    const auto append_cards = [&out](const std::vector<Card>& cards) {
      std::vector<int> ordered;
      ordered.reserve(cards.size());
      for (const Card card : cards) ordered.push_back(static_cast<int>(card));
      std::sort(ordered.begin(), ordered.end());
      out << '[';
      for (const int card : ordered) out << card << ',';
      out << ']';
    };
    const auto append_pokemon = [&out](const Pokemon& pokemon) {
      out << static_cast<int>(pokemon.card) << ':' << pokemon.entered_turn << ':'
          << pokemon.grass << ':' << pokemon.fire << ':'
          << static_cast<int>(pokemon.tool) << ':'
          << pokemon.double_dragon << ';';  // DDE is public attached state: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ ; https://github.com/FlareZ123/pokemon-sims/issues/2238
    };

    append_cards(state.deck);
    append_cards(state.hand);
    append_cards(state.prizes);
    append_cards(state.discard);
    append_cards(state.discarded_this_turn);
    if (state.active) append_pokemon(*state.active);
    else out << "no-active;";
    out << "bench{";
    for (const Pokemon& pokemon : state.bench) append_pokemon(pokemon);
    out << "}:" << state.turn << ':' << state.supporter_used << ':'
        << state.manual_energy_used << ':' << state.retreat_used << ':'
        << state.stadium_used << ':'
        << state.vstar_power_used << ':' << state.dark_asset_used << ':'
        << state.turn_ended << ':'
        << state.path_lock_removed << ':' << static_cast<int>(state.stadium) << ':'
        << state.issue_1447_vessel_ready_turn.value_or(0); // Route-continuation state: https://github.com/FlareZ123/pokemon-sims/issues/3561
    return out.str();
  }

  void add(const int turn, std::string_view action, std::string_view rules, std::string_view detail) {
    if (!enabled) return;
    std::ostringstream out;
    if (turn > 0) out << "T" << turn << " | ";
    out << action << " | rules: " << rules << " | " << detail;
    lines.push_back(out.str());
  }

  void add_policy_once(const State& state, std::string_view key, const int turn,
                       std::string_view action, std::string_view rules,
                       std::string_view detail) {
    if (!enabled) return;
    const std::string fingerprint = policy_state_fingerprint(state);
    const std::string owned_key{key};
    const auto found = last_policy_state_by_key.find(owned_key);
    if (found != last_policy_state_by_key.end() && found->second == fingerprint) return;
    last_policy_state_by_key[owned_key] = fingerprint;
    add(turn, action, rules, detail);
  }

  void note(std::string_view detail) {
    if (enabled) lines.emplace_back(detail);
  }
};
