find_package(Qt6 REQUIRED COMPONENTS Core Gui GuiPrivate Qml Quick)

add_library(ro_qtui STATIC
    QtUiRuntime.h
    QtUiSpikeBridge.h
    QtUiSpikeBridge.cpp
    QtUiState.h
    QtUiSpikeState.h
    QtUiStateAdapter.h
    QtUiStateAdapter.cpp
    QtUiSpikeAssets.qrc
)

set_target_properties(ro_qtui PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC OFF
)

target_include_directories(ro_qtui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ro_qtui PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::Qml
    Qt6::Quick
    ro_core
    ro_gamemode
    ro_session
    ro_ui
    ro_world
)
