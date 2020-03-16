#!/bin/sh
# Build a Debian package from source

if [ $# -ne 1 ]; then
  echo "Usage: $0 <IMP version>"
  exit 1
fi

VERSION=$1
DATE=`date -R`
CODENAME=`lsb_release -c -s`

# Make sure we can find the rest of our input files
TOOL_DIR=`dirname "$0"`
# Get absolute path to IMP top dir
TOP_DIR=`cd "${TOOL_DIR}/../.." && pwd`

imp_dir_name=`basename ${TOP_DIR}`

cd ${TOP_DIR} || exit 1
rm -rf debian
cp -r tools/debian/ . || exit 1

# Add all module directories to imp.install
(cd modules && ${TOP_DIR}/tools/debian/make-imp-install.py < ${TOP_DIR}/tools/debian/imp.install > ${TOP_DIR}/debian/imp.install)

rm debian/make-package.sh debian/make-imp-install.py || exit 1
perl -pi -e "s/\@VERSION\@/$VERSION/; s/\@DATE\@/$DATE/; s/\@CODENAME\@/$CODENAME/;" debian/changelog  || exit 1
if [ "${CODENAME}" = "xenial" -o "${CODENAME}" = "bionic" ]; then
  # CGAL cmake support on Xenial or later requires Qt5 headers too
  perl -pi -e "s/libcgal\-dev/libcgal-dev, libcgal-qt5-dev/g" debian/control || exit 1
  # python2-dev is called python-dev on older Ubuntu
  perl -pi -e "s/python2\-dev/python-dev/g" debian/control || exit 1
fi
# Older distributions don't support python3-protobuf
if [ "${CODENAME}" = "xenial" -o "${CODENAME}" = "trusty" ]; then
  perl -pi -e "s/, python3-protobuf//" debian/control || exit 1
fi
cd .. || exit 1
if [ "${imp_dir_name}" != "imp" ]; then
  mv "${imp_dir_name}" imp
fi
tar -czf imp_${VERSION}.orig.tar.gz imp || exit 1
cd imp || exit 1
dpkg-buildpackage -us -uc || exit 1
rm -rf ${TOP_DIR}/debian

if [ "${imp_dir_name}" != "imp" ]; then
  cd ${TOP_DIR}/..
  mv imp "${imp_dir_name}"
fi
