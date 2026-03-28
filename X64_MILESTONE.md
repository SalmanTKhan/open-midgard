# x64 Milestone Build

This repository now supports side-by-side `Win32` and `x64` CMake presets.

## Build Presets

- `vs2022-win32`
- `vs2022-x64`
- `build-debug`
- `build-release`
- `build-debug-x64`
- `build-release-x64`

## First x64 Milestone Scope

The first `x64` milestone is aimed at a playable client shell rather than full legacy middleware parity.

Supported target areas:

- Client startup
- GRF loading, including `0x200` compressed indexes via in-process `zlib`
- Login flow
- Server select
- Character select
- Character creation UI flow
- World entry and core in-game mode startup
- UI messaging and chat event plumbing
- Modern rendering path setup

## Legacy Middleware Policy

The `x64` preset disables the legacy middleware integrations below by default:

- Miles audio (`Mss32.dll`)
- Bink video (`binkw32.dll`)
- Granny (`granny2.dll`)
- Intel JPEG Library (`ijl15.dll`)

Reasons:

- The bundled copies are 32-bit only.
- They are either optional for the current client milestone or not on a critical path.
- The code now tolerates those integrations being absent in `x64`.

## Runtime Notes

- `cps.dll` is no longer required for GRF decompression.
- `x64` builds rely on linked `zlib` instead of a DLL-exported `uncompress`.
- The DLL loader reports missing optional middleware, but those missing DLLs are non-fatal for the first `x64` milestone.

## Current Expectation

Use `Win32` when validating old middleware behavior.

Use `x64` when validating:

- pointer-safe internal messaging
- architecture-safe build configuration
- asset loading without `cps.dll`
- milestone gameplay shell behavior without legacy media middleware
