# OpenMidgard

OpenMidgard is an open source recreation of the Ragnarok Online client.

The current direction of the project is intentionally biased toward the pre-Renewal 2008-era client experience: classic presentation, classic game flow, and packet behavior aligned to that period rather than later Renewal-era client behavior.

This repository is focused on rebuilding the client as source code, modernizing the renderer and platform layer where needed, and keeping the gameplay/client feel anchored to the classic 2008 style.

## Status

- Open source Ragnarok Online client recreation in active development.
- Current work includes the world renderer, UI systems, networking, effects, and modern rendering backends.
- The project currently prefers the 2008 pre-Renewal client style and behavior.

## Packet Version Target

The current packet target is:

- `packet_ver 23`
- `2008-09-10aSakexe`

More specifically, this is the intended target for client-originating map-server packets.

## Progress Video

Current progress showcase:

- https://www.youtube.com/watch?v=ApS1wgUmgzE

## Repository Layout

- `src/` — client source code
- `cmake/` — CMake helpers and deployment scripts
- `docs/` — project notes and focused implementation docs
- `third_party/` — third-party notices and bundled support files

## Prerequisites

At a minimum you will need:

- CMake 3.20 or newer
- A C++17 compiler
- Git
- A legally obtained Ragnarok Online data/runtime setup for testing

For runtime asset discovery, the client looks for things like `data.grf`, `sdata.grf`, `adata.grf`, `data/clientinfo.xml`, and `data/` near the executable or current working directory. You can also point it at a custom runtime root with:

- `OPEN_MIDGARD_DATA_DIR=/path/to/runtime`

You can persist the same setting in `open-midgard.ini`:

```ini
[Data]
RuntimeRoot=E:\Ragnarok\Runtime
```

If your GRFs live outside the runtime root or executable directory, set a separate GRF search path:

- `OPEN_MIDGARD_GRF_DIR=/path/to/grfs`

Or in `open-midgard.ini`:

```ini
[Data]
GrfRoot=E:\Ragnarok\Client
```

When `data.grf`, `sdata.grf`, `adata.grf`, `data_hp.grf`, `event.grf`, or `fdata.grf` are missing beside the executable, OpenMidgard now falls back to `GrfRoot` for those archives while still loading local `data\` overrides such as Sabine's alpha/beta client files. The runtime also understands the 2001 Alpha `data.grf` trailer/table layout used by Sabine-compatible Alpha client installs.

Legacy UI skins can also be selected from the original Ragnarok-style skin folders beside the executable. OpenMidgard scans both `skin\` and `skins\`, and the Options window can switch the active skin at runtime.

```ini
[UI]
Skin=default
```

OpenMidgard checks `<root>\<Skin>\...` first, then `<root>\default\...`, and finally the normal archive/data UI paths, where `<root>` is either `skin` or `skins`. That matches drop-in custom RO skin packs that provide assets such as `basic_interface\*.bmp`, `login_interface\*.bmp`, and related UI files.

If `clientinfo.xml` / `sclientinfo.xml` are absent, OpenMidgard falls back to a direct account-server endpoint. It reads these sources in order of precedence:

1. `OPEN_MIDGARD_AUTH_HOST`, `OPEN_MIDGARD_AUTH_PORT`, `OPEN_MIDGARD_PACKET_VERSION`
2. `client.cfg` beside the executable or runtime root, using AppleClient-style keys such as `auth_host=127.0.0.1`, `auth_port=6900`, `packet_version=200`
3. `open-midgard.ini`

For most setups, use a single client preset and only set the account endpoint separately:

```ini
[Client]
Profile=auto

[Network]
AuthHost=127.0.0.1
AuthPort=6900
```

Supported `Client.Profile` values include:

- `auto` for the default packet_ver 23 / 2008-era path
- `packetver22`
- `legacy0072`
- `alpha`
- `beta1`
- `beta2`

Legacy configs that only set `Network.PacketVersion` still work as a compatibility fallback. In particular, Sabine-style values such as `200` and `300` now feed the newer `[Packets]` and `[Login]` runtime selection automatically. The older preset spellings such as `sabinealpha`, `sabinebeta1`, `sabinebeta2`, and `sabine` are still accepted as aliases.

When `clientinfo.xml` contains multiple `<connection>` entries, each connection can also carry its own runtime overrides. OpenMidgard applies the selected connection's overrides before falling back to `open-midgard.ini`.

Supported per-connection XML tags include:

- `<clientprofile>` or `<profile>` with values such as `rathena`, `eathena`, `alpha`, `beta1`, `beta2`, `packetver22`, or `packetver23`
- `<charlistreceivelayout>`
- `<mapsendprofile>`
- `<mapgameplaysendprofile>`
- `<zoneconnectprofile>`
- `<accountloginpacket>`
- `<clientdateoverride>`
- `<clienthashoverridemd5>`
- `<clienthashsourceexe>`
- `<clienttypeoverride>`
- `<clientversionoverride>`

Example:

```xml
<connection>
  <display>eAthena Test</display>
  <address>127.0.0.1</address>
  <port>6900</port>
  <version>18</version>
  <profile>eathena</profile>
</connection>

<connection>
  <display>Sabine Beta1</display>
  <address>127.0.0.1</address>
  <port>6900</port>
  <version>200</version>
  <profile>beta1</profile>
</connection>
```

If you need a nonstandard server layout, the fine-grained overrides are still available:

- `[Packets] CharListReceiveLayout`
- `[Packets] MapSendProfile`
- `[Packets] MapGameplaySendProfile`
- `[Packets] ZoneConnectProfile`
- `[Login] AccountLoginPacket`
- `[Login] ClientDateOverride`
- `[Login] ClientHashOverrideMd5`
- `[Login] ClientHashSourceExe`
- `[Login] ClientTypeOverride`
- `[Login] ClientVersionOverride`

```ini
[Network]
AuthHost=127.0.0.1
AuthPort=6900
PacketVersion=200
```

If you want live debug output in a console window while running the Windows client, enable:

- `OPEN_MIDGARD_LOG_CONSOLE=1`

Or in `open-midgard.ini`:

```ini
[Logging]
Console=1
```

## Building On Windows

Windows currently has the most complete build flow.

### Option 1: Visual Studio 2022

Available presets:

- `vs2022-win32`
- `vs2022-x64`

Configure and build:

```powershell
cmake --preset vs2022-win32
cmake --build --preset build-release --config Release
```

Or for x64:

```powershell
cmake --preset vs2022-x64
cmake --build --preset build-release-x64 --config Release
```

Notes:

- Native Direct3D 11 and Direct3D 12 backends are only enabled on Windows MSVC builds.
- If you do not want the executable copied to `D:/Spel/OldRO`, configure with `-DRO_ENABLE_DEV_DEPLOY=OFF`.

### Option 2: MinGW + Qt 6

The repository includes a ready-made preset for the current Qt-based runtime path:

- `mingw-qt-x64`

Configure and build:

```powershell
cmake --preset mingw-qt-x64
cmake --build --preset build-mingw-qt-x64
```

Debug build:

```powershell
cmake --preset mingw-qt-x64-debug
cmake --build --preset build-mingw-qt-x64-debug
```

Expected environment:

- Qt 6 desktop kit compatible with the chosen compiler
- For the included preset, the project expects a Qt/MinGW layout similar to:
  - `C:/Qt/6.11.0/mingw_64`
  - `C:/Qt/Tools/mingw1310_64`

Notes:

- On Windows, the Qt-enabled MinGW build is one of the main day-to-day workflows in this repo.
- If Qt is installed elsewhere, set `RO_QT_ROOT` to the matching kit root.
- Qt deployment is handled automatically for Windows Qt builds.

## Building On Linux

Linux has an included preset for the non-Windows Qt/Vulkan path:

- `linux-qt-vulkan`

Configure and build:

```bash
cmake --preset linux-qt-vulkan
cmake --build --preset build-linux-qt-vulkan -j
```

What this means in practice:

- Non-Windows builds force the Qt 6 UI path on.
- Native Direct3D 11 and Direct3D 12 backends are disabled automatically.
- The Linux path is Vulkan-oriented.

Recommended dependencies:

- `cmake`
- `ninja`
- `g++` or `clang++` with C++17 support
- Qt 6 development packages, including QML/Quick
- Vulkan development packages / SDK

Practical note:

- On Ubuntu-like systems, the Qt runtime may also need the `QtQml.WorkerScript` module package installed separately for the QML UI to start correctly.

## Building On macOS

macOS does not currently have a checked-in preset, but the project does have a non-Windows CMake path you can configure manually.

Recommended tools:

- Xcode Command Line Tools or Apple Clang with C++17 support
- `cmake`
- `ninja`
- Qt 6

Example configure and build:

```bash
cmake -S . -B build-macos \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DRO_ENABLE_DEV_DEPLOY=OFF \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/macos

cmake --build build-macos -j
```

Notes:

- On non-Windows platforms, the project enables the Qt 6 UI path automatically.
- Native Direct3D backends are disabled automatically.
- The macOS path is less exercised than the Windows build flow, so expect some platform-specific integration work.

## Running The Client

After building, run:

- Windows: `open-midgard.exe`
- Linux/macOS: `open-midgard`

Make sure your runtime directory contains the client data/configuration files the client expects, and set `OPEN_MIDGARD_DATA_DIR` or `[Data] RuntimeRoot` if they live elsewhere. If the GRFs are stored in a different directory, set `OPEN_MIDGARD_GRF_DIR` or `[Data] GrfRoot`.

## Project Goals

- Recreate the Ragnarok Online client as open source code
- Preserve the feel and behavior of the classic pre-Renewal client
- Modernize the renderer and platform layer where it makes the project easier to build, debug, and maintain
- Keep packet behavior aligned to the chosen 2008-era target profile

## License

See `LICENSE` for the project license.
