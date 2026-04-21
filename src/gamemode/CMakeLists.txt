add_library(ro_gamemode STATIC
    Mode.h
    Mode.cpp
    CursorRenderer.h
    CursorRenderer.cpp
    LoginMode.h
    LoginMode.cpp
    GameMode.h
    GameMode.cpp
    View.h
    View.cpp
    GameModePacket.h
    GameModePacket.cpp
)

target_include_directories(ro_gamemode PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ro_gamemode PUBLIC
    ro_core
    ro_cipher
    ro_network
    ro_world
    ro_audio
    ro_ui
)

# Mode.cpp pumps the gamepad every frame, so the SDL3 include path needs
# to propagate to ro_gamemode. ro_input isn't a link dep of ro_gamemode
# today, but we can reach the SDL3 target directly.
if(RO_HAS_GAMEPAD)
    if(TARGET SDL3::SDL3)
        target_link_libraries(ro_gamemode PRIVATE SDL3::SDL3)
    elseif(TARGET SDL3::SDL3-static)
        target_link_libraries(ro_gamemode PRIVATE SDL3::SDL3-static)
    endif()
endif()

if(RO_ENABLE_QT6_UI)
    target_link_libraries(ro_gamemode PUBLIC
        Qt6::Core
        Qt6::Gui
    )
endif()
