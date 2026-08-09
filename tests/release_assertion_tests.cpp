#include <cassert>

namespace {
void verify_release_assertions_enabled() {
  // This intentionally false assertion must terminate every test configuration:
  // https://en.cppreference.com/w/cpp/error/assert
  assert(false && "Release test assertions must remain enabled");
}
}

int main() {
  verify_release_assertions_enabled();
  return 0;
}
