set(RO_QTUI_SOURCES
    QtUiRuntime.h
    QtUiSpikeBridge.h
    QtUiSpikeBridge.cpp
    QtUiState.h
    QtUiSpikeState.h
    QtUiStateAdapter.h
    QtUiStateAdapter.cpp
    QtUiTheme.h
    QtUiTheme.cpp
    QtUiSpikeAssets.qrc
)

list(APPEND RO_QTUI_SOURCES
    QtPlatformWindow.h
    QtPlatformWindow.cpp
)

add_library(ro_qtui STATIC ${RO_QTUI_SOURCES})

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
    Qt6::Qml
    Qt6::Quick
    ro_core
    ro_gamemode
    ro_session
    ro_ui
    ro_world
)

if(WIN32 AND RO_HAS_NATIVE_D3D12)
    target_link_libraries(ro_qtui PUBLIC
        Qt6::GuiPrivate
    )
endif()

if(RO_ENABLE_CAPTURE)
    target_link_libraries(ro_qtui PRIVATE ro_capture)
endif()

# Qt's <QVulkanInstance> only forward-declares the class; it relies on
# <vulkan/vulkan.h> already being on the include path in the TU that uses it.
# Pull Vulkan headers in explicitly so the build isn't dependent on another
# package (e.g. vcpkg sdl2) transitively providing them.
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    target_include_directories(ro_qtui PUBLIC ${Vulkan_INCLUDE_DIRS})
endif()
