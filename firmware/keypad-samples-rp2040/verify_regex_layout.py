#!/usr/bin/env python3
from pathlib import Path

repo = Path(__file__).resolve().parents[2]

definition = repo / "regular expressions start end all any.txt"
renderer = repo / "render-keypads" / "src" / "Renderer.idr"
firmware = Path(__file__).resolve().with_name("main.c")

expected_meanings = [
    "START OF LINE",
    "END OF LINE",
    "END OF SLURP",
    "maybe (ok if not?)",
    'big capture ("greedy")',
    'small capture ("non greedy")',
    "group of characters in exactly this order ()",
    "any character from this list []",
    "letter",
    "number",
    "non weird character",
    "weird character",
    "incantation runes (control characters)",
]

lines = [line.strip() for line in definition.read_text(encoding="utf-8").splitlines()]
meaning_lines = [line for line in lines[3:] if line]
if meaning_lines != expected_meanings:
    raise SystemExit(
        "Regex text definition no longer matches the 13-key contract:\n"
        + "\n".join(f"  {index}: {meaning}" for index, meaning in enumerate(meaning_lines))
    )

renderer_text = renderer.read_text(encoding="utf-8")
renderer_section = renderer_text.split("regex : Board", 1)[1].split("separation : Board", 1)[0]
renderer_markers = [
    'three "START" "OF" "LINE"',
    'three "END" "OF" "LINE"',
    'three "END" "OF" "SLURP"',
    'two "MAYBE" "OK IF NOT?"',
    'two "BIG CAPTURE" "GREEDY"',
    'two "SMALL CAPTURE" "NON GREEDY"',
    'three "EXACT ORDER" "GROUP" "()"',
    'three "ANY FROM" "LIST" "[]"',
    'one "LETTER"',
    'one "NUMBER"',
    'two "NON WEIRD" "CHARACTER"',
    'two "WEIRD" "CHARACTER"',
    'two "INCANTATION RUNES" "CONTROL CHARACTERS"',
]

position = -1
for marker in renderer_markers:
    next_position = renderer_section.find(marker, position + 1)
    if next_position < 0:
        raise SystemExit(f"Renderer Regex layout is missing or reorders: {marker}")
    position = next_position

firmware_text = firmware.read_text(encoding="utf-8")
regex_firmware = firmware_text.split("#elif KEYPAD_LAYOUT == LAYOUT_REGEX", 1)[1].split(
    "#elif KEYPAD_LAYOUT == LAYOUT_PASTEBINS", 1
)[0]
keymap = regex_firmware.split("static const key_binding_t keymap[KEY_COUNT] = {", 1)[1].split("};", 1)[0]
expected_bindings = [
    "macro_start_line",
    "macro_end_line",
    "macro_end_slurp",
    "macro_maybe",
    "macro_big_capture",
    "macro_small_capture",
    "macro_exact_order_group",
    "macro_any_from_list",
    "macro_letter",
    "macro_number",
    "macro_non_weird",
    "macro_weird",
    "macro_control_characters",
]

position = -1
for binding in expected_bindings:
    marker = f"BIND({binding})"
    next_position = keymap.find(marker, position + 1)
    if next_position < 0:
        raise SystemExit(f"Regex firmware keymap is missing or reorders: {marker}")
    position = next_position

if keymap.count("BIND(") != 13:
    raise SystemExit("Regex firmware keymap must contain exactly 13 active bindings")
if keymap.count("UNUSED_BINDING") != 3:
    raise SystemExit("Regex firmware keymap must leave exactly three positions unused")

print("Regex definition, renderer, and firmware agree on 13 row-major meanings")
