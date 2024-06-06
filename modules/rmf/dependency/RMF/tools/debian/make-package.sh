#!/bin/sh
# Build a Debian package from source

set -e

CODENAME=`lsb_release -c -s`

# Make sure we can find the rest of our input files
TOOL_DIR=`dirname "$0"`
# Get absolute path to top dir
TOP_DIR=`cd "${TOOL_DIR}/../.." && pwd`

cd ${TOP_DIR}
rm -rf debian
cp -r tools/debian/ .
rm debian/make-package.sh
sed -i -e "s/\@CODENAME\@/$CODENAME/g" debian/changelog

if [ "${CODENAME}" = "focal" ]; then
  sed -i -e "s/debhelper-compat (= 13)/debhelper-compat (= 12)/" debian/control
fi

dpkg-buildpackage -S
