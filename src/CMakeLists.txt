set(RADIQD_SOURCES
    daemon/main.cpp
    daemon/OverlayController.cpp
    daemon/SocketServer.cpp
    daemon/AppCatalogModel.cpp
    daemon/ConfigStore.cpp
)

qt_add_executable(radiqd
    ${RADIQD_SOURCES}
)

qt_add_qml_module(radiqd
    URI Radiq
    VERSION 1.0
    QML_FILES
        ui/qml/Main.qml
)

target_include_directories(radiqd PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon
)

target_link_libraries(radiqd PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    LayerShellQt::Interface
)
