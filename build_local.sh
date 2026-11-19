#!/usr/bin/env bash
# Genesix by DSK - local build for Linux and macOS (generic, no personal paths).
# Usage: ./build_local.sh [Release|Debug] [package]
set -euo pipefail
cd "$(dirname "$0")"
CONFIG="${1:-Release}"
GX_DEPS="${GX_DEPS:-$HOME/.gx-deps}"
EXTRA=()
command -v ccache >/dev/null && EXTRA+=(-DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache)
PLATFORM=linux
if [ "$(uname)" = "Darwin" ]; then
  PLATFORM=mac
  EXTRA+=("-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64")
fi
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE="$CONFIG" "-DFETCHCONTENT_BASE_DIR=$GX_DEPS" "${EXTRA[@]}"
cmake --build build
if [ "$PLATFORM" = "linux" ] && [ -z "${DISPLAY:-}" ] && command -v xvfb-run >/dev/null; then
  xvfb-run -a ctest --test-dir build --output-on-failure
else
  ctest --test-dir build --output-on-failure
fi
if [ "${2:-}" = "package" ]; then
  VER=$(python3 -c "import re;print(re.search(r'project\(Genesix VERSION ([0-9.]+)',open('CMakeLists.txt').read()).group(1))")
  python3 scripts/package.py --platform "$PLATFORM" --build build --config "$CONFIG" --version "$VER"
fi
echo "VST3:       build/Genesix_artefacts/$CONFIG/VST3/Genesix by DSK.vst3"
[ "$PLATFORM" = "mac" ] && echo "AU:         build/Genesix_artefacts/$CONFIG/AU/Genesix by DSK.component"
echo "Standalone: build/Genesix_artefacts/$CONFIG/Standalone/"
