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
        analyzer/packetanalyzer.cpp
        analyzer/packetanalyzer.h
        analyzer/packetanalyzer_interface.h
        analyzer/packetanalyzer_adapter.h
        analyzer/analyzer_factory.h
        core/AKIP/usbinterface.cpp
        core/AKIP/usbinterface.h
        core/AKIP/akip_scpi.h
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

        core/applicationmanager.h core/applicationmanager.cpp
        core/AKIP/akip_manager.h core/AKIP/akip_manager.cpp
        core/communication/communicationengine.h core/communication/communicationengine.cpp
        core/utilits/fileloader.h core/utilits/fileloader.cpp
        core/logwrapper.h core/logwrapper.cpp
        core/logentry.h
        core/logging/logconfig.h core/logging/logconfig.cpp

        core/logentry.cpp
        resources/logging.qrc
        core/logging/loguimanager.h core/logging/loguimanager.cpp
        core/logging/logdistributor.h core/logging/logdistributor.cpp


        analyzer/packetanalyzer.cpp
        analyzer/packetanalyzer.h
        analyzer/packetanalyzer_interface.h
        analyzer/packetanalyzer_adapter.h
        analyzer/analyzer_factory.h
        core/logging/logging_unified.h
        gui/akip_pult.h gui/akip_pult.cpp gui/akip_pult.ui

        core/AKIP/usbinterface.cpp
        core/AKIP/usbinterface.h
        core/AKIP/akip_scpi.h

    )
target_compile_definitions(PPB_Tester_Software PRIVATE NOMINMAX) #запрещаем wndows определять макросы мин/макс
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

# --- КОНФИГУРАЦИЯ ДЛЯ CH375DLL (АКИП-3417) ---
# Правильный путь: библиотеки лежат в core/AKIP/ch375_sdk
set(CH375_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/core/AKIP/ch375_sdk")

# 1. Добавляем пути к заголовочным файлам:
#    - ${CMAKE_CURRENT_SOURCE_DIR}/core/AKIP - чтобы работал #include "ch375_sdk/CH375DLL_EN.H"
#    - ${CH375_SDK_DIR} - для прямого #include "CH375DLL_EN.H"
target_include_directories(PPB_Tester_Software PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core/AKIP
    ${CH375_SDK_DIR}
)

# 2. Библиотека
set(CH375_LIB_FILE "${CH375_SDK_DIR}/CH375DLL64.lib")
set(CH375_DLL_FILE "${CH375_SDK_DIR}/CH375DLL64.dll")

# 3. Линковка (проверяем существование файла)
if(EXISTS ${CH375_LIB_FILE})
    target_link_libraries(PPB_Tester_Software PRIVATE ${CH375_LIB_FILE})
    message(STATUS "CH375DLL64.lib found, linking...")
else()
    message(WARNING "CH375DLL64.lib NOT found at ${CH375_LIB_FILE}")
endif()

# 4. Копирование .dll в выходную папку
if(EXISTS ${CH375_DLL_FILE})
    add_custom_command(TARGET PPB_Tester_Software POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CH375_DLL_FILE}"
            $<TARGET_FILE_DIR:PPB_Tester_Software>
        COMMENT "Copying CH375DLL64.dll to output directory"
    )
endif()

# 5. Системные библиотеки для MinGW
if(MINGW)
    target_link_libraries(PPB_Tester_Software PRIVATE
        -luser32 -lgdi32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lws2_32
    )

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
endif()
include(GNUInstallDirs)
install(TARGETS PPB_Tester_Software
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PPB_Tester_Software)
endif()
