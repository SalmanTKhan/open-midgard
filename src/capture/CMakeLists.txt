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
# Resolve libraries to absolute paths. `find_package(FFMPEG)` on vcpkg-mingw
# hands back bare names like "avcodec", which ld can't find unless we also
# propagate FFMPEG_LIBRARY_DIRS — and even then the resolution is flaky across
# generators. `find_library` with hints from the vcpkg layout is more
# reliable.
find_package(FFMPEG COMPONENTS avcodec avformat avutil swscale QUIET)

set(_ro_ffmpeg_hint_dirs)
if(FFMPEG_LIBRARY_DIRS)
    list(APPEND _ro_ffmpeg_hint_dirs ${FFMPEG_LIBRARY_DIRS})
endif()
if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
    list(APPEND _ro_ffmpeg_hint_dirs
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib")
endif()

find_path(RO_FFMPEG_INCLUDE_DIR libavcodec/avcodec.h
    HINTS ${FFMPEG_INCLUDE_DIRS})
find_library(RO_FFMPEG_AVCODEC_LIB  NAMES avcodec  HINTS ${_ro_ffmpeg_hint_dirs})
find_library(RO_FFMPEG_AVFORMAT_LIB NAMES avformat HINTS ${_ro_ffmpeg_hint_dirs})
find_library(RO_FFMPEG_AVUTIL_LIB   NAMES avutil   HINTS ${_ro_ffmpeg_hint_dirs})
find_library(RO_FFMPEG_SWSCALE_LIB  NAMES swscale  HINTS ${_ro_ffmpeg_hint_dirs})

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
else()
    message(FATAL_ERROR
        "ro_capture: FFmpeg libraries not found. "
        "include=${RO_FFMPEG_INCLUDE_DIR} "
        "avcodec=${RO_FFMPEG_AVCODEC_LIB} "
        "avformat=${RO_FFMPEG_AVFORMAT_LIB} "
        "avutil=${RO_FFMPEG_AVUTIL_LIB} "
        "swscale=${RO_FFMPEG_SWSCALE_LIB}. "
        "Install via vcpkg (ffmpeg[vpx]) or disable RO_ENABLE_CAPTURE.")
endif()
unset(_ro_ffmpeg_hint_dirs)
