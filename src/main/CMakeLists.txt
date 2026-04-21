cmake_minimum_required(VERSION 3.20)

set(RO_MAIN_SOURCE WinMain.cpp)
if(NOT WIN32)
    set(RO_MAIN_SOURCE AppMain_stub.cpp)
endif()

add_library(ro_main STATIC
    WinMain.h
    ${RO_MAIN_SOURCE}
)

target_include_directories(ro_main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ro_main PUBLIC
    ro_gamemode
    ro_session
    ro_network
    ro_core
    ro_cipher
    ro_input
)

if(RO_HAS_GAMEPAD)
    if(TARGET SDL3::SDL3)
        target_link_libraries(ro_main PRIVATE SDL3::SDL3)
    elseif(TARGET SDL3::SDL3-static)
        target_link_libraries(ro_main PRIVATE SDL3::SDL3-static)
    endif()
endif()
