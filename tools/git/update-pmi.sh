#!/bin/sh

# Update the copy of PMI inside the IMP repository.

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

source "${SCRIPT_DIR}/../dev_tools/git/subtree-utils.sh"

pull modules/pmi git@github.com:salilab/pmi.git develop
