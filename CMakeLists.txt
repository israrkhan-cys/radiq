cmake_minimum_required(VERSION 3.21)
project(radiq VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network)
find_package(LayerShellQt REQUIRED)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_standard_project_setup()

add_subdirectory(src)
add_subdirectory(tools/radiqctl)
