# Overlay of vcpkg's community x64-mingw-dynamic triplet.
#
# Why this file exists:
#   libvpx's autoconf configure refuses `--enable-shared` on Windows
#   ("--enable-shared only supported on ELF, OS/2, and Darwin for now"), so the
#   stock mingw-dynamic triplet fails to build libvpx. ffmpeg[vpx] pulls libvpx
#   as a hard dependency, which in turn is what the capture subsystem needs for
#   WebM recording. Per-port overrides via the `PORT` variable let us keep the
#   rest of the dependency graph dynamic while forcing libvpx to static.
#
# To use: point vcpkg at this directory via `VCPKG_OVERLAY_TRIPLETS`. The
# top-level CMakeLists.txt does this automatically when it auto-detects vcpkg.

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_ENV_PASSTHROUGH PATH)

set(VCPKG_CMAKE_SYSTEM_NAME MinGW)
set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)

# libvpx does not support Windows shared builds. Pull it in statically so
# ffmpeg[vpx] can link it and still be consumed as a DLL. Nothing else in the
# graph benefits from being forced static, so scope the override narrowly.
if(PORT STREQUAL "libvpx")
    set(VCPKG_LIBRARY_LINKAGE static)
endif()
