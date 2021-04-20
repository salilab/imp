#!/bin/sh

# Update the copy of cmake_modules inside the RMF repository.

SCRIPT_DIR="$(dirname "$0")"

source "${SCRIPT_DIR}/../dev_tools/git/subtree-utils.sh"

pull cmake_modules git@github.com:salilab/cmake_modules.git main salilab/cmake_modules
