#!/usr/bin/env python3
"""Generate one C++ runner for every standalone simulator regression source."""
from __future__ import annotations

import argparse
import os
import re
import tempfile
from pathlib import Path

ANGLE_INCLUDE = re.compile(r'^\s*#include\s+(<[^>]+>)\s*$', re.MULTILINE)
LOCAL_INCLUDE = re.compile(r'^\s*#include\s+"([^"]+)"\s*$')
MAIN = re.compile(r'(?m)^int\s+main\s*\(\s*\)\s*\{')


def matching_brace(text: str, start: int) -> int:
    depth = 0
    index = start
    state = "code"
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and next_char == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return index
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and next_char == "/":
                state = "code"
                index += 2
                continue
        else:
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        index += 1
    raise ValueError("unmatched C++ brace")


def expand_test_file(path: Path, source_root: Path, tests_root: Path, stack: tuple[Path, ...] = ()) -> str:
    resolved = path.resolve()
    if resolved in stack:
        raise ValueError(f"test include cycle at {resolved}")
    simulator = (source_root / "src" / "regidrago_sim.cpp").resolve()
    compatibility_simulator = (tests_root / "src" / "regidrago_sim.cpp").resolve()
    output: list[str] = []
    for line in resolved.read_text(encoding="utf-8").splitlines():
        match = LOCAL_INCLUDE.match(line)
        if match:
            target = (resolved.parent / match.group(1)).resolve()
            if target in {simulator, compatibility_simulator}:
                continue
            if target.exists() and tests_root.resolve() in target.parents:
                output.append(expand_test_file(target, source_root, tests_root, stack + (resolved,)))
                continue
        output.append(line)
    return "\n".join(output) + "\n"


def extract_access_block(text: str, unique_name: str) -> tuple[str, list[str]]:
    blocks: list[str] = []
    position = 0
    while True:
        match = re.search(r'namespace\s+sim\s*\{', text[position:])
        if not match:
            return text, blocks
        namespace_start = position + match.start()
        namespace_brace = position + match.end() - 1
        namespace_end = matching_brace(text, namespace_brace)
        block = text[namespace_start : namespace_end + 1]
        if "struct EngineTestAccess" not in block:
            position = namespace_end + 1
            continue
        blocks.append(block.replace("EngineTestAccess", unique_name))
        tail = namespace_end + 1
        trailing = re.match(r'\s*;?\s*(?://\s*namespace\s+sim)?\s*', text[tail:])
        if trailing:
            tail += trailing.end()
        text = text[:namespace_start] + text[tail:]
        position = namespace_start


def identifier(value: str) -> str:
    return re.sub(r'\W+', '_', value)


def generate(source_root: Path, output_cpp: Path, output_cmake: Path) -> None:
    tests_root = source_root / "tests"
    ignored = {"release_assertion_tests.cpp"}
    test_files = sorted(path for path in tests_root.glob("*.cpp") if path.name not in ignored)
    source_headers = set(ANGLE_INCLUDE.findall(
        (source_root / "src" / "trace_engine_v2" / "part_000.inc").read_text(encoding="utf-8")
    ))
    headers: set[str] = set(source_headers)
    access_blocks: list[str] = []
    cases: list[tuple[str, str, str]] = []

    for path in test_files:
        text = expand_test_file(path, source_root, tests_root)
        headers.update(ANGLE_INCLUDE.findall(text))
        text = ANGLE_INCLUDE.sub("", text)
        text = re.sub(r'^\s*#define\s+REGIDRAGO_SIM_NO_MAIN\s*$', '', text, flags=re.MULTILINE)
        case_name = path.stem
        access_name = f"EngineTestAccess_{identifier(case_name)}"
        text, blocks = extract_access_block(text, access_name)
        if path.name != "regidrago_sim_tests.cpp" and len(blocks) != 1:
            raise ValueError(f"{path} must define exactly one EngineTestAccess block")
        access_blocks.extend(blocks)
        text = text.replace("EngineTestAccess", access_name)
        mains = list(MAIN.finditer(text))
        if len(mains) != 1:
            raise ValueError(f"{path} must define exactly one no-argument main")
        main_open = mains[0].end() - 1
        main_close = matching_brace(text, main_open)
        text = text[:main_close] + "\n  return 0;\n" + text[main_close:]
        text = text[: mains[0].start()] + "int run() {" + text[mains[0].end() :]
        cases.append((case_name, f"case_{identifier(case_name)}", text))

    generated: list[str] = [
        "// Generated by tests/generate_unified_tests.py. Do not edit.\n",
        "// The simulator is parsed once for all regression cases, avoiding one full\n",
        "// recompilation per test source. CMake still registers every case separately.\n",
    ]
    headers.discard("<windows.h>")
    for header in sorted(headers):
        generated.append(f"#include {header}\n")
    generated.extend([
        "#if defined(_WIN32)\n#include <windows.h>\n#endif\n",
        "\n#define REGIDRAGO_SIM_NO_MAIN\n",
        "#define private public\n",
        "#define protected public\n",
        '#include "src/regidrago_sim.cpp"\n',
        "#undef protected\n",
        "#undef private\n\n",
    ])
    for block in access_blocks:
        generated.append(block + "\n\n")
    for case_name, namespace, text in cases:
        generated.append(f"namespace {namespace} {{\n")
        generated.append(f'#line 1 "tests/{case_name}.cpp"\n')
        generated.append(text)
        generated.append(f"\n}}  // namespace {namespace}\n\n")
    generated.append("namespace {\nstruct UnifiedCase { const char* name; int (*run)(); };\n")
    generated.append("const UnifiedCase kCases[] = {\n")
    for case_name, namespace, _ in cases:
        generated.append(f'  {{"{case_name}", &{namespace}::run}},\n')
    generated.append("};\n}  // namespace\n\n")
    generated.append(r'''int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: regidrago_unified_tests <case>\n";
    return 2;
  }
  const std::string requested{argv[1]};
  for (const UnifiedCase& test : kCases) {
    if (requested != test.name) continue;
    std::cout << "[ RUN      ] " << test.name << '\n' << std::flush;
    const int status = test.run();
    if (status != 0) {
      std::cerr << "[  FAILED  ] " << test.name << " (status " << status << ")\n";
      return status;
    }
    std::cout << "[       OK ] " << test.name << '\n' << std::flush;
    return 0;
  }
  std::cerr << "unknown case: " << requested << '\n';
  return 2;
}
''')

    output_cpp.parent.mkdir(parents=True, exist_ok=True)
    cmake_content = (
        "set(REGIDRAGO_UNIFIED_CASES\n"
        + "".join(f"  {case_name}\n" for case_name, _, _ in cases)
        + ")\n"
    )
    for destination, content in ((output_cpp, "".join(generated)), (output_cmake, cmake_content)):
        with tempfile.NamedTemporaryFile(
            mode="w", encoding="utf-8", newline="\n", dir=destination.parent, delete=False
        ) as temporary:
            temporary.write(content)
            temporary.flush()
            os.fsync(temporary.fileno())
            temporary_path = Path(temporary.name)
        os.replace(temporary_path, destination)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--output-cpp", type=Path, required=True)
    parser.add_argument("--output-cmake", type=Path, required=True)
    args = parser.parse_args()
    generate(args.source_root.resolve(), args.output_cpp.resolve(), args.output_cmake.resolve())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
