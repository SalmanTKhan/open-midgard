# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

OpenMidgard is an open-source recreation of the pre-Renewal (2008-era) Ragnarok Online client. Bias decisions toward classic 2008 client behavior, not Renewal.

## Build & Run

There is no test target wired into any preset — there is nothing to run for "tests". Verify changes by building.

**Default build for verifying code changes** (per `.cursor/rules/build-mode-rules.mdc`): MSVC x64 Release with dev-deploy. Use this unless the user asks for something else.

```
cmake --preset vs2022-x64 -DRO_ENABLE_DEV_DEPLOY=ON
cmake --build --preset build-release-x64
```

Other real build flows (all defined in `CMakePresets.json`):

- **MinGW + Qt 6** — day-to-day Qt UI work on Windows: presets `mingw-qt-x64` and `mingw-qt-x64-debug`, build with `build-mingw-qt-x64[-debug]`. Expects `C:/Qt/6.11.0/mingw_64` and `C:/Qt/Tools/mingw1310_64`; override the kit with `RO_QT_ROOT`. **Also requires the LunarG Vulkan SDK** so `<vulkan/vulkan.h>` is on the include path — Qt only defines `QVulkanInstance` when that header is found, otherwise `src/qtui/QtUiSpikeBridge.cpp` fails with "incomplete type".
- **VS2022 Win32** — `vs2022-win32` / `build-release` (legacy 32-bit path).
- **Linux** — `linux-qt-vulkan` (Ninja). Non-Windows automatically forces `RO_ENABLE_QT6_UI=ON` and disables D3D backends.

Output binary: `open-midgard(.exe)`. Runtime data discovery looks for `data.grf` / `data/clientinfo.xml` near the executable; override with `OPEN_MIDGARD_DATA_DIR`. Dev deploy copies the executable and runs `windeployqt` into `D:/Spel/OldRO`; disable with `-DRO_ENABLE_DEV_DEPLOY=OFF`.

Notable CMake options (top-level `CMakeLists.txt`):

- `RO_ENABLE_QT6_UI` — gate the Qt6/QML UI runtime. `RO_ENABLE_QT6_UI_SPIKE` is a deprecated alias; the build keeps both flags in sync automatically.
- `RO_QT_UI_DEFAULT_ENABLED` — runtime default for whether the Qt UI is on (overridable at runtime via `OPEN_MIDGARD_QT_UI`).
- `RO_ENABLE_NATIVE_D3D11` / `RO_ENABLE_NATIVE_D3D12` — MSVC-Windows-only native backends; force-disabled on non-Windows.
- `DX7_SDK_DIR` env var — optional path to a real DirectX 7 SDK; falls back to the Windows SDK headers.

## Architecture

Single executable `open_midgard` assembled from per-subsystem static libraries. Each `src/<subsystem>/` is its own CMake target listed in the top-level `CMakeLists.txt`. Entry points: `src/main/WinMain.cpp` (Windows) and `src/main/AppMain_stub.cpp` (non-Windows).

Library layering (top → bottom):

- `ro_main` — entry, app loop, window
- `ro_gamemode`, `ro_session` — mode FSM (`LoginMode`, `GameMode`), session state
- `ro_world`, `ro_render3d`, `ro_render`, `ro_ui` — world simulation, 3D backend abstraction, 2D primitives, legacy UI
- `ro_qtui` — Qt6/QML UI runtime (only built when `RO_ENABLE_QT6_UI`); coexists with the legacy UI rather than replacing it
- `ro_skill`, `ro_item`, `ro_pathfinder`, `ro_network`, `ro_res`, `ro_audio`, `ro_input`, `ro_cipher`, `ro_core`, `ro_security`, `ro_lua`

Two cross-cutting axes worth knowing before editing:

- **UI is dual-stack.** Legacy `src/ui` and modern `src/qtui` ship side-by-side. When Qt is disabled, `src/qtui/QtUiRuntimeNoop.cpp` provides stubs so `ro_main` still links. Code that touches UI usually needs to consider both paths.
- **Renderer is multi-backend.** `src/render3d/RenderBackend.{h,cpp}` is the abstraction. Native D3D11/D3D12 are MSVC-Windows-only. MinGW and non-Windows are Vulkan-oriented. The legacy DX7 path (`Device.cpp`, `Dx7Compat.h`) is still present for the original client behavior.

Compile-time platform/feature flags (`RO_PLATFORM_WINDOWS/LINUX/MACOS`, `RO_HAS_NATIVE_D3D11/12`, `RO_ENABLE_QT6_UI`, `RO_VULKAN_USE_WIN32_SURFACE`, `RO_X64_FIRST_MILESTONE_BUILD`, etc.) are defined in the top-level `CMakeLists.txt` and consumed throughout `src/`.

The x64 build force-disables `RO_ENABLE_MILES_AUDIO`, `RO_ENABLE_BINK_VIDEO`, `RO_ENABLE_GRANNY`, and `RO_ENABLE_IJL` because those legacy 32-bit DLLs aren't usable in x64. Don't re-enable them on x64.

## Project-specific rules

These mirror `.cursor/rules/*.mdc` and `.github/instructions/*.instructions.md` — follow them.

- **Packet alignment: `packet_ver 23` (`2008-09-10aSakexe`).** Read `PACKET_VERSION_ALIGNMENT.md` before any packet-related change. The target applies to **client-send** map-server packets. The receive table is intentionally not forced to packet_ver 23 — keep receive parsing aligned with what the server actually emits (current evidence matches `PACKETVER <= 20081217`). For server-side or protocol verification, prefer `Ref/eAthena_src_2011` and `Ref/RunningServer` over the decompiled client. Don't assume the decompiled `Ref/` client uses the same packet_ver.
- **Reference lookups: check `Ref/` before guessing.** `Ref/` is the decompiled original client. `Ref/eAthena_src_2011` and `Ref/RunningServer` are the authoritative server/protocol references. `Ref/GRF-Content` documents unpacked GRF asset names/paths. If the `Ref/` tree isn't present in the working copy, say so and ask rather than fabricating.
- **Deliver complete implementations.** No placeholder logic, mock data paths, or "TODO" stubs unless the user explicitly asks for scaffolding. If blocked by missing information, state the blocker and ask.
- **Media files: inspect bytes, don't render.** Don't open sprites, audio, or GRF binaries with image/audio viewers. Examine headers and binary metadata. If a task genuinely needs visual or audio inspection, stop and ask the user how to proceed.

## Deeper docs

- `README.md` — presets, prerequisites, runtime layout
- `PACKET_VERSION_ALIGNMENT.md` — authoritative packet decision record
- `docs/qt_build_and_compliance.md` — Qt kit detection, `windeployqt`, Qt LGPL bundle (`third_party/qt` → `licenses/qt`, with `qt.conf` relocating Qt trees under `thirdparty/qt`)
- `docs/qt6_qml_migration_spike.md` — Qt6/QML UI background
- `TODO.md`, `X64_MILESTONE.md`, `ANTI_ALIASING.md` — focused implementation notes
