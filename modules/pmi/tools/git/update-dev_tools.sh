#!/bin/sh

# Update the copy of dev_tools inside the PMI repository.

SCRIPT_DIR="$(dirname "$0")"

source "${SCRIPT_DIR}/../dev_tools/git/subtree-utils.sh"

pull tools/dev_tools git@github.com:salilab/developer_tools.git main salilab/developer_tools
