cmake_minimum_required(VERSION 3.10)
project(tux-game LANGUAGES CXX)

# Force C++17 or higher
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GLFW
find_package(glfw3 REQUIRED)

# Collect source files under src/
file(GLOB_RECURSE GAME_SOURCES
     "${CMAKE_SOURCE_DIR}/src/*.cpp"
     "${CMAKE_SOURCE_DIR}/src/*.cc"
     "${CMAKE_SOURCE_DIR}/src/*.cxx"
     "${CMAKE_SOURCE_DIR}/src/*.c")

if(NOT GAME_SOURCES)
    message(FATAL_ERROR "No source files found in src/ — ensure your source files are present and the path is correct.")
endif()

add_executable(tux_game ${GAME_SOURCES})

# Link GLFW (the imported target or library name may vary between platforms/distributions)
# Prefer imported target if available:
if(TARGET glfw)
  target_link_libraries(tux_game PRIVATE glfw)
else()
  target_link_libraries(tux_game PRIVATE glfw3)
endif()

# Platform-specific configurations
if(UNIX AND NOT APPLE)
    # Linux requires OpenGL and X11 libraries
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    # Link OpenGL and the various X11 libs needed
    target_link_libraries(tux_game PRIVATE ${OPENGL_LIBRARIES} ${X11_LIBRARIES} Xrandr Xinerama Xcursor Xi)
elseif(APPLE)
    # macOS requires system frameworks
    target_link_libraries(tux_game PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# ==========================================
# FIXED INSTALLATION & PACKAGING (CPack)
# ==========================================

# 1. Fixed target name from MyGame to tux_game
install(TARGETS tux_game RUNTIME DESTINATION bin)

# 2. Package Metadata
set(CPACK_PACKAGE_NAME "TuxSlide")
set(CPACK_PACKAGE_VENDOR "BlenderUploads")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An exciting C++ game starring Tux!")
set(CPACK_PACKAGE_CONTACT "blenderuploads@example.com") # Put your real email here

# 3. Linux Debian Specifics (.deb)
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_SECTION "games")
endif()

# 4. Windows Specifics
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
endif()

include(CPack)

# --- packaging start ---

# make sure you already define a target for your executable, e.g.:
# add_executable(tux-slide src/main.cpp ...)

# On macOS create an .app bundle:
if(APPLE)
  set_target_properties(tux-slide PROPERTIES MACOSX_BUNDLE TRUE)
endif()

# Install the built product and resources so CPack can package them.
# Adjust resource files and paths to match your project.
install(TARGETS tux-slide
  RUNTIME DESTINATION bin                # linux/windows
  BUNDLE DESTINATION .                   # macOS .app goes to top-level
)

# Install runtime assets (models/textures/sounds) into Resources for macOS or share for other OSes
install(DIRECTORY assets/ DESTINATION share/tux-slide/assets)

# Basic package metadata
set(CPACK_PACKAGE_NAME "Tux-slide")
set(CPACK_PACKAGE_VENDOR "YourNameOrOrg")
set(CPACK_PACKAGE_CONTACT "you@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A tiny low-poly 3D game starring Tux")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

# Generators you want to produce. Keep platform-specific generators but CPack will only produce what works on the current machine.
# On Windows: NSIS -> produces an .exe installer (NSIS must be installed)
# On macOS: DragNDrop -> .dmg (uses hdiutil)
# On Linux: DEB -> .deb (requires dpkg-deb available)
set(CPACK_GENERATOR "NSIS;DragNDrop;DEB")

# Windows/NSIS customization
set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
set(CPACK_NSIS_MODINSTALL "on")

# Debian customization
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
# If your runtime needs specific system packages, list them here:
# set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgl1-mesa-glx (>= 17), libglfw3")

# Include system libs so CPack pulls standard runtime libs if needed
include(InstallRequiredSystemLibraries)

# Finally include CPack
include(CPack)
# --- packaging end ---
