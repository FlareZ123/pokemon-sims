#include <cassert>

int main() {
  // This intentionally false assertion must terminate every test configuration:
  // https://en.cppreference.com/w/cpp/error/assert
  assert(false && "Release test assertions must remain enabled");
  return 0;
}
