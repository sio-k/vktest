#!/bin/bash
# stop on error
set -e

# make sure we're running in the directory where the script is
cd "$(dirname "$(realpath "$0")")"

# run cmake
mkdir -p build
cd build
cmake ..

