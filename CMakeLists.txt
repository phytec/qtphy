cmake_minimum_required(VERSION 3.16)

option(QML_SINK, "Qt6 QML video sink plugin")

project(qtphy VERSION 0.3.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Qml Quick Gui DBus Multimedia)

list(APPEND qtphy_sources
    resources/resources.qrc
    src/main.cpp
    src/device_info.hpp
    src/device_info.cpp
    src/rauc.hpp
    src/rauc.cpp
)

list(APPEND qtphy_libraries
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::Gui
    Qt6::DBus
    Qt6::Multimedia
)

if(QML_SINK)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(gstreamer REQUIRED IMPORTED_TARGET gstreamer-1.0)
    list(APPEND qtphy_sources
        src/multimedia_qmlsink.hpp
        src/multimedia_qmlsink.cpp
    )
    list(APPEND qtphy_libraries
        PkgConfig::gstreamer
    )
    add_definitions(-DQML_SINK)
endif()

qt_standard_project_setup()

qt_add_executable(qtphy
    ${qtphy_sources}
)

target_link_libraries(qtphy PRIVATE ${qtphy_libraries})
