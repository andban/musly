# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013, Dominik Schnitzer <dominik@schnitzer.at>

include_directories(
    ${EIGEN3_INCLUDE_DIR})


add_executable(musly
    tools.cpp
    fileiterator.cpp
    programoptions.cpp
    collectionfile.cpp
    main.cpp)

target_link_libraries(musly
    libmusly)

target_compile_features(musly PUBLIC cxx_std_17)

install(TARGETS musly DESTINATION bin)
