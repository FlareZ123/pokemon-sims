#include <cassert>

namespace {
constexpr char kReleaseAssertionMessage[] = "Release test assertions must remain enabled";
}

int main() {
  // This intentionally false assertion must terminate every test configuration:
  // https://en.cppreference.com/w/cpp/error/assert
  assert(false && kReleaseAssertionMessage);
  return 0;
}
