#!/bin/bash

# exit if a command fails
set -e

# make directory for build output
mkdir -p lib

# build library
cd build
make OPENCV=/usr/local LIBRARY_PATH=/usr/local/lib