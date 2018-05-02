#!/bin/bash
# run built game

# stop on error
set -e

# make sure we're running in the directory the script is in
cd "$(dirname "$(realpath "$0")")"

./build/vktest
