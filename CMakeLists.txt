cmake_minimum_required(VERSION 3.21)

project(DesktopApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Network 
    Multimedia 
    TextToSpeech
    Svg
    Test
)

# Enable Qt MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Compiler flags for strict development
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # For MinGW/GNU remove -Werror to tolerate external header warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/core/Application.cpp
    src/core/CrashHandler.cpp
    src/ui/MainWindow.cpp
    src/ui/ConversationListWidget.cpp
    src/ui/MessageThreadWidget.cpp
    src/ui/MessageComposer.cpp
    src/ui/SettingsDialog.cpp
    src/ui/SimpleMarkdown.cpp
    src/ui/LoginWindow.cpp
    src/ui/EnhancedMessageWidget.cpp
    src/theme/ThemeManager.cpp
    src/theme/IconRegistry.cpp
    src/data/JsonStore.cpp
    src/data/Models.cpp
    src/providers/ProviderSDK.cpp
    src/providers/ProviderManager.cpp
    src/providers/EchoProvider.cpp
    src/services/FileVault.cpp
    src/services/AudioRecorder.cpp
    src/services/SettingsStore.cpp
    src/services/SearchEngine.cpp
    src/services/AuthenticationService.cpp
)

# UI files
set(UI_SOURCES)

# Resource files
set(RESOURCES
    resources.qrc
)

# Create static library with core sources (without main)
add_library(DesktopAppLib STATIC ${SOURCES} ${UI_SOURCES} ${RESOURCES})

target_link_libraries(DesktopAppLib
    PUBLIC
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network 
    Qt6::Multimedia 
    Qt6::TextToSpeech
    Qt6::Svg
)

# Main executable (only main.cpp plus library)
add_executable(DesktopApp src/main.cpp)
target_link_libraries(DesktopApp PRIVATE DesktopAppLib Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::TextToSpeech Qt6::Svg)

# Link Qt6 libraries
# (Platform-specific libs applied to both targets)
foreach(tgt IN ITEMS DesktopApp DesktopAppLib)
    if(WIN32)
    target_link_libraries(${tgt} PRIVATE crypt32 Dbghelp)
    elseif(APPLE)
        target_link_libraries(${tgt} PRIVATE "-framework Security")
    elseif(UNIX)
        target_link_libraries(${tgt} PRIVATE secret-1)
    endif()
endforeach()

# Testing fully removed (can be restored later if needed)

# Install target
install(TARGETS DesktopApp DESTINATION bin)

# CPack configuration for installers
include(CPack)
set(CPACK_PACKAGE_NAME "DesktopApp")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern AI Chat Desktop Application")
set(CPACK_PACKAGE_VENDOR "DesktopApp Project")

if(WIN32)
    set(CPACK_GENERATOR "WIX")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
endif()
