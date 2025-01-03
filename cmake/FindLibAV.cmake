# Module for locating libav.
#
# Customizable variables:
#   LIBAV_ROOT_DIR
#     Specifies libav's root directory.
#
# Read-only variables:
#   LIBAV_FOUND
#     Indicates whether the library has been found.
#
#   LIBAV_INCLUDE_DIRS
#      Specifies libav's include directory.
#
#   LIBAV_LIBRARIES
#     Specifies libav libraries that should be passed to target_link_libararies.
#
#   LIBAV_<COMPONENT>_LIBRARIES
#     Specifies the libraries of a specific <COMPONENT>.
#
#   LIBAV_<COMPONENT>_FOUND
#     Indicates whether the specified <COMPONENT> was found.
#
#
# Copyright (c) 2013 Sergiu Dotenco
# Changed 2015 by Jan Schlüter: https://bitbucket.org/sergiu/libav-cmake/issue/2
# Changed 2016 by Jan Schlüter: support both ffmpeg and libav
# Changed 2017 by Sergey Smirnov: support architecture-specific include dirs
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTLIBAVLAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

INCLUDE (FindPackageHandleStandardArgs)

IF (CMAKE_VERSION VERSION_GREATER 2.8.7)
  SET (_LIBAV_CHECK_COMPONENTS FALSE)
ELSE (CMAKE_VERSION VERSION_GREATER 2.8.7)
  SET (_LIBAV_CHECK_COMPONENTS TRUE)
ENDIF (CMAKE_VERSION VERSION_GREATER 2.8.7)

FIND_PATH (LIBAV_ROOT_DIR
  NAMES include/libavcodec/avcodec.h
        include/libavdevice/avdevice.h
        include/libavfilter/avfilter.h
        include/libavutil/avutil.h
        include/libswscale/swscale.h
  PATHS ENV LIBAVROOT
  DOC "libav root directory")

FIND_PATH (LIBAV_INCLUDE_DIR
  NAMES libavcodec/avcodec.h
        libavdevice/avdevice.h
        libavfilter/avfilter.h
        libavutil/avutil.h
        libswscale/swscale.h
  HINTS ${LIBAV_ROOT_DIR}
  PATH_SUFFIXES include include/ffmpeg
  DOC "libav include directory")

if (NOT LibAV_FIND_COMPONENTS)
  set (LibAV_FIND_COMPONENTS avcodec avdevice avfilter avformat avutil swscale)
endif (NOT LibAV_FIND_COMPONENTS)

FOREACH (_LIBAV_COMPONENT ${LibAV_FIND_COMPONENTS})
  STRING (TOUPPER ${_LIBAV_COMPONENT} _LIBAV_COMPONENT_UPPER)
  SET (_LIBAV_LIBRARY_BASE LIBAV_${_LIBAV_COMPONENT_UPPER}_LIBRARY)

  FIND_LIBRARY (${_LIBAV_LIBRARY_BASE}
    NAMES ${_LIBAV_COMPONENT}
    HINTS ${LIBAV_ROOT_DIR}
    PATH_SUFFIXES bin lib
    DOC "libav ${_LIBAV_COMPONENT} library")

  MARK_AS_ADVANCED (${_LIBAV_LIBRARY_BASE})

  SET (LIBAV_${_LIBAV_COMPONENT_UPPER}_FOUND TRUE)

  IF (${_LIBAV_LIBRARY_BASE})
    # setup the LIBAV_<COMPONENT>_LIBRARIES variable
    SET (LIBAV_${_LIBAV_COMPONENT_UPPER}_LIBRARIES ${${_LIBAV_LIBRARY_BASE}})
    LIST (APPEND LIBAV_LIBRARIES ${LIBAV_${_LIBAV_COMPONENT_UPPER}_LIBRARIES})
    LIST (APPEND _LIBAV_ALL_LIBS ${${_LIBAV_LIBRARY_BASE}})
  ELSE (${_LIBAV_LIBRARY_BASE})
    SET (LIBAV_${_LIBAV_COMPONENT_UPPER}_FOUND FALSE)

    IF (_LIBAV_CHECK_COMPONENTS)
      LIST (APPEND _LIBAV_MISSING_LIBRARIES ${_LIBAV_LIBRARY_BASE})
    ENDIF (_LIBAV_CHECK_COMPONENTS)
  ENDIF (${_LIBAV_LIBRARY_BASE})

  SET (LibAV_${_LIBAV_COMPONENT}_FOUND ${LIBAV_${_LIBAV_COMPONENT_UPPER}_FOUND})
ENDFOREACH (_LIBAV_COMPONENT ${LibAV_FIND_COMPONENTS})

SET (LIBAV_INCLUDE_DIRS ${LIBAV_INCLUDE_DIR})

IF (DEFINED _LIBAV_MISSING_COMPONENTS AND _LIBAV_CHECK_COMPONENTS)
  IF (NOT LibAV_FIND_QUIETLY)
    MESSAGE (STATUS "One or more libav components were not found:")
    # Display missing components indented, each on a separate line
    FOREACH (_LIBAV_MISSING_COMPONENT ${_LIBAV_MISSING_COMPONENTS})
      MESSAGE (STATUS "  " ${_LIBAV_MISSING_COMPONENT})
    ENDFOREACH (_LIBAV_MISSING_COMPONENT ${_LIBAV_MISSING_COMPONENTS})
  ENDIF (NOT LibAV_FIND_QUIETLY)
ENDIF (DEFINED _LIBAV_MISSING_COMPONENTS AND _LIBAV_CHECK_COMPONENTS)

# Determine library's version

FIND_PROGRAM (LIBAV_EXECUTABLE NAMES ffmpeg avconv
  HINTS ${LIBAV_ROOT_DIR}
  PATH_SUFFIXES bin
  DOC "ffmpeg or avconv executable")

IF (LIBAV_EXECUTABLE)
  EXECUTE_PROCESS (COMMAND ${LIBAV_EXECUTABLE} -version
    OUTPUT_VARIABLE _LIBAV_EXECUTABLE_OUTPUT ERROR_QUIET)

  GET_FILENAME_COMPONENT(_CMD_NAME ${LIBAV_EXECUTABLE} NAME_WLE)
  STRING (REGEX REPLACE
    ".*${_CMD_NAME}([ \t]+version)?[ \t]+v?([0-9.]*).*" "\\2"
    LIBAV_VERSION "${_LIBAV_EXECUTABLE_OUTPUT}")

  IF (LIBAV_VERSION)
    STRING (REGEX REPLACE "([0-9]+)\\.([0-9]+)(\\.([0-9]+))?" "\\1"
      LIBAV_VERSION_MAJOR "${LIBAV_VERSION}")
    STRING (REGEX REPLACE "([0-9]+)\\.([0-9]+)(\\.([0-9]+))?" "\\2"
      LIBAV_VERSION_MINOR "${LIBAV_VERSION}")

    IF ("${LIBAV_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
      STRING (REGEX REPLACE "([0-9]+)\\.([0-9]+)(\\.([0-9]+))?" "\\3"
        LIBAV_VERSION_PATCH "${LIBAV_VERSION}")
      SET (LIBAV_VERSION_COMPONENTS 3)
    ELSEIF ("${LIBAV_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)$")
      SET (LIBAV_VERSION_COMPONENTS 2)
    ELSEIF ("${LIBAV_VERSION}" MATCHES "^([0-9]+)$")
      # mostly developer/alpha/beta versions
      SET (LIBAV_VERSION_COMPONENTS 2)
      SET (LIBAV_VERSION_MINOR 0)
      SET (LIBAV_VERSION "${LIBAV_VERSION}.0")
    ENDIF ("${LIBAV_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
  ELSE (LIBAV_VERSION)
    # clear variable if empty, can trap version check otherwise
    UNSET (LIBAV_VERSION)
  ENDIF (LIBAV_VERSION)

ENDIF (LIBAV_EXECUTABLE)

IF (WIN32)
  FIND_PROGRAM (LIB_EXECUTABLE NAMES lib
    HINTS "$ENV{VS110COMNTOOLS}/../../VC/bin"
          "$ENV{VS100COMNTOOLS}/../../VC/bin"
          "$ENV{VS90COMNTOOLS}/../../VC/bin"
          "$ENV{VS71COMNTOOLS}/../../VC/bin"
          "$ENV{VS80COMNTOOLS}/../../VC/bin"
    DOC "Library manager")

  MARK_AS_ADVANCED (LIB_EXECUTABLE)
ENDIF (WIN32)

MACRO (GET_LIB_REQUISITES LIB REQUISITES)
  IF (LIB_EXECUTABLE)
    GET_FILENAME_COMPONENT (_LIB_PATH ${LIB_EXECUTABLE} PATH)

    IF (MSVC)
      # Do not redirect the output
      UNSET (ENV{VS_UNICODE_OUTPUT})
    ENDIF (MSVC)

    EXECUTE_PROCESS (COMMAND ${LIB_EXECUTABLE} /nologo /list ${LIB}
      WORKING_DIRECTORY ${_LIB_PATH}/../../Common7/IDE
      OUTPUT_VARIABLE _LIB_OUTPUT ERROR_QUIET)

    STRING (REPLACE "\n" ";" "${REQUISITES}" "${_LIB_OUTPUT}")
    LIST (REMOVE_DUPLICATES ${REQUISITES})
  ENDIF (LIB_EXECUTABLE)
ENDMACRO (GET_LIB_REQUISITES)

IF (_LIBAV_ALL_LIBS)
  # collect lib requisites using the lib tool
  FOREACH (_LIBAV_COMPONENT ${_LIBAV_ALL_LIBS})
    GET_LIB_REQUISITES (${_LIBAV_COMPONENT} _LIBAV_REQUISITES)
  ENDFOREACH (_LIBAV_COMPONENT)
ENDIF (_LIBAV_ALL_LIBS)

IF (NOT LIBAV_BINARY_DIR)
  SET (_LIBAV_UPDATE_BINARY_DIR TRUE)
ELSE (NOT LIBAV_BINARY_DIR)
  SET (_LIBAV_UPDATE_BINARY_DIR FALSE)
ENDIF (NOT LIBAV_BINARY_DIR)

SET (_LIBAV_BINARY_DIR_HINTS bin)

IF (_LIBAV_REQUISITES)
  FIND_FILE (LIBAV_BINARY_DIR NAMES ${_LIBAV_REQUISITES}
      HINTS ${LIBAV_ROOT_DIR}
    PATH_SUFFIXES ${_LIBAV_BINARY_DIR_HINTS} NO_DEFAULT_PATH)
ENDIF (_LIBAV_REQUISITES)

IF (LIBAV_BINARY_DIR AND _LIBAV_UPDATE_BINARY_DIR)
  SET (_LIBAV_BINARY_DIR ${LIBAV_BINARY_DIR})
  UNSET (LIBAV_BINARY_DIR CACHE)

  IF (_LIBAV_BINARY_DIR)
    GET_FILENAME_COMPONENT (LIBAV_BINARY_DIR ${_LIBAV_BINARY_DIR} PATH)
  ENDIF (_LIBAV_BINARY_DIR)
ENDIF (LIBAV_BINARY_DIR AND _LIBAV_UPDATE_BINARY_DIR)

SET (LIBAV_BINARY_DIR ${LIBAV_BINARY_DIR} CACHE PATH "libav binary directory")

MARK_AS_ADVANCED (LIBAV_INCLUDE_DIR LIBAV_BINARY_DIR)

IF (NOT _LIBAV_CHECK_COMPONENTS)
 SET (_LIBAV_FPHSA_ADDITIONAL_ARGS HANDLE_COMPONENTS)
ENDIF (NOT _LIBAV_CHECK_COMPONENTS)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (LibAV
  REQUIRED_VARS LIBAV_INCLUDE_DIR ${_LIBAV_MISSING_LIBRARIES}
  VERSION_VAR LIBAV_VERSION
  ${_LIBAV_FPHSA_ADDITIONAL_ARGS})

