#!/bin/sh

# Update the copy of python-ihm inside the IMP repository.

SCRIPT_DIR="$(dirname "$0")"

source "${SCRIPT_DIR}/../dev_tools/git/subtree-utils.sh"

pull modules/core/dependency/python-ihm https://github.com/ihmwg/python-ihm.git main ihmwg/python-ihm
