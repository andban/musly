# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#     2014-2016, Jan Schlueter <jan.schlueter@ofai.at>

add_subdirectory(
    libresample)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external")
    add_subdirectory(
        external)
    set_source_files_properties(${LIBMUSLY_EXTERNAL}
        PROPERTIES COMPILE_FLAGS "${LIBMUSLY_EXTERNAL_FLAGS}")
    set_source_files_properties(lib.cpp
        PROPERTIES COMPILE_FLAGS "-DLIBMUSLY_EXTERNAL ${LIBMUSLY_EXTERNAL_FLAGS}")
endif()

if(EXISTS "${LIBAV_INCLUDE_DIRS}/libavutil/channel_layout.h")
    set_source_files_properties(decoders/libav.cpp
        PROPERTIES COMPILE_FLAGS "-DHAVE_AVUTIL_CHANNEL_LAYOUT")
endif()

if(USE_OPENMP AND OPENMP_FOUND)
    # disable OpenMP for kiss FFT, it slows things down terribly
    set_source_files_properties(kissfft/kiss_fft.c
        PROPERTIES COMPILE_FLAGS "-U_OPENMP")
endif()

include_directories(
    ${LIBMUSLY_INCLUDE}
    ${EIGEN3_INCLUDE_DIR}
    ${LIBAV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR})

add_library(libmusly
    kissfft/kiss_fft.c
    kissfft/kiss_fftr.c
    methods/mandelellis.cpp
    methods/timbre.cpp
    decoders/libav.cpp
    resampler.cpp
    plugins.cpp
    method.cpp
    decoder.cpp
    windowfunction.cpp
    powerspectrum.cpp
    melspectrum.cpp
    discretecosinetransform.cpp
    mfcc.cpp
    gaussianstatistics.cpp
    mutualproximity.cpp
    lib.cpp
    ${LIBMUSLY_EXTERNAL})
target_compile_features(libmusly PUBLIC cxx_std_17)

if(BUILD_STATIC)
    set_target_properties(libmusly
        PROPERTIES COMPILE_FLAGS "-DBUILD_STATIC")
endif()

# This defines MUSLY_BUILDING_LIBRARY *only when building as shared library*,
# allowing us to use __declspec(dllexport) instead of dllimport on Windows.
set_target_properties(libmusly
    PROPERTIES DEFINE_SYMBOL MUSLY_BUILDING_LIBRARY)

target_link_libraries(libmusly
    ${LIBMUSLY_LIBS}
    ${LIBAV_LIBRARIES})
if(WIN32 OR MINGW)
    # link against winsock2 for ntohl() and htonl()
    target_link_libraries(libmusly ws2_32)
endif()

set_target_properties(libmusly
    PROPERTIES PREFIX "")

install(TARGETS libmusly
    DESTINATION lib)
