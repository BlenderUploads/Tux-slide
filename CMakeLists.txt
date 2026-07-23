cmake_minimum_required(VERSION 3.10)
project(tux_game CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(tux_game src/main.cpp)
target_link_libraries(tux_game glfw OpenGL::GL GLU)

# ==========================================
# INSTALLATION & PACKAGING (CPack)
# ==========================================

# 1. Define where the game binary should be installed on the user's system
install(TARGETS MyGame DESTINATION bin)

# 2. Basic Package Metadata
set(CPACK_PACKAGE_NAME "TuxSlide")
set(CPACK_PACKAGE_VENDOR "BlenderUploads")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An exciting C++ game starring Tux!")
set(CPACK_PACKAGE_CONTACT "your-email@example.com") # Required for DEB packages

# 3. Linux Debian Specifics (.deb)
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB")
    # Automatically determine package dependencies (like glibc, glfw) on the user's system
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_SECTION "games")
endif()

# 4. Windows Specifics (Generates a clean ZIP package or Nullsoft Installer if NSIS is present)
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
endif()

# Include the CPack module (Must be at the absolute bottom of the file)
include(CPack)

