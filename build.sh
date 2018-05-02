#!/bin/bash
# stop on error
set -e

# set working directory of this script to be the directory the script is in
cd "$(dirname "$(realpath "$0")")"

astyle --options=astylerc ./*.c ./*.h

cmake --build build --target vktest --config Debug
