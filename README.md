# Moonshot Game Template

Template for creating games with the [Moonshot](https://github.com/jfraper/moonshot) engine.

## Quick Start

```bash
# 1. Create your project from this template
gh repo create my-game --template jfraper/moonshot-template --clone
cd my-game

# 2. Initialize the engine submodule
git submodule update --init --recursive

# 3. Build and run
./scripts/build.sh --run
```

## Setup

1. Rename the project in `CMakeLists.txt` (change `mygame` to your project name)
2. Edit `src/main.cpp` to set your game title and window size
3. Put game assets in `assets/`

## Scripts

| Command | Description |
|---------|-------------|
| `./scripts/build.sh --run` | Debug build and run (with hot reload) |
| `./scripts/build.sh --release --run` | Release build and run |
| `./scripts/build.sh --clean --run` | Clean rebuild |
| `./scripts/build.sh --update-engine` | Update engine to latest version |
| `./scripts/run.sh` | Run last build |

## Project Structure

```
my-game/
  engine/          # Moonshot engine (git submodule)
  src/
    main.cpp       # Game entry point
  assets/          # Game assets (textures, sounds, etc.)
  scripts/         # Build and run scripts
  dist/            # Build output (auto-generated)
```

## Updating the Engine

```bash
# Update to latest engine
./scripts/build.sh --update-engine

# Or pin to a specific version
cd engine
git checkout v0.17.0
cd ..
git add engine
git commit -m "pin engine to v0.17.0"
```
