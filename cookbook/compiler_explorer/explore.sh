#!/usr/bin/env bash
clear

# Configuration
COMPILER="${COMPILER:-g++}"
COMPILER_OPTIONS="${COMPILER_OPTIONS:--std=c++23 -O0 -fno-asynchronous-unwind-tables -fno-dwarf2-cfi-asm -masm=intel}"

FILE="${1:-$VOY_EVENT_PATH}"
if [ -z "$FILE" ]; then
    echo "Usage: $0 <file.cpp>"
    exit 1
fi

TMP_ASM=$(mktemp "/tmp/$(basename "$FILE" .cpp).XXXXXX" --suffix=.s)

if ! $COMPILER -S $COMPILER_OPTIONS -o "$TMP_ASM" "$FILE"; then
    rm -f "$TMP_ASM"
    exit 1
fi

CAT_CMD="cat"
if command -v batcat >/dev/null 2>&1; then
    CAT_CMD="batcat --language asm --style plain --color=always"
fi

c++filt < "$TMP_ASM" | awk '
/^[[:space:]]+\./ {next}
/^\.L[A-Z]/ {next}
/^[0-9]+:/ {next}
/^[[:space:]]*#[[:space:]]*(GNU|compiled|GGC|options|APP|NO_APP)/ {next}
/^[[:space:]]*$/ {next}
{print}' | $CAT_CMD

rm -f "$TMP_ASM"
