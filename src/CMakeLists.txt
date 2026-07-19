set(RADIALD_SOURCES
    daemon/main.cpp
    daemon/OverlayController.cpp
    daemon/SocketServer.cpp
    daemon/AppCatalogModel.cpp
    daemon/ConfigStore.cpp
)

qt_add_executable(radiald
    ${RADIALD_SOURCES}
)

qt_add_qml_module(radiald
    URI Radial
    VERSION 1.0
    QML_FILES
        ui/qml/Main.qml
)

target_include_directories(radiald PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon
)

target_link_libraries(radiald PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    LayerShellQt::Interface
)
