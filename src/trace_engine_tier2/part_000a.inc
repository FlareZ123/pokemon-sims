#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace sim {

// The traceability IDs referenced below are defined in docs/RULES_TRACEABILITY.md.
enum class Card : std::uint8_t {
  RegidragoV,
  RegidragoVstar,
  Dragapult,
  MegaDragonite,
  DialgaGX,
  GoodraVstar,
  TapuLeleGX,
  LatiasEx,
  MawileGX,
  Oricorio,
  Dipplin,
  BrilliantBlender,
  MysteriousTreasure,
  QuickBall,
  UltraBall,
  EvolutionIncense,
  EarthenVessel,
  Arven,
