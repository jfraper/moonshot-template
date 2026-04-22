#!/bin/sh

# Run the game.
# Usage: ./scripts/run.sh [--debug] [--release]
#
# Options:
#   --debug           Run Debug build (default)
#   --release         Run Release build
#
# Automatically detects the platform and looks in dist/<type>-<platform>/

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Derive project name from directory
PROJECT_NAME="$(basename "$PROJECT_ROOT")"

# Detect platform
case "$(uname)" in
  Darwin)           PLATFORM="macos" ;;
  Linux)            PLATFORM="linux" ;;
  MINGW*|MSYS*|CYGWIN*) PLATFORM="windows" ;;
  *)                PLATFORM="linux" ;;
esac

# Parse arguments (default: debug)
BUILD_TYPE="debug"
while [ "$#" -gt 0 ]; do
  case "$1" in
    --debug) BUILD_TYPE="debug" ;;
    --release) BUILD_TYPE="release" ;;
  esac
  shift
done

DIST_DIR="${PROJECT_ROOT}/dist/${BUILD_TYPE}-${PLATFORM}"

# Auto-setup: init submodule if engine not present
if [ ! -f "${PROJECT_ROOT}/engine/CMakeLists.txt" ]; then
  echo "Engine not found. Running setup..."
  "$SCRIPT_DIR/setup.sh"
  if [ $? -ne 0 ]; then
    echo "Error: Failed to initialize engine submodule."
    exit 1
  fi
fi

# Check for Debug executable (.out suffix)
if [ "$BUILD_TYPE" = "debug" ] && [ -x "${DIST_DIR}/${PROJECT_NAME}.out" ]; then
  echo "Running: dist/${BUILD_TYPE}-${PLATFORM}/${PROJECT_NAME}.out"
  cd "$DIST_DIR" && "./${PROJECT_NAME}.out"
  exit $?
fi

# Check for Release executable (no suffix)
if [ "$BUILD_TYPE" = "release" ] && [ -x "${DIST_DIR}/${PROJECT_NAME}" ]; then
  echo "Running: dist/${BUILD_TYPE}-${PLATFORM}/${PROJECT_NAME}"
  cd "$DIST_DIR" && "./${PROJECT_NAME}"
  exit $?
fi

echo "Error: No executable found in dist/${BUILD_TYPE}-${PLATFORM}/"
echo "Run ./scripts/build.sh first."
exit 1
