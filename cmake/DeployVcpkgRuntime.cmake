# DeployVcpkgRuntime.cmake
#
# Invoked as a post-build step: copies vcpkg-installed runtime DLLs from the
# active triplet into a deploy directory. Applies to ffmpeg (avcodec/avformat/
# avutil/swscale/swresample), libwebp, SDL3, and any transitive MinGW runtime
# DLLs that landed in vcpkg's bin folder.
#
# Inputs (passed via -D on the cmake command line):
#   RO_VCPKG_BIN_DIR — absolute path to vcpkg_installed/<triplet>/(debug/)bin
#   RO_DEPLOY_DIR    — absolute path to the directory that should receive DLLs
#
# Missing sources are tolerated (capture/features may be off); only presence
# of the bin dir itself matters.

if(NOT DEFINED RO_VCPKG_BIN_DIR OR NOT DEFINED RO_DEPLOY_DIR)
    message(FATAL_ERROR "DeployVcpkgRuntime.cmake: RO_VCPKG_BIN_DIR and RO_DEPLOY_DIR must be set.")
endif()

if(NOT IS_DIRECTORY "${RO_VCPKG_BIN_DIR}")
    # No vcpkg runtime to copy (e.g. all deps static). Nothing to do.
    return()
endif()

file(MAKE_DIRECTORY "${RO_DEPLOY_DIR}")

set(_patterns
    "avcodec-*.dll"
    "avformat-*.dll"
    "avutil-*.dll"
    "swscale-*.dll"
    "swresample-*.dll"
    "libwebp*.dll"
    "libsharpyuv*.dll"
    "SDL3*.dll"
)

foreach(_pattern IN LISTS _patterns)
    file(GLOB _matches LIST_DIRECTORIES FALSE "${RO_VCPKG_BIN_DIR}/${_pattern}")
    foreach(_src IN LISTS _matches)
        get_filename_component(_name "${_src}" NAME)
        set(_dst "${RO_DEPLOY_DIR}/${_name}")
        if(NOT EXISTS "${_dst}" OR "${_src}" IS_NEWER_THAN "${_dst}")
            file(COPY "${_src}" DESTINATION "${RO_DEPLOY_DIR}")
            message(STATUS "Deployed vcpkg runtime: ${_name} -> ${RO_DEPLOY_DIR}")
        endif()
    endforeach()
endforeach()
