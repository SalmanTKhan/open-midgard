#!/usr/bin/env bash
# Convenience wrapper around `cmake --preset` / `cmake --build --preset`.
#
# Usage:
#   ./build.sh                       # default: mingw-qt-x64-debug
#   ./build.sh <configure-preset>    # configure + build that preset
#   ./build.sh all                   # every preset valid on this host
#   ./build.sh list                  # show known presets
#
# Configure preset -> build preset mapping must match CMakePresets.json.

set -euo pipefail

DEFAULT_PRESET="mingw-qt-x64-debug"

build_preset_for() {
    case "$1" in
        vs2022-win32)        echo "build-release" ;;
        vs2022-x64)          echo "build-release-x64" ;;
        mingw-qt-x64)        echo "build-mingw-qt-x64" ;;
        mingw-qt-x64-debug)  echo "build-mingw-qt-x64-debug" ;;
        mingw-qt-x64-ci)     echo "build-mingw-qt-x64-ci" ;;
        linux-qt-vulkan)     echo "build-linux-qt-vulkan" ;;
        *) return 1 ;;
    esac
}

host_presets() {
    case "$(uname -s)" in
        MINGW*|MSYS*|CYGWIN*|Windows_NT)
            echo "vs2022-x64 mingw-qt-x64 mingw-qt-x64-debug"
            ;;
        Linux)
            echo "linux-qt-vulkan"
            ;;
        Darwin)
            echo ""  # no macOS preset wired up yet
            ;;
        *)
            echo ""
            ;;
    esac
}

run_one() {
    local cfg="$1"
    local bld
    if ! bld="$(build_preset_for "$cfg")"; then
        echo "error: unknown configure preset '$cfg'" >&2
        exit 2
    fi
    echo "==> [$cfg] configure"
    cmake --preset "$cfg"
    echo "==> [$cfg] build ($bld)"
    cmake --build --preset "$bld"
}

target="${1:-$DEFAULT_PRESET}"

case "$target" in
    list)
        echo "Configure presets:"
        echo "  vs2022-win32, vs2022-x64"
        echo "  mingw-qt-x64, mingw-qt-x64-debug, mingw-qt-x64-ci"
        echo "  linux-qt-vulkan"
        echo
        echo "Host-default 'all' on $(uname -s):"
        echo "  $(host_presets)"
        ;;
    all)
        presets="$(host_presets)"
        if [ -z "$presets" ]; then
            echo "error: no presets registered for host $(uname -s)" >&2
            exit 2
        fi
        failed=""
        for p in $presets; do
            if ! run_one "$p"; then
                failed="$failed $p"
            fi
        done
        if [ -n "$failed" ]; then
            echo "FAILED:$failed" >&2
            exit 1
        fi
        echo "All presets built: $presets"
        ;;
    *)
        run_one "$target"
        ;;
esac
