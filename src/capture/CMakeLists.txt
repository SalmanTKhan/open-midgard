cmake_minimum_required(VERSION 3.20)

add_library(ro_capture STATIC
    CaptureConfig.h
    CaptureConfig.cpp
    FrameCapture.h
    FrameCapture.cpp
    WebpEncoder.h
    WebpEncoder.cpp
    WebmRecorder.h
    WebmRecorder.cpp
)

target_include_directories(ro_capture PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ro_capture PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ro_capture PUBLIC ro_core)

# libwebp (WebP still encoder). vcpkg port: libwebp; exports WebP::webp.
find_package(WebP CONFIG QUIET)
if(TARGET WebP::webp)
    target_link_libraries(ro_capture PRIVATE WebP::webp)
else()
    find_path(WEBP_INCLUDE_DIR webp/encode.h)
    find_library(WEBP_LIBRARY NAMES webp)
    if(WEBP_INCLUDE_DIR AND WEBP_LIBRARY)
        target_include_directories(ro_capture PRIVATE ${WEBP_INCLUDE_DIR})
        target_link_libraries(ro_capture PRIVATE ${WEBP_LIBRARY})
    else()
        message(FATAL_ERROR "ro_capture: libwebp not found. Install via vcpkg (libwebp) or disable RO_ENABLE_CAPTURE.")
    endif()
endif()

# FFmpeg (avcodec + avformat + swscale). vcpkg port: ffmpeg[avcodec,avformat,swscale,vpx].
# The VP8 encoder ships inside avcodec when libvpx was linked in during the
# ffmpeg build; we call avcodec_find_encoder_by_name("libvpx") at runtime.
#
# Discovery order:
#   1. pkg-config (Homebrew / Linux distro packages): clean and propagates
#      include + link dirs automatically via the imported target.
#   2. find_package(FFMPEG) — Qt bundles a FindFFMPEG.cmake; best-effort hint
#      source. Often fails silently on macOS, which is fine.
#   3. find_library with extended hints (vcpkg layout, FFMPEG_DIR, Homebrew
#      prefixes) as a last resort.

set(_ro_ffmpeg_resolved FALSE)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(RO_FFMPEG_PC QUIET IMPORTED_TARGET
        libavformat libavcodec libswscale libavutil)
    if(RO_FFMPEG_PC_FOUND)
        target_link_libraries(ro_capture PRIVATE PkgConfig::RO_FFMPEG_PC)
        set(_ro_ffmpeg_resolved TRUE)
        message(STATUS "ro_capture: FFmpeg resolved via pkg-config (${RO_FFMPEG_PC_VERSION})")
    endif()
endif()

if(NOT _ro_ffmpeg_resolved)
    # Qt's bundled FindFFMPEG.cmake expects uppercase component names; lowercase
    # yields a "Found ... missing components: AVCODEC AVFORMAT ..." warning.
    find_package(FFMPEG COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE QUIET)

    set(_ro_ffmpeg_hint_libs)
    set(_ro_ffmpeg_hint_incs)
    if(FFMPEG_LIBRARY_DIRS)
        list(APPEND _ro_ffmpeg_hint_libs ${FFMPEG_LIBRARY_DIRS})
    endif()
    if(FFMPEG_INCLUDE_DIRS)
        list(APPEND _ro_ffmpeg_hint_incs ${FFMPEG_INCLUDE_DIRS})
    endif()
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        list(APPEND _ro_ffmpeg_hint_libs
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib")
        list(APPEND _ro_ffmpeg_hint_incs
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
    endif()
    foreach(_ro_ffmpeg_prefix
            "${FFMPEG_DIR}"
            "$ENV{FFMPEG_DIR}"
            "/opt/homebrew/opt/ffmpeg"
            "/usr/local/opt/ffmpeg")
        if(_ro_ffmpeg_prefix AND IS_DIRECTORY "${_ro_ffmpeg_prefix}")
            list(APPEND _ro_ffmpeg_hint_libs "${_ro_ffmpeg_prefix}/lib")
            list(APPEND _ro_ffmpeg_hint_incs "${_ro_ffmpeg_prefix}/include")
        endif()
    endforeach()

    find_path(RO_FFMPEG_INCLUDE_DIR libavcodec/avcodec.h
        HINTS ${_ro_ffmpeg_hint_incs})
    find_library(RO_FFMPEG_AVCODEC_LIB  NAMES avcodec  HINTS ${_ro_ffmpeg_hint_libs})
    find_library(RO_FFMPEG_AVFORMAT_LIB NAMES avformat HINTS ${_ro_ffmpeg_hint_libs})
    find_library(RO_FFMPEG_AVUTIL_LIB   NAMES avutil   HINTS ${_ro_ffmpeg_hint_libs})
    find_library(RO_FFMPEG_SWSCALE_LIB  NAMES swscale  HINTS ${_ro_ffmpeg_hint_libs})

    if(RO_FFMPEG_INCLUDE_DIR
       AND RO_FFMPEG_AVCODEC_LIB
       AND RO_FFMPEG_AVFORMAT_LIB
       AND RO_FFMPEG_AVUTIL_LIB
       AND RO_FFMPEG_SWSCALE_LIB)
        target_include_directories(ro_capture PRIVATE ${RO_FFMPEG_INCLUDE_DIR})
        target_link_libraries(ro_capture PRIVATE
            ${RO_FFMPEG_AVFORMAT_LIB}
            ${RO_FFMPEG_AVCODEC_LIB}
            ${RO_FFMPEG_SWSCALE_LIB}
            ${RO_FFMPEG_AVUTIL_LIB})
        set(_ro_ffmpeg_resolved TRUE)
        message(STATUS "ro_capture: FFmpeg resolved via find_library (${RO_FFMPEG_AVCODEC_LIB})")
    endif()
    unset(_ro_ffmpeg_hint_libs)
    unset(_ro_ffmpeg_hint_incs)
endif()

if(NOT _ro_ffmpeg_resolved)
    message(FATAL_ERROR
        "ro_capture: FFmpeg libraries not found. "
        "Tried pkg-config (libavformat/libavcodec/libswscale/libavutil), "
        "find_package(FFMPEG), and find_library with hints from "
        "FFMPEG_DIR and Homebrew prefixes. "
        "Install via vcpkg (ffmpeg[vpx]), `brew install ffmpeg`, or disable RO_ENABLE_CAPTURE.")
endif()
unset(_ro_ffmpeg_resolved)
