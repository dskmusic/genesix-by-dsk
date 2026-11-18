#!/usr/bin/env bash
# Genesix by DSK - local Linux build (generic, no personal paths).
# Usage: ./build_local.sh [Release|Debug] [package]
set -euo pipefail
cd "$(dirname "$0")"
CONFIG="${1:-Release}"
GX_DEPS="${GX_DEPS:-$HOME/.gx-deps}"
LAUNCHER=""
command -v ccache >/dev/null && LAUNCHER="-DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE="$CONFIG" -DFETCHCONTENT_BASE_DIR="$GX_DEPS" $LAUNCHER
cmake --build build
ctest --test-dir build --output-on-failure
if [ "${2:-}" = "package" ]; then python3 scripts/package.py --platform linux --build build; fi
echo "VST3:       build/Genesix_artefacts/$CONFIG/VST3/Genesix by DSK.vst3"
echo "Standalone: build/Genesix_artefacts/$CONFIG/Standalone/Genesix by DSK"
