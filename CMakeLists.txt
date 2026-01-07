# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.
#
# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#          2014, Jan Schlueter <jan.schlueter@ofai.at>

cmake_minimum_required(VERSION 3.24)

project(musly
    VERSION 0.2
    LANGUAGES C CXX
)

# disable in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

# set default build type
set(default_build_type "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE
    )
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY 
        STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
endif()

# determine if we get imported by another cmake project
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IS_SUBPROJECT OFF)
else()
    set(IS_SUBPROJECT ON)
endif()

option(BUILD_STATIC "Make a static build of libmusly" OFF)
if (BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF)
    message(STATUS "Building static libmusly")
else ()
    set(BUILD_SHARED_LIBS ON)
    message(STATUS "Building shared libmusly")
endif ()

option(BUILD_TEST "Build selftest executable" OFF)

option(MUSLY_USE_OWN_LIBKISSFFT OFF)
option(MUSLY_USE_OWN_LIBRESAMPLE OFF)

#
# Project Setup
#

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CheckIPOSupported)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(NOT IS_SUBMODULE)
    # setup c++
    set(CMAKE_C_EXTENSIONS OFF)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    set(CMAKE_INCLUDE_CURRENT_DIR OFF)

    # add support for folders IDE project generators
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(musly_CMAKEPKG_INSTALL_DIR 
    "${CMAKE_INSTALL_DATADIR}/musly/cmake"
    CACHE PATH "The directory relative to CMAKE_INSTALL_PREFIX where the Musly CMake package files are installed.")


if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  message(STATUS "Interprocedural optimizations turned off because WIN32 GCC is funky")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATIONS OFF)
  set(musly_IPO_SUPPORTED NO)
elseif(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATIONS OR CMAKE_INTERPROCEDURAL_OPTIMIZATIONS)
  check_ipo_supported(RESULT musly_IPO_SUPPORTED)
  message(STATUS "Interprocedural optimizations supported: ${musly_IPO_SUPPORTED}")
endif()

#
# Dependencies
#

include(FetchContent)

# make sure Eigen3 library is present. Since it is header-only no need to insist on
# using installed packages, but we'll take it when it is there.
FetchContent_Declare(Eigen3
    URL      https://gitlab.com/libeigen/eigen/-/archive/3.4.1/eigen-3.4.1.tar.gz
    URL_HASH SHA256=b93c667d1b69265cdb4d9f30ec21f8facbbe8b307cf34c0b9942834c6d4fdbe2
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS NO_MODULE
)
FetchContent_MakeAvailable(Eigen3)

if(NOT MUSLY_USE_OWN_LIBRESAMPLE)
    find_package(LibResample REQUIRED)
else()
    message(STATUS "Using vendored version of libresample.")
    add_subdirectory(libresample)
endif()

if(NOT MUSLY_USE_OWN_LIBKISSFFT)
    find_package(kissfft REQUIRED)
else()
    message(STATUS "Using vendored version of kissfft.")
    add_subdirectory(libkissfft)
endif()

find_package(LibAV 4.3 COMPONENTS avcodec avformat avutil)

add_subdirectory(libmusly)

if (NOT IS_SUBPROJECT)
    if (NOT WIN32 AND NOT MINGW)
        add_subdirectory(musly)
    endif()

    if (BUILD_TEST)
        enable_testing()
        add_subdirectory(test)
    endif ()

    # Documentation
    set(musly_DOC_FILES AUTHORS COPYING README.md)
    set(musly_DOC_PATH "share/doc/musly")
    install(FILES ${musly_DOC_FILES}
        DESTINATION ${musly_DOC_PATH})

endif()

#
# Install CMake Config Package
#

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MuslyConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MuslyConfig.cmake
    INSTALL_DESTINATION ${musly_CMAKEPKG_INSTALL_DIR}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MuslyConfigVersion.cmake"
    VERSION "${musly_VERSION_MAJOR}.${musly_VERSION_MINOR}"
    COMPATIBILITY SameMajorVersion
)

export(EXPORT MuslyTargets
    FILE MuslyTargets.cmake
    NAMESPACE Musly::
)

install(EXPORT MuslyTargets
    FILE MuslyTargets.cmake
    NAMESPACE Musly::
    DESTINATION ${musly_CMAKEPKG_INSTALL_DIR}
)

export(PACKAGE Musly)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/MuslyConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/MuslyConfigVersion.cmake
    DESTINATION ${musly_CMAKEPKG_INSTALL_DIR}
)

#
# Install pkg-config configuration file
#

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/libmusly.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libmusly.pc
    @ONLY
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libmusly.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

#
# Packaging
#

if(NOT IS_SUBMODULE)
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
    set(CPACK_SOURCE_IGNORE_FILES
        /.git
        /.vscode
        /.travis\\\\.yml
        /\\\\.github
        /dist
        /.*build.*
        /\\\\.DS_Store
    )
    include(CPack)
endif()


#
# Uninstall
#

# Add uninstall target
add_custom_target(uninstall_musly
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MuslyUninstall.cmake
)
