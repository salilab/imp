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
cp -r tools/debian-ppa debian

# Add all module directories to imp.install
(cd modules && ${TOP_DIR}/tools/debian-ppa/make-imp-install.py < ${TOP_DIR}/tools/debian-ppa/imp.install > ${TOP_DIR}/debian/imp.install)

rm debian/make-package.sh debian/make-imp-install.py
sed -i -e "s/\@CODENAME\@/$CODENAME/g" debian/changelog

if [ "${CODENAME}" = "focal" ]; then
  sed -i -e "s/debhelper-compat (= 13)/debhelper-compat (= 12)/" debian/control
fi

# Workaround gcc -frounding-math bug; see
# https://bugzilla.redhat.com/show_bug.cgi?id=2085189
if [ "${CODENAME}" = "noble" ]; then
  perl -pi -e "s/CXXFLAGS :=.*/CXXFLAGS := -std=c++20/" debian/rules
fi

dpkg-buildpackage -S
