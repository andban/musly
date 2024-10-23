# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#     2014-2016, Jan Schlueter <jan.schlueter@ofai.at>
#

add_library(libmusly)

target_sources(libmusly
    PRIVATE
        src/kissfft/kiss_fft.c
        src/kissfft/kiss_fftr.c
        src/methods/mandelellis.cpp
        src/methods/timbre.cpp
        src/resampler.cpp
        src/plugins.cpp
        src/method.cpp
        src/decoder.cpp
        src/windowfunction.cpp
        src/powerspectrum.cpp
        src/melspectrum.cpp
        src/discretecosinetransform.cpp
        src/mfcc.cpp
        src/gaussianstatistics.cpp
        src/mutualproximity.cpp
        src/lib.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS include FILES
        include/musly/musly.h
        include/musly/musly_types.h
)

target_link_libraries(libmusly 
    musly_resample Eigen3::Eigen
)
target_include_directories(libmusly
    PRIVATE src/ include/ Eigen3::Eigen musly_resample
    INTERFACE $<INSTALL_INTERFACE:include>
)

# This defines MUSLY_BUILDING_LIBRARY *only when building as shared library*,
# allowing us to use __declspec(dllexport) instead of dllimport on Windows.
set_target_properties(libmusly PROPERTIES 
    PREFIX ""
    DEFINE_SYMBOL MUSLY_BUILDING_LIBRARY
)

target_compile_definitions(libmusly
    PRIVATE -DMUSLY_VERSION="${PROJECT_VERSION}"
)

if(BUILD_STATIC)
    target_compile_definitions(libmusly
        PUBLIC -DMUSLY_STATIC
    )
endif()

if (LINUX)
    find_package(LibAV 0.8 COMPONENTS avcodec avformat avutil REQUIRED)
else()
    find_package(LibAV 0.8 COMPONENTS avcodec avformat avutil)
endif()

if(LIBAV_FOUND)
    target_sources(libmusly PRIVATE src/decoders/libav.cpp)
    target_link_libraries(libmusly ${LIBAV_LIBRARIES})
    target_include_directories(libmusly PRIVATE ${LIBAV_INCLUDE_DIRS})
    target_compile_definitions(libmusly PRIVATE -DMUSLY_DECODER_LIBAV)

    if(EXISTS "${LIBAV_INCLUDE_DIRS}/libavutil/channel_layout.h")
        set_source_files_properties(decoders/libav.cpp PROPERTIES
            COMPILE_FLAGS "-DHAVE_AVUTIL_CHANNEL_LAYOUT"
        )
    endif()
endif()

if(APPLE)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    mark_as_advanced(AUDIOTOOLBOX_LIBRARY COREFOUNDATION_LIBRARY)

    target_sources(libmusly 
        PRIVATE src/decoders/coreaudio.cpp
    )
    target_link_libraries(libmusly 
        ${AUDIOTOOLBOX_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
    )
    target_compile_definitions(libmusly
        PRIVATE -DMUSLY_DECODER_COREAUDIO
    )
endif()

if(WIN32 OR MINGW)
    target_sources(libmusly 
        PRIVATE src/decoders/mediafoundation.cpp
    )
    target_link_libraries(libmusly 
        ws2_32
        Mf
        Mfplat
        mfreadwrite
        mfuuid
        ole32
    )
    target_compile_definitions(libmusly
        PRIVATE -DMUSLY_DECODER_MEDIAFOUNDATION -DNOMINMAX -D_USE_MATH_DEFINES)
endif()

install(TARGETS libmusly 
    FILE_SET HEADERS
    LIBRARY
    ARCHIVE
)