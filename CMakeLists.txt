# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.
#
# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#          2014, Jan Schlueter <jan.schlueter@ofai.at>

cmake_minimum_required(VERSION 3.23)

project(
    musly
    VERSION "0.2"
    LANGUAGES C CXX
)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(GNUInstallDirs)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_STATIC "Make a static build of libmusly" OFF)
if (BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF)
else ()
    set(BUILD_SHARED_LIBS ON)
endif ()

option(BUILD_TEST "Build selftest executable" OFF)


# make sure Eigen3 library is present
include(FetchContent)
FetchContent_Declare(Eigen3
    URL      https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
    URL_HASH SHA256=8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    INSTALL_COMMAND ""
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS NO_MODULE
)

# can be replaced with FetchContent_MakeAvailable(Eigen3), when CMake >= 3.28
FetchContent_GetProperties(Eigen3)
if(NOT eigen3_POPULATED)
  FetchContent_Populate(Eigen3)
  add_subdirectory(${eigen3_SOURCE_DIR} ${eigen3_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


add_subdirectory(libresample)
add_subdirectory(libmusly)

# when the project is included as dependency from another
# project, provide additional targets
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCEDIR)
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


