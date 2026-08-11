# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Scope restriction (read first)

This directory (`space_tesla_hunter/`) is the Qt client for the Space Tesla Hunter shooter game. It is **manual, agent-free coding only**:

- You may read and inspect any file here.
- You are **NOT ALLOWED TO MAKE ANY CHANGES** to this project's code, resources, or build files.
- The **only** exceptions are this `CLAUDE.md` and `README.md`, which you may write.

This overrides any general instinct to fix bugs, refactor, or add features here. If asked to change this client, decline and explain that it is manual-only, and point to the sibling `space_tesla_hunter_server/` project (in the parent repo) for server-side changes.

Note this directory is its own Git repository — the parent repo's `.gitignore` excludes it, so `git` commands run here act on the client only.

## Build commands

CMake (>= 3.19) + Qt 6 (>= 6.5) C++ project, developed via Qt Creator but buildable from the CLI. Required Qt modules: `Core`, `Widgets`, `Multimedia`.

```bash
# Configure (point CMAKE_PREFIX_PATH at your local Qt 6 install, e.g. ~/Qt/6.11.1/macos)
cmake -S . -B build -DCMAKE_PREFIX_PATH=~/Qt/6.11.1/macos -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build

# Run (macOS app bundle)
open build/space_tesla_hunter.app
```

Qt Creator builds into `build/Qt_<version>_for_macOS_<config>/` instead; both trees are gitignored.

There is no test suite and no lint config (CTest's `Testing/` scaffold exists only because `qt_standard_project_setup()` enables it by default; no tests are registered). Verification is by running the app.

`Qt::Network` is used (coop mode) but not named in `target_link_libraries` — it resolves transitively through `Qt::Multimedia`.

Coop mode needs the Go server from the parent repo running locally; see `space_tesla_hunter_server/README.md` for how to start it.

## Architecture

Single-window Qt Widgets app (`SpaceTeslaHunter : QMainWindow`, built from `spaceteslahunter.ui`, fixed 600x800) that swaps a `QGraphicsView`'s scene between a menu and an active game. `README.md` covers the same ground for a human audience, including the wire protocol in more detail.

- **`SpaceTeslaHunter`** (`spaceteslahunter.*`) — top-level window/controller. Owns the `QMediaPlayer` background music, the `MenuGraphicsScene`, and (while playing) a `GameEngineBase*` + `GameGraphicScene`. `startNewGame(GameMode)` picks the engine subclass, wires input/result/health connections, then calls `StartNewGame()`.
- **`MenuGraphicsScene`** (`menugraphicsscene.*`) — draws the menu background and hosts overlaid `QPushButton`s (start, start-coop, exit), emitting Qt signals on click.
- **`GameGraphicScene`** (`gamegraphicscene.*`) — the in-game `QGraphicsScene`; forwards `keyPressEvent`/`keyReleaseEvent` as `keyPressed`/`keyReleased` signals since input is handled by the engine, not the scene.
- **`GameEngineBase`** (`gameenginebase.*`) — abstract base for both modes. Holds the `gameScene` pointer and the `actions` bitmask, declares the shared signals (`gameFinished(GameStatus)`, `playerHealthChanged(int)`, `gameError(QString)`) and the pure-virtual `StartNewGame()`. Key events set/clear bits in `actions`: up=1, right=2, down=4, left=8, fire=16. That same byte is what movement reads locally *and* what ships out on the wire in coop.
- **`GameEngine`** (`gameengine.*`) — single-player mode, the sole gameplay authority. Owns all `Ship`/`Bullet` instances directly (not via the scene) in `std::set` members, driven by five independent `QTimer`s:
  - `worldTimer` (16ms/60 FPS) — moves ships/bullets, runs `damageLogic()`, checks win/loss via `checkGameStatus()`, emits `gameFinished`.
  - `playerShotTimer` (100ms) — spawns a player bullet while space is held.
  - `enemyShotTimer` (333ms) — each alive enemy has a probability (`ENEMY_SHIP_SHOT_PROB_PERCENT`) of shooting.
  - `clashTimer` (1000ms) — gates ship-to-ship collision damage to once per second via `clashAllowed`.
  - `enemiesGenTimer` (1000ms) — tops up enemy count toward `MAX_ALIVE_ENEMIES_AMOUNT`.
  - Collision/hit-testing uses `QGraphicsItem::boundingRegion` intersection, not a physics engine.
- **`GameEngineNet`** (`gameenginenet.*`) — coop mode. The server owns the simulation; this class only sends input and renders snapshots. See below.
- **`ApiError` / `JoinResponse`** (`apiresponses.*`) — hand-written JSON DTOs for the HTTP control plane, mirroring the server's response shapes.
- **`Ship`** (`ship.*`) — a `QObject` + `QGraphicsPixmapItem`. Has `ShipType` (Player/Enemy), `ShipSide` (Local/Remote), and `ShipState` (Alive/Damaged/Dead) driven by health thresholds (100/70/40/10/0), each level mapping to a different sprite via `shipTypeResourcesMap`. Three enemy variants, each with its own damage sprite set.
- **`Bullet`** (`bullet.*`) — a `QGraphicsPixmapItem` with a `BulletType` (Player/Enemy) that fixes its damage and sprite via static maps.
- **`Resources`** (`resources.*`) — lazily-constructed process-wide singleton (`GetInstance()`) caching all `QPixmap`s by `ResourceId`; must be initialized once with `ResourcesParams` (done in `SpaceTeslaHunter`'s constructor) before any other call site can use the parameterless `GetInstance()`. Resource paths are baked into the Qt resource system via `qt_add_resources` in `CMakeLists.txt` and referenced with `qrc:` / `:/` paths.

### Coop networking (`GameEngineNet`)

`StartNewGame()` runs the HTTP join **synchronously**, spinning a nested `QEventLoop` on `POST {HTTP_SERVER}/api/v1/rooms/join` and parsing `roomId`, `playerSlot`, `sessionToken`, `gameConfig` into `joinResponse`. Failure emits `gameError` and the match never starts. Then:

- **Outbound** every 60ms (`udpSocketSender`): 19-byte big-endian header (`msgType`, `version`, `roomId`, `playerSlot`, `sessionToken`, `sequence`) + one `actions` byte, hand-packed via `QDataStream` in `Input::pack()`. A level signal, not an event stream — each packet overwrites the last.
- **Inbound** (`processPendingDatagrams`): `Snapshot::unpack` reads the header, status, 3-slot roster, then ship and bullet records. Ships/bullets are reconciled by **server id** into `std::map`s — unknown id constructs the item and adds it to the scene, known id updates position/health, anything absent from the snapshot is removed and deleted. Disappearance *is* the death signal.
- Ownership comes off the ship record: `owner == 0xFF` (`SHIP_RECORD_ENEMY`) is an enemy, `owner == joinResponse.playerSlot` is the local player (that ship grabs the keyboard and drives `playerHealthChanged`), anything else is a teammate rendered as `ShipSide::Remote`.
- A snapshot whose status is not `InProgress` disconnects the socket connections, stops the timers and emits `gameFinished`.
- Before the first snapshot, `startGameSec` counts `DELAY_BEFORE_START_SEC` (30) down through `playerHealthChanged`, so the health LCD doubles as a lobby countdown.

Server endpoints are compile-time constants at the top of `gameenginenet.h` (`HTTP_SERVER`, `UDP_SERVER`, `UDP_PORT`) — retargeting means editing and rebuilding.

### Notable design points

- Game object lifetime is manual (`new`/`delete`), not Qt parent-child ownership, for `Ship`/`Bullet`/engines/`GameGraphicScene` — see `SpaceTeslaHunter::deleteGameObjects()` and each engine's destructor.
- `deleteGameObjects()` is deliberately commented out in `SpaceTeslaHunter::gameFinished`, so a finished match's engine and scene outlive it.
- `gameError` is emitted by `GameEngineNet` but nothing connects to it — a failed join is currently silent to the player.
- All gameplay tuning constants (speeds, damage values, spawn limits, shot probability, countdown) are free-standing `const int` globals near their related class, not a central config. In coop they are advisory; the server's values decide outcomes.
- Wire-format structs are packed/unpacked by hand in `gameenginenet.cpp` with explicit `BigEndian` `QDataStream`s — field order there is the protocol contract with the server, not an implementation detail.
