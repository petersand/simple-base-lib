simple-base-lib
===============

This is version 0.0.4.

## Data path configuration

The library expects to find a path.conf in the application's current directory 
specifying the path of the application's data directory.

## Optional dependencies

The following flags can be defined to enable code that depends on external
libraries:

* USE_OPENCV (for image processing functions and image/video I/O)
* USE_CDT (for triangulation functions)
* USE_ZLIB (for gzip support in the File class)
* USE_PYTHON (for scripting)

## Recommended Visual Studio settings

* General / Character set: "Not Set"
* C/C++ / General / Detect 64-bit Issues: No
* C/C++ / Preprocessor Definitions: WIN32;_CONSOLE;_CRT_SECURE_NO_DEPRECATE

## Recommended gcc flags

* `-Wall -Wstrict-aliasing`
* `-msse2 -mfpmath=sse -march=i686`
* `-O2`

## Documentation

To generate documentation with doxygen, run `doxygen sbl_doxygen.txt` from the doc sub-directory.