#!/bin/sh

# Update the copy of RMF inside the IMP repository.

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

source "${SCRIPT_DIR}/subtree-utils.sh"

pull modules/rmf/dependency/RMF git@github.com:salilab/rmf.git develop
