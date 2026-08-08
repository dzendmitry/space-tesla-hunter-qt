# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Scope restriction (read first)

This directory (`space_tesla_hunter/`) is the Qt client for the Space Tesla Hunter shooter game. It is **manual, agent-free coding only**:

- You may read and inspect any file here.
- You are **NOT ALLOWED TO MAKE ANY CHANGES** to this project's code, resources, or build files.
- The **only** exception is this `CLAUDE.md` file itself, which you may update.

This overrides any general instinct to fix bugs, refactor, or add features here. If asked to change this client, decline and explain that it is manual-only, and point to the sibling `space_tesla_hunter_server/` project (in the parent repo) for server-side changes.

## Build commands

This is a CMake + Qt 6 (>= 6.5) C++ project, developed via Qt Creator but buildable from the CLI. Required Qt modules: `Core`, `Widgets`, `Multimedia`.

```bash
# Configure (point CMAKE_PREFIX_PATH at your local Qt 6 install, e.g. ~/Qt/6.11.1/macos)
cmake -S . -B build -DCMAKE_PREFIX_PATH=~/Qt/6.11.1/macos -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build

# Run (macOS app bundle)
open build/space_tesla_hunter.app
```

There is no test suite and no lint config in this project (CTest's `Testing/` scaffold exists only because `qt_standard_project_setup()` enables it by default; no tests are registered).

## Architecture

Single-window Qt Widgets app (`SpaceTeslaHunter : QMainWindow`, built from `spaceteslahunter.ui`) that swaps a `QGraphicsView`'s scene between a menu and an active game:

- **`SpaceTeslaHunter`** (`spaceteslahunter.*`) — top-level window/controller. Owns the `QMediaPlayer` background music, the `MenuGraphicsScene`, and (while playing) the `GameEngine` + `GameGraphicScene`. Wires menu button signals to start/exit game, and `GameEngine::gameFinished` back to itself to show a victory/defeat `QMessageBox` and return to the menu.
- **`MenuGraphicsScene`** (`menugraphicsscene.*`) — a `QGraphicsScene` subclass that draws the menu background and hosts overlaid `QPushButton`s (start, start-coop [stub, not implemented], exit), emitting Qt signals on click.
- **`GameGraphicScene`** (`gamegraphicscene.*`) — the in-game `QGraphicsScene`; forwards `keyPressEvent`/`keyReleaseEvent` as signals (`keyPressed`/`keyReleased`) since input is handled by the engine, not the scene.
- **`GameEngine`** (`gameengine.*`) — the actual game loop and sole gameplay authority. Owns all `Ship`/`Bullet` instances directly (not via the scene) in `std::set` members, and drives everything through five independent `QTimer`s:
  - `worldTimer` (16ms/60 FPS) — moves ships/bullets, runs `damageLogic()`, checks win/loss via `checkGameStatus()`, emits `gameFinished`.
  - `playerShotTimer` (100ms) — spawns a player bullet while space is held.
  - `enemyShotTimer` (333ms) — each alive enemy has a probability (`ENEMY_SHIP_SHOT_PROB_PERCENT`) of shooting.
  - `clashTimer` (1000ms) — gates ship-to-ship collision damage to once per second via `clashAllowed`.
  - `enemiesGenTimer` (1000ms) — tops up enemy count toward `MAX_ALIVE_ENEMIES_AMOUNT`.
  - Player movement direction is tracked as a 4-bit bitmask (`direction`) set/cleared by `keyPressed`/`keyReleased` (arrow keys), rather than driven per-keystroke.
  - Collision/hit-testing uses `QGraphicsItem::boundingRegion` intersection, not the physics engine.
- **`Ship`** (`ship.*`) — a `QObject` + `QGraphicsPixmapItem`. Has `ShipType` (Player/Enemy), `ShipSide` (Local/Remote — the multiplayer hook), and `ShipState` (Alive/Damaged/Dead) driven by health thresholds (100/70/40/10/0), each level mapping to a different sprite via `shipTypeResourcesMap`.
- **`Bullet`** (`bullet.*`) — a `QGraphicsPixmapItem` with a `BulletType` (Player/Enemy) that fixes its damage and sprite via static maps.
- **`Resources`** (`resources.*`) — a lazily-constructed process-wide singleton (`GetInstance()`) caching all `QPixmap`s by `ResourceId`; must be initialized once with `ResourcesParams` (done in `SpaceTeslaHunter`'s constructor) before any other call site can use the parameterless `GetInstance()`. Resource file paths are baked into the Qt resource system via `qt_add_resources` in `CMakeLists.txt` and referenced with `qrc:` / `:/` paths.

### Notable design points

- `ShipSide::Remote` and the "coop" menu button/`startNewCoopGame` slot are stubs — multiplayer is planned but unimplemented; `GameEngine` currently only ever creates `ShipSide::Local` ships.
- Game object lifetime is manual (`new`/`delete`), not Qt parent-child ownership, for `Ship`/`Bullet`/`GameEngine`/`GameGraphicScene` — see `SpaceTeslaHunter::deleteGameObjects()` and `GameEngine`'s destructor.
- All gameplay tuning constants (speeds, damage values, spawn limits, shot probability) are free-standing `const int` globals near their related class, not a central config.
