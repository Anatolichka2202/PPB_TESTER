cmake_minimum_required(VERSION 3.16)

project(PPB_Tester_Software VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        main.cpp
        gui/testerwindow.cpp
        gui/testerwindow.h
        gui/testerwindow.ui
        core/crc.cpp
        core/crc.h
        core/dataconverter.h
        core/dataconverter.cpp
        core/ds18b20.cpp
        core/ds18b20.h
        core/logger.h
        core/logger.cpp
        core/packetbuilder.h
        core/packetbuilder.cpp
        core/ppbprotocol.h
        core/udpclient.cpp
        core/udpclient.h
        core/scenariomanager.h
        core/scenariomanager.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PPB_Tester_Software
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        core/udpclient.h core/udpclient.cpp
        core/packetbuilder.h core/packetbuilder.cpp
        core/ppbprotocol.h
        core/dataconverter.h core/dataconverter.cpp
        core/ds18b20.h core/ds18b20.cpp
        core/logger.h core/logger.cpp
        core/crc.h core/crc.cpp
        core/ppbcommunication.h core/ppbcommunication.cpp
        analyzer/shiftdetector.h analyzer/shiftdetector.cpp
        analyzer/guerrillaanalyzer.h analyzer/guerrillaanalyzer.cpp
        analyzer/analysisfactory.h analyzer/analysisfactory.cpp
        analyzer/testsequenceanalyzer.h analyzer/testsequenceanalyzer.cpp
        analyzer/intelligentanalyzer.h analyzer/intelligentanalyzer.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PPB_Tester_Software APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(PPB_Tester_Software SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PPB_Tester_Software
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(PPB_Tester_Software PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PPB_Tester_Software)
endif()
set_target_properties(PPB_Tester_Software PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PPB_Tester_Software
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PPB_Tester_Software)
endif()
