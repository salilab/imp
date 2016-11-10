#!/bin/sh

# Update the copy of RMF inside the IMP repository.

SCRIPT_DIR="$(dirname "$0")"

source "${SCRIPT_DIR}/../dev_tools/git/subtree-utils.sh"

pull modules/rmf/dependency/RMF https://github.com/salilab/rmf.git develop salilab/rmf
