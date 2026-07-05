Scenario sc(std::string label, DciProfile profile = DciProfile::StrictJit, LockMode locks = LockMode::None, bool first = false) {
  return Scenario{std::move(label), profile, locks, first, 4};
}
State st(int turn, Card active = Card::RegidragoV, int entered = 0, int grass = 0, int fire = 0) {
  State s; s.turn = turn; s.active = Pokemon{active, entered, grass, fire, Tool::None}; return s;
}
bool has(const std::vector<Card>& cards, Card card) { return std::find(cards.begin(), cards.end(), card) != cards.end(); }
void expect(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }

