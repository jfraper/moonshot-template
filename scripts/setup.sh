#!/bin/sh

# Initialize or update the moonshot engine submodule.
# Usage: ./scripts/setup.sh
#
# This script is also called automatically by build.sh when the
# engine/ directory is missing or --update-engine is passed.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT" || exit 1

if [ ! -f "engine/CMakeLists.txt" ]; then
  echo "Initializing engine submodule..."
  git submodule update --init --recursive
else
  echo "Updating engine submodule..."
  git submodule update --recursive --remote
fi

if [ $? -ne 0 ]; then
  echo "Error: Failed to initialize/update engine submodule."
  exit 1
fi

echo "Engine ready."
