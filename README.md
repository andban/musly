Musly
=====

**Note**: This fork of Musly just contains updates to the cmake scripts for better compability with newer versions, some changes to compile it with MSVC, and added makeshift native decoders for Windows and macOS.


(c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
and 2014-2016, Jan Schlüter <jan.schlueter@ofai.at>

Musly is a program and library for high performance audio music similarity
computation. Musly only uses the audio signal when computing similarities!
No meta data is used in the similarity computation process.

The source code is released under the MPL 2.0 see the file <musly/COPYING>

* The main Musly website is located at: <http://www.musly.org>
* Please fork me at: <https://github.com/dominikschnitzer/musly>
* Please report any Musly bug at:
  <https://github.com/dominikschnitzer/musly/issues>
* Documentation is found in the local <musly/doc> directory and on the main
  website.
* Build status: [![Build Status](https://travis-ci.org/dominikschnitzer/musly.svg?branch=master)](https://travis-ci.org/dominikschnitzer/musly)

## Version History ##

### VERSION 0.2 (under development) ###
Not released yet.

The second public Musly release. It includes the following changes and
additions:

-   `musly_jukebox_addtracks()` requires a fifth argument `generate_ids`: If
    nonzero, sequential track IDs are generated by Musly (as before), and if
    zero, track IDs are given by the user.
-   `musly_jukebox_removetracks()`, `musly_jukebox_trackcount()`,
    `musly_jukebox_maxtrackid()`, and `musly_jukebox_gettrackids() are added
    to the API.
-   `musly_jukebox_guessneighbors_filtered()` is added to the API, allowing
    the filter step to be constrained to a subset of registered tracks.
-   `musly_track_analyze_audiofile()` accepts two parameters `excerpt_length`
    and `excerpt_start` controlling the length and position of the audio
    excerpt to be decoded and analyzed instead of the previous `max_seconds`,
    and the libav-based decoder's performance has been improved significantly.
-   The musly command line client can compute sparse distance matrices (-s).
-   When configured with `-DUSE_OPENMP=On` in `cmake` (or enabling the option
    in `ccmake`), the musly command line client and library are built with
    OpenMP support. The command line client parallelizes several computations.
-   `musly_jukebox_binsize()`, `musly_jukebox_tobin()`,
    `musly_jukebox_frombin()`, `musly_jukebox_tostream()`,
    `musly_jukebox_fromstream()`, `musly_jukebox_tofile()` and
    `musly_jukebox_fromfile()` are added to the API, allowing to serialize
    and restore the internal state of a jukebox influenced by calls to
    `musly_jukebox_setmusicstyle()`, `musly_jukebox_addtracks()` and
    `musly_jukebox_removetracks()`. The command line client makes use of this
    when given the option -j or -J.
-   `musly_findmin()` is added to the API.

### VERSION 0.1 ###
Released on 30 Jan 2014.

The first public Musly release. It includes two basic audio music similarity
measures:

-   *mandelellis* (as published in "M. Mandel and D. Ellis: Song-level
    features and support vector machines for music classification. In the
    proceedings of the 6th International Conference on Music Information Retrieval,
    ISMIR, 2005)
    
-   *timbre* the mandelellis similarity measure tweaked for best
    results. We use a 25 MFCCs representation, the Jensen-Shannon divergence
    and normalize the similarities with Mutual Proximity
    (D. Schnitzer et al.: Using mutual proximity to improve
    content-based audio similarity. In the proceedings of the 12th
    International Society for Music Information Retrieval
    Conference, ISMIR, 2011).

For benchmarks and qualitative comparisons between other audio music
similarity measures visit <http://www.musly.org>.


## Installation ##

Musly uses the CMake build system, and depends on Eigen 3 and ffmpeg or libav
0.8 or above.

### Ubuntu prerequisites ###

On Ubuntu, all requirements can be installed with:
`sudo apt-get install cmake libeigen3-dev libavcodec-dev libavformat-dev libavutil-dev`

### Windows prerequisites ###

For Windows, the easiest way seems to be to use MSYS2. Install it and launch
the "MinGW-w64 Win64 Shell" from the start menu (*not* the "MSYS2 Shell"),
then run:

```bash
# Update repository information
pacman -Sy
# Install prerequisites
pacman -S mingw-w64-x86_64-{gcc,cmake-git,pkgconf,eigen3} make wget p7zip
# Download and install precompiled ffmpeg libraries
wget https://ffmpeg.zeranoe.com/builds/win64/dev/ffmpeg-latest-win64-dev.7z
wget https://ffmpeg.zeranoe.com/builds/win64/shared/ffmpeg-latest-win64-shared.7z
7z x ffmpeg-latest-win64-dev.7z
7z x ffmpeg-latest-win64-shared.7z
mv ffmpeg-latest-win64-dev/include/* /usr/include
mv ffmpeg-latest-win64-dev/lib/* /usr/lib
mv ffmpeg-latest-win64-shared/bin/* /usr/bin
rm -rf ffmpeg-latest-win64-*
# Download and extract latest musly snapshot from git
wget https://github.com/dominikschnitzer/musly/archive/master.zip
7z x master.zip
cd musly-master
```

### Compilation ###

To compile Musly, it is good practice to create an empty directory to build in.
This can be placed inside your Musly source tree. Assuming you are in the source
tree, run the following commands to build and install Musly (**Note**: For
Windows, add `-G"MSYS Makefiles"` to both of the `cmake` lines below):

```bash
$ cmake -S . -B build
$ cmake --build build 
$ sudo cmake --install build
```

To perform a self-test of the library, optionally run (still inside `build`):

```bash
cmake -S . -B build -DBUILD_TEST=ON
cmake --build build
ctest --test-dir build
```

It should end with `100% tests passed, 0 tests failed`.


## Command Line Tool ##

The command line interface is able to:

* Generate M3U playlists for music collections.
* Output full or sparse similarity matrices for more in-depth research of the
  audio music similarity functions. It uses the music-ir.org MIREX format
  (see <musly/doc/MIREX-DistanceMatrix.md>)
* Additionally the music similarity features can be ouput in text format
  to ease reuse of the features.
  
The command line tool is called "musly". Use "musly -h" to read about all
available options. See <http://www.musly.org> for more information.


## Library ##

Please see the library documentation in <doc/html> directory or on-line.


## Development ##

To help Musly development and fix bugs the most convenient way is to use
Eclipse. To generate an Eclipse project run:

```bash
cmake -G"Eclipse CDT4 - Unix Makefiles"\
      -DCMAKE_ECLIPSE_EXECUTABLE=/path/to/eclipse/eclipse \
      /path/to/musly-src
```

from an empty directory. The directory is filled with Eclipse project
information which can be imported as an Eclipse project.

