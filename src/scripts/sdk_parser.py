import json
import argparse
from pathlib import Path

def build_pattern_call(entry: dict) -> str:
    if "instruction" in entry:
        return entry["instruction"]

    signature = entry.get("signature")
    if not signature:
        raise ValueError(f"Entry for {entry.get('symbol')} must have 'signature' or 'instruction'")

    has_deref = "deref" in entry
    has_ref = "ref" in entry

    if has_deref and has_ref:
        raise ValueError("Entry cannot contain both 'deref' and 'ref'")

    if has_deref:
        offset = entry["deref"]
        return f'selaura::pattern<"{signature}", selaura::signature_offset<selaura::deref, {offset}>>::resolve();'

    if has_ref:
        offset = entry["ref"]
        return f'selaura::pattern<"{signature}", selaura::signature_offset<selaura::ref, {offset}>>::resolve();'

    return f'selaura::pattern<"{signature}">::resolve();'

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, help="Path to patterns.json")
    parser.add_argument("--def-out", required=True, help="Path to output .def file")
    parser.add_argument("--cpp-out", required=True, help="Path to output .hpp file")
    args = parser.parse_args()

    input_path = Path(args.input)
    def_output = Path(args.def_out)
    cpp_output = Path(args.cpp_out)

    data = json.loads(input_path.read_text(encoding="utf-8"))

    # Generate .def file
    def_lines = ["LIBRARY sdk.dll", "EXPORTS"]
    for idx, entry in enumerate(data, start=1):
        def_lines.append(f"    {entry['symbol']} @{idx} NONAME")

    def_output.parent.mkdir(parents=True, exist_ok=True)
    def_output.write_text("\n".join(def_lines), encoding="utf-8")

    # Generate .hpp file
    cpp_lines = [
        "#pragma once",
        "#include <pch.hpp>",
        "#include <memory/patterns.hpp>",
        "",
        "inline auto fakeImportResolver = +[](std::uint64_t ordinal) -> std::uintptr_t {",
        "    switch (static_cast<int>(ordinal)) {",
    ]

    for idx, entry in enumerate(data, start=1):
        cpp_lines.append(f"        case {idx}: // {entry['symbol']}")
        cpp_lines.append(f"            return {build_pattern_call(entry)}")

    cpp_lines.extend(["        default: return 0x0;", "    }", "};"])

    cpp_output.parent.mkdir(parents=True, exist_ok=True)
    cpp_output.write_text("\n".join(cpp_lines), encoding="utf-8")

if __name__ == "__main__":
    main()