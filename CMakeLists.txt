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
