#!/bin/bash

# exit if a command fails
set -e

# go to the directory where this repository is located
# see .github/workflows/regress.yml for mount definition
cd /tmp

# make directory for build output
mkdir -p lib

# build library
cd build
make OPENCV=/usr/local LIBRARY_PATH=/usr/local/lib