cmake_minimum_required(VERSION 3.20)

add_library(ro_input STATIC
    Input.h
    Input.cpp
    Gamepad.h
    Gamepad.cpp
    GamepadBindings.h
    GamepadBindings.cpp
    HotkeyBindings.h
    HotkeyBindings.cpp
    VirtualCursor.h
    VirtualCursor.cpp
)

target_include_directories(ro_input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ro_input PUBLIC
    ro_core
)

if(RO_HAS_GAMEPAD)
    # PUBLIC so any consumer of ro_input (UIControllerWnd, Mode.cpp's
    # per-frame Poll wiring) picks up the SDL3 include path. Gamepad.h and
    # GamepadBindings.h expose SDL3 types in their public surface.
    if(TARGET SDL3::SDL3)
        target_link_libraries(ro_input PUBLIC SDL3::SDL3)
    elseif(TARGET SDL3::SDL3-static)
        target_link_libraries(ro_input PUBLIC SDL3::SDL3-static)
    elseif(SDL3_LIBRARIES)
        target_include_directories(ro_input PUBLIC ${SDL3_INCLUDE_DIRS})
        target_link_libraries(ro_input PUBLIC ${SDL3_LIBRARIES})
    endif()
endif()

# Gamepad.cpp routes synthesized events through g_windowMgr (ro_ui) and
# g_modeMgr (ro_gamemode). Link them privately so their PUBLIC include dirs
# (notably Qt6 headers when RO_ENABLE_QT6_UI is on) propagate to this target.
# CMake resolves target names lazily, so forward-referencing libraries that
# are added later via add_subdirectory() is supported.
target_include_directories(ro_input PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ro_input PRIVATE ro_ui ro_gamemode)
