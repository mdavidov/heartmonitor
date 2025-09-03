cmake_minimum_required(VERSION 3.21)

project(HeartMonitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED CMAKE_PREFIX_PATH)
    message(WARNING "\nWARNING: CMAKE_PREFIX_PATH must be defined "
            "as path to your Qt installation, "
            "e.g. 'cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/msvc2022_64 ...' "
            "(or /opt/Qt/6.9.1/macos). Using DEFAULT for Qt 6.9.1.")
    if(APPLE)
        set(CMAKE_PREFIX_PATH "/opt/Qt/6.9.1/macos")
        set(CMAKE_MAKE_PROGRAM /opt/homebrew/bin/ninja)  # this is for Qt Creator
    elseif(UNIX)
        set(CMAKE_MAKE_PROGRAM "/opt/Qt/6.8.1/gcc_arm64")
    elseif(WIN32)
        set(CMAKE_PREFIX_PATH "C:/Qt/6.8.1/msvc2022_64")
    endif()
endif()

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Qml
    Quick
    Bluetooth
    SerialPort
    Multimedia
    Sql
    Svg
    Widgets
)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

# Set up Qt6 QML module
qt6_standard_project_setup()

# Sources
set(SOURCES
    src/hmcontroller.h
    src/hmcontroller.cpp
    src/ecgdatamodel.h
    src/ecgdatamodel.cpp
    src/bluetoothmanager.h
    src/bluetoothmanager.cpp
    src/arrhythmiadetector.h
    src/arrhythmiadetector.cpp
)

# QML files
set(QML_FILES
    qml/main.qml
    qml/ecggraph.qml
)

# Create executable
qt6_add_executable(${PROJECT_NAME}.bin
    src/main.cpp
    resources.qrc
)

# Add QML module
qt6_add_qml_module(${PROJECT_NAME}.bin
    URI HeartMonitor
    VERSION 1.0
    QML_FILES ${QML_FILES}
    SOURCES ${SOURCES}
)

set_target_properties(${PROJECT_NAME}.bin PROPERTIES MACOSX_BUNDLE FALSE)

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME}.bin PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::Bluetooth
    Qt6::SerialPort
    Qt6::Multimedia
    Qt6::Sql
    Qt6::Svg
    Qt6::Widgets
)

# Platform-specific settings
# if(WIN32)
#     set_target_properties(${PROJECT_NAME} PROPERTIES
#         WIN32_EXECUTABLE TRUE
#     )
# endif()

# if(APPLE)
#     set_target_properties(${PROJECT_NAME} PROPERTIES
#         MACOSX_BUNDLE TRUE
#         MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist"
#     )
# endif()

# # Install
# install(TARGETS ${PROJECT_NAME}
#     BUNDLE DESTINATION .
#     RUNTIME DESTINATION bin
# )
