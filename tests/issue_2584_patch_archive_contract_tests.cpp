#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::filesystem::path chunk_directory() {
  std::filesystem::path directory = std::filesystem::current_path();
  for (int depth = 0; depth < 3; ++depth) {
    const auto candidate = directory / "scripts" / "issue_1394_patch_chunks";
    if (std::filesystem::is_directory(candidate)) return candidate;
    if (!directory.has_parent_path()) break;
    directory = directory.parent_path();
  }
  throw std::runtime_error("Could not locate issue-1394 patch chunks");
}

std::string read_chunk(const std::filesystem::path& path) {
  std::ifstream input(path);
  expect(input.good(), "Could not open issue-1394 patch chunk");
  std::string content((std::istreambuf_iterator<char>(input)),
                      std::istreambuf_iterator<char>());
  while (!content.empty() &&
         std::isspace(static_cast<unsigned char>(content.back()))) {
    content.pop_back();
  }
  return content;
}

bool is_base64_character(const char value) {
  const unsigned char byte = static_cast<unsigned char>(value);
  return std::isalnum(byte) || value == '+' || value == '/' || value == '=';
}

void test_issue_1394_archive_is_complete_base64_stream() {
  // The loader concatenates five text chunks before decoding. Python's strict
  // decoder requires a correctly padded Base64 stream, so every stored split is
  // kept on a four-character boundary and padding is confined to the final chunk:
  // https://docs.python.org/3/library/base64.html#base64.b64decode
  // Confirmed malformed-archive bug: https://github.com/FlareZ123/pokemon-sims/issues/2584
  // Validated source artifact: https://github.com/FlareZ123/pokemon-sims/actions/runs/30057886658
  const auto directory = chunk_directory();
  std::vector<std::filesystem::path> chunks;
  for (const auto& entry : std::filesystem::directory_iterator(directory)) {
    if (entry.is_regular_file() && entry.path().extension() == ".b64") {
      chunks.push_back(entry.path());
    }
  }
  std::sort(chunks.begin(), chunks.end());
  expect(chunks.size() == 5, "Issue-1394 archive no longer has exactly five chunks");

  std::string encoded;
  for (std::size_t index = 0; index < chunks.size(); ++index) {
    const std::string chunk = read_chunk(chunks[index]);
    expect(!chunk.empty(), "Issue-1394 patch chunk is empty");
    expect(chunk.size() % 4 == 0,
           "Issue-1394 patch chunk is not split on a Base64 quantum boundary");
    expect(std::all_of(chunk.begin(), chunk.end(), is_base64_character),
           "Issue-1394 patch chunk contains a non-Base64 character");
    if (index + 1 != chunks.size()) {
      expect(chunk.find('=') == std::string::npos,
             "Base64 padding appeared before the final issue-1394 chunk");
    }
    encoded += chunk;
  }

  expect(encoded.size() == 21648,
         "Issue-1394 patch archive length no longer matches the validated artifact");
  expect(encoded.ends_with("="),
         "Issue-1394 patch archive lost its final Base64 padding");
}

}  // namespace

int main() {
  test_issue_1394_archive_is_complete_base64_stream();
}
