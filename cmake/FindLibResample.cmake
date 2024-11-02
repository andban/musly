#[=======================================================================[.rst:
FindLibResample
---------------

Locate libresample.

This module defines:

``LIBRESAMPLE_FOUND``
  if false, do not try to link with libresample
``LIBRESAMPLE_LIBRARY``
  location of libresample library
``LIBRESAMLE_INCLUDE_DIR``
  location of libresample.h
``resample::resample``
  target for libresample for usage in target_link_libraries()

#]=======================================================================]
include(FindPackageHandleStandardArgs)

find_path(LIBRESAMPLE_INCLUDE_DIR
    NAMES libresample.h
    HINTS
        ENV LIBRESAMPLE_DIR
    PATH_SUFFIXES include
)

find_library(LIBRESAMPLE_LIBRARY
    NAMES resample
    HINTS
        ENV LIBRESAMPLE_DIR
    PATH_SUFFIXES lib
)

set(_libresample_library_type SHARED)
if(LIBRESAMPLE_LIBRARY)
    if(UNIX)
        get_filename_component(_libresample_ext ${LIBRESAMPLE_LIBRARY} EXT)
        if(_libresample_ext STREQUAL ${CMAKE_STATIC_LIBRARY_SUFFIX})
            set(_libresample_library_type STATIC)
        endif()
        unset(_libresample_ext)
    endif()
endif()

find_package_handle_standard_args(
    LibResample
    REQUIRED_VARS 
        LIBRESAMPLE_INCLUDE_DIR 
        LIBRESAMPLE_LIBRARY
)

if(LIBRESAMPLE_FOUND)
    message(STATUS "-I ${LIBRESAMPLE_INCLUDE_DIR}, -L ${LIBRESAMPLE_LIBRARY}")
    add_library(resample::resample ${_libresample_library_type} IMPORTED)

    set_target_properties(resample::resample PROPERTIES
        IMPORTED_LOCATION "${LIBRESAMPLE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBRESAMPLE_INCLUDE_DIR}"
    )
endif()

unset(_libresample_library_type)

