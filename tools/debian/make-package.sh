#!/bin/sh
# Build a Debian package from source

if [ $# -ne 1 ]; then
  echo "Usage: $0 <IMP version>"
  exit 1
fi

VERSION=$1
DATE=`date -R`

# Make sure we can find the rest of our input files
TOOL_DIR=`dirname "$0"`
# Get absolute path to IMP top dir
TOP_DIR=`cd "${TOOL_DIR}/../.." && pwd`

imp_dir_name=`basename ${TOP_DIR}`

cd ${TOP_DIR} || exit 1
rm -rf debian
cp -r tools/debian/ . || exit 1
rm debian/make-package.sh || exit 1
perl -pi -e "s/\@VERSION\@/$VERSION/; s/\@DATE\@/$DATE/" debian/changelog  || exit 1
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
