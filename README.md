# Space Tesla Hunter — client

Qt 6 desktop client for Space Tesla Hunter, a 2D top-down shooter. It ships two
game modes behind the same window and the same sprites:

- **Single player** — the whole simulation runs locally in `GameEngine`. No
  network, no server, no configuration.
- **Coop** — the client becomes a thin input sender and renderer. The
  authoritative simulation lives in the Go server (`space_tesla_hunter_server/`
  in the parent repository); this side only forwards keystrokes and draws the
  snapshots that come back.

The two modes share the scene, the sprites and the input handling, and differ
only in who decides where things are — which is why both engines derive from
`GameEngineBase`.

## Build and run

CMake 3.19+ and Qt 6.5+ with the `Core`, `Widgets` and `Multimedia` modules.
Developed in Qt Creator, but the CLI build is equivalent:

```sh
# Point CMAKE_PREFIX_PATH at your Qt 6 install
cmake -S . -B build -DCMAKE_PREFIX_PATH=~/Qt/6.11.1/macos -DCMAKE_BUILD_TYPE=Debug
cmake --build build

open build/space_tesla_hunter.app        # macOS
```

Qt Creator's own builds land in `build/Qt_<version>_for_macOS_<config>/` instead.

`Qt::Network` is not named in `target_link_libraries` — the coop mode's
`QUdpSocket` and `QNetworkAccessManager` resolve through `Qt::Multimedia`'s
transitive dependency on it.

There is no test suite and no lint configuration. The `Testing/` directory under
`build/` exists only because `qt_standard_project_setup()` enables CTest by
default; no tests are registered.

## Playing

| Key | Action |
|---|---|
| Arrow keys | Move |
| Space (hold) | Fire |

The window is fixed at 600×800 — the scene is sized to the view, and neither
mode rescales. The `QLCDNumber` in the bottom-left shows player health during a
match; in coop it doubles as the pre-game countdown, counting down from 30
seconds until the first snapshot arrives.

Reaching a win or a loss pops a victory/defeat image and returns to the menu.

## Architecture

```
SpaceTeslaHunter (QMainWindow)      window, music, scene swapping
   ├── MenuGraphicsScene            background + overlaid QPushButtons
   └── GameGraphicScene             forwards key events as signals
           ↑
       GameEngineBase               input bitmask, gameFinished/playerHealthChanged
           ├── GameEngine           local simulation (single player)
           └── GameEngineNet        HTTP join + UDP snapshot rendering (coop)
```

`SpaceTeslaHunter` owns one `QGraphicsView` and swaps its scene between the menu
and an active game. Starting a match constructs a fresh `GameGraphicScene` plus
whichever engine the mode calls for, wires four connections (two for input, one
for the result, one for the health display) and calls `StartNewGame()`.

`GameGraphicScene` handles no gameplay itself: it re-emits `keyPressEvent` /
`keyReleaseEvent` as `keyPressed` / `keyReleased` signals, because input belongs
to the engine. `GameEngineBase` turns those into a 5-bit `actions` mask — up,
right, down, left, fire — which is both what local movement reads and, byte for
byte, what goes out on the wire in coop.

Game objects (`Ship`, `Bullet`, engines, scenes) are managed with manual
`new`/`delete` rather than Qt parent-child ownership.

### Single player — `GameEngine`

The engine owns every `Ship` and `Bullet` directly (in `std::set`s, not via the
scene) and drives the game from five independent `QTimer`s:

| Timer | Interval | Responsibility |
|---|---|---|
| `worldTimer` | 16ms | Move everything, apply damage, check win/loss |
| `playerShotTimer` | 100ms | Spawn a player bullet while fire is held |
| `enemyShotTimer` | 333ms | Each live enemy fires with `ENEMY_SHIP_SHOT_PROB_PERCENT` probability |
| `clashTimer` | 1000ms | Gate ship-to-ship collision damage to once per second |
| `enemiesGenTimer` | 1000ms | Top the field back up to `MAX_ALIVE_ENEMIES_AMOUNT` |

Hit testing intersects `QGraphicsItem::boundingRegion` — there is no physics
engine.

### Coop — `GameEngineNet`

`StartNewGame()` does the HTTP join first, synchronously: it `POST`s to
`/api/v1/rooms/join` and spins a nested `QEventLoop` until the reply lands, then
parses the `roomId`, `playerSlot`, `sessionToken` and `gameConfig` out of the
JSON (`apiresponses.h`). Any failure emits `gameError` and the match never
starts.

From then on the client is stateless about gameplay:

- **Outbound**, every 60ms: a 19-byte big-endian header (`msgType`, `version`,
  `roomId`, `playerSlot`, `sessionToken`, `sequence`) plus one byte of `actions`.
  It is a *level* signal — each packet overwrites the last, so sending more of
  them buys neither speed nor bullets, and the steady cadence is what covers UDP
  loss and keeps the server's disconnect timer quiet.
- **Inbound**: snapshots carrying room status, the three-slot roster, and ship
  and bullet records. Ships and bullets are reconciled by server id into
  `std::map`s — an unknown id constructs the item and adds it to the scene, a
  known id just gets its position and health updated, and anything absent from
  the snapshot is deleted. Disappearance *is* the death signal.

Ownership is read off each ship record: `owner == 0xFF` marks an enemy,
`owner == playerSlot` marks the local player (which is the ship that grabs the
keyboard and feeds the health display), and anything else is a teammate,
rendered as `ShipSide::Remote`. A snapshot whose status is no longer
`InProgress` tears down the socket and timers and ends the match.

Server addresses are compile-time constants at the top of `gameenginenet.h`
(`http://127.0.0.1:8080` and `127.0.0.1:8081`) — pointing the client at another
host means editing them and rebuilding.

## Sprites and resources

`Resources` is a process-wide lazy singleton caching every `QPixmap` by
`ResourceId`. It must be initialized once with `ResourcesParams` — done in
`SpaceTeslaHunter`'s constructor — before any other call site can use the
parameterless `GetInstance()`.

Ships change sprite as they take damage: health crosses 100 / 70 / 40 / 10 / 0
and `shipTypeResourcesMap` supplies the matching image, so damage is legible
without a health bar. There are three enemy variants, each with its own five-step
damage set. All assets are baked into the Qt resource system by `qt_add_resources`
in `CMakeLists.txt` and referenced as `:/…` or `qrc:/…`.

## Tuning constants

Gameplay values are free-standing `const int` globals next to the class they
affect, not a central config. In coop they are advisory — the server's own
values decide what actually happens.

| Constant | Value | Where |
|---|---|---|
| `PLAYER_SHIP_SPEED` | 10 | `ship.h` |
| `ENEMY_SHIP_SPEED` | 1 | `ship.h` |
| `PLAYER_SHIP_CLASH_DAMAGE` | 10 | `ship.h` |
| `ENEMY_SHIP_CLASH_DAMAEGE` | 35 | `ship.h` |
| `ENEMY_SHIP_SHOT_PROB_PERCENT` | 30 | `ship.h` |
| `DEFAULT_BULLET_SPEED` | 5 | `bullet.h` |
| Player / enemy bullet damage | 35 / 5 | `bullet.h` |
| `MAX_ALIVE_ENEMIES_AMOUNT` | 5 | `gameengine.h` |
| `DELAY_BEFORE_START_SEC` | 30 | `gameenginenet.h` |

## Known rough edges

- `deleteGameObjects()` is commented out in `SpaceTeslaHunter::gameFinished`, so
  the engine and scene of a finished match survive until the window closes;
  starting another match leaves the previous pair behind.
- The coop join blocks the UI thread on a nested event loop while the HTTP
  request is in flight.
- `gameError` is emitted by `GameEngineNet` but nothing connects to it, so a
  failed join is silent from the player's side.

## Development policy

This directory is **manual, agent-free coding**: coding agents may read it but
must not modify its code, resources or build files. See [`CLAUDE.md`](CLAUDE.md).
The server side, which has no such restriction, lives in
`space_tesla_hunter_server/` in the parent repository.
