import json
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent

INPUT_JSON = BASE_DIR / "../patterns/patterns.json"
DEF_OUTPUT = BASE_DIR / "../../sdk/sdk.def"
CPP_OUTPUT = BASE_DIR / "../patterns/resolver.hpp"

LIB_NAME = "sdk.dll"

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
        return (
            f'selaura::pattern<"{signature}", '
            f'selaura::signature_offset<selaura::deref, {offset}>>::resolve();'
        )

    if has_ref:
        offset = entry["ref"]
        return (
            f'selaura::pattern<"{signature}", '
            f'selaura::signature_offset<selaura::ref, {offset}>>::resolve();'
        )

    return f'selaura::pattern<"{signature}">::resolve();'


def main():
    data = json.loads(INPUT_JSON.resolve().read_text(encoding="utf-8"))

    def_lines = [
        f"LIBRARY {LIB_NAME}",
        "EXPORTS",
    ]

    for idx, entry in enumerate(data, start=1):
        def_lines.append(f"    {entry['symbol']} @{idx} NONAME")

    DEF_OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    DEF_OUTPUT.write_text("\n".join(def_lines), encoding="utf-8")

    cpp_lines = [
        "#pragma once",
        "#include <pch.hpp>",
        "",
        "// Auto-generated function",
        "inline auto fakeImportResolver = +[](std::uint64_t ordinal) -> std::uintptr_t {",
        "    switch (static_cast<int>(ordinal)) {",
    ]

    for idx, entry in enumerate(data, start=1):
        symbol = entry["symbol"]

        call = build_pattern_call(entry)

        cpp_lines.append(f"        case {idx}: // {symbol}")
        cpp_lines.append(f"            return {call}")

    cpp_lines.extend([
        "        default:",
        "            return 0x0;",
        "    }",
        "};",
    ])

    CPP_OUTPUT.write_text("\n".join(cpp_lines), encoding="utf-8")

    print(f"[+] Generated {DEF_OUTPUT.resolve()}")
    print(f"[+] Generated {CPP_OUTPUT.resolve()}")


if __name__ == "__main__":
    main()