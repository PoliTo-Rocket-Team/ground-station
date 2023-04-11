cmake_minimum_required(VERSION 3.16)

project(ground-station VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4.3 REQUIRED COMPONENTS Quick Charts)

qt_add_executable(appground-station
    main.cpp
    antenna.h antenna.cpp
    resources.qrc
)

qt_add_qml_module(appground-station
    URI ground-station
    VERSION 1.0
    QML_FILES main.qml UIButton.qml Message.qml VectorPlot.qml VariablePlot.qml
)

set_target_properties(appground-station PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appground-station
    PRIVATE Qt6::Quick Qt6::Charts)

install(TARGETS appground-station
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
