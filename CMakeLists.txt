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
        core/utilits/crc.cpp
        core/utilits/crc.h
        core/utilits/dataconverter.h
        core/utilits/dataconverter.cpp
        core/utilits/ds18b20.cpp
        core/utilits/ds18b20.h
        core/logger.h
        core/logger.cpp
        core/communication/packetbuilder.h
        core/communication/packetbuilder.cpp
        core/communication/ppbprotocol.h
        core/communication/udpclient.cpp
        core/communication/udpclient.h
        core/scenariomanager.h
        core/scenariomanager.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PPB_Tester_Software
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        core/communication/udpclient.h core/communication/udpclient.cpp
        core/communication/packetbuilder.h core/communication/packetbuilder.cpp
        core/communication/ppbprotocol.h
        core/utilits/dataconverter.h core/utilits/dataconverter.cpp
        core/utilits/ds18b20.h core/utilits/ds18b20.cpp
        core/logger.h core/logger.cpp
        core/utilits/crc.h core/utilits/crc.cpp
        core/communication/ppbcommunication.h core/communication/ppbcommunication.cpp





        gui/pult.h gui/pult.cpp gui/pult.ui
        core/communication/commandandoperation.h core/communication/commandandoperation.cpp
        gui/ppbcontroller.h gui/ppbcontroller.cpp
        core/communication/commandinterface.h
        analyzer/testsequencecomparator.h analyzer/testsequencecomparator.cpp
        core/applicationmanager.h core/applicationmanager.cpp
        core/AKIP/akip_manager.h core/AKIP/akip_manager.cpp
        core/communication/communicationengine.h core/communication/communicationengine.cpp
        core/utilits/fileloader.h core/utilits/fileloader.cpp






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
