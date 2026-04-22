#!/bin/sh

# Build the project.
# Usage: ./scripts/build.sh [--run] [--release] [--clean] [--update-engine]
#
# Options:
#   --run             Run after building
#   --release         Release build (default: Debug with hot reload)
#   --clean           Clean build directory before building
#   --update-engine   Pull latest engine before building
#
# Build outputs go to dist/<build_type>-<platform>/ (e.g., dist/debug-macos/)
#
# Examples:
#   ./scripts/build.sh --run                        # Debug build and run
#   ./scripts/build.sh --release                    # Release build
#   ./scripts/build.sh --clean --run                # Clean, build, and run
#   ./scripts/build.sh --update-engine --run        # Update engine, build, and run

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ENGINE_DIR="${PROJECT_ROOT}/engine"
BUILD_DIR="${PROJECT_ROOT}/build"

# Derive project name from directory
PROJECT_NAME="$(basename "$PROJECT_ROOT")"

BUILD_TYPE="Debug"
RUN_AFTER_BUILD=false
CLEAN=false
UPDATE_ENGINE=false

OS_NAME=$(uname)

# Detect platform
case "$OS_NAME" in
  Darwin)
    PLATFORM="macos"
    SDK_PATH=$(xcrun --sdk macosx --show-sdk-path)
    OSX_DEPLOYMENT_TARGET=$(sw_vers -productVersion | cut -d '.' -f 1-2)
    ;;
  Linux)
    PLATFORM="linux"
    SDK_PATH=""
    OSX_DEPLOYMENT_TARGET=""
    ;;
  MINGW*|MSYS*|CYGWIN*)
    PLATFORM="windows"
    SDK_PATH=""
    OSX_DEPLOYMENT_TARGET=""
    ;;
  *)
    PLATFORM="linux"
    SDK_PATH=""
    OSX_DEPLOYMENT_TARGET=""
    ;;
esac

# Parse arguments
while [ "$#" -gt 0 ]; do
  case "$1" in
    --run) RUN_AFTER_BUILD=true ;;
    --release) BUILD_TYPE="Release" ;;
    --clean) CLEAN=true ;;
    --update-engine) UPDATE_ENGINE=true ;;
  esac
  shift
done

BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
DIST_DIR="${PROJECT_ROOT}/dist/${BUILD_TYPE_LOWER}-${PLATFORM}"

# Auto-setup: init submodule if engine not present
if [ ! -f "$ENGINE_DIR/CMakeLists.txt" ]; then
  echo "Engine not found. Running setup..."
  "$SCRIPT_DIR/setup.sh"
  if [ $? -ne 0 ]; then
    echo "Error: Failed to initialize engine submodule."
    exit 1
  fi
elif [ "$UPDATE_ENGINE" = true ]; then
  "$SCRIPT_DIR/setup.sh"
fi

# Clean
if [ "$CLEAN" = true ]; then
  rm -rf "$BUILD_DIR"
  rm -rf "$DIST_DIR"
fi

mkdir -p "$BUILD_DIR"

BUILD_START_TIME=$(date +%s)

# Pre-build external dependencies (engine script)
cd "$ENGINE_DIR" && ./scripts/build-external.sh "$BUILD_TYPE" "$OSX_DEPLOYMENT_TARGET"
if [ $? -ne 0 ]; then
  echo "====> External dependency build failed!"
  exit 1
fi

cd "$BUILD_DIR" || exit 1

# Determine CPU count
case "$OS_NAME" in
  Darwin) NPROC=$(sysctl -n hw.ncpu) ;;
  *) NPROC=$(nproc 2>/dev/null || echo 4) ;;
esac

# CMake configure + build
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [ "$PLATFORM" = "macos" ]; then
  CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_OSX_SYSROOT=$SDK_PATH -DCMAKE_OSX_DEPLOYMENT_TARGET=$OSX_DEPLOYMENT_TARGET"
fi

cmake "$PROJECT_ROOT" $CMAKE_ARGS && make -j${NPROC}
BUILD_STATUS=$?

cd "$PROJECT_ROOT" || exit 1

if [ "$BUILD_STATUS" -eq 0 ]; then
  BUILD_END_TIME=$(date +%s)
  BUILD_DURATION=$((BUILD_END_TIME - BUILD_START_TIME))
  echo "====> Build complete! (${BUILD_TYPE}) - ${BUILD_DURATION}s"
  echo "      Output: dist/${BUILD_TYPE_LOWER}-${PLATFORM}/"

  if [ "$RUN_AFTER_BUILD" = true ]; then
    "$SCRIPT_DIR/run.sh" --${BUILD_TYPE_LOWER}
  fi
else
  echo "====> Build FAILED!"
  exit 1
fi
