#!/bin/sh

# Script to generate a simple IMP installer
#
# First build and install IMP itself with something like
#
# scons -j3 destdir=/tmp/imp-1.0 prefix=/usr/local \
#           includepath=/opt/local/include modeller=/Library/modeller-9v7/ \
#           install


VER=1.0
DESTDIR=/tmp/imp-${VER}
SOURCE=/Users/ben/imp-${VER}
PREFIX=/usr/local
PYTHON=2.3
TARGET_OSX_VER=10.4

mkdir -p ${DESTDIR}/Library/Python/${PYTHON}/site-packages/
echo "${PREFIX}/lib/python${PYTHON}/site-packages/" > ${DESTDIR}/Library/Python/${PYTHON}/site-packages/IMP.pth
cd ${DESTDIR}${PREFIX}/lib && for lib in *.dylib; do
  # Remove DESTDIR from shared library id
  install_name_tool -id ${PREFIX}/lib/$lib $lib
  for dep in *.dylib; do
    install_name_tool -change ${DESTDIR}${PREFIX}/lib/$dep \
                              ${PREFIX}/lib/$dep $lib
  done
  # Remove build directory from Python extension library id
  for py in python*/site-packages/*.so; do
    install_name_tool -change ${SOURCE}/build/lib/$lib \
                              ${PREFIX}/lib/$lib $py
  done
done

cd /tmp
mkdir imp-${VER}-package
/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker \
       -build -i ${SOURCE}/tools/mac/Info.plist \
       -d ${SOURCE}/tools/mac/Description.plist -ds \
       -p "imp-${VER}-package/IMP ${VER} ${TARGET_OSX_VER}.pkg" -f ${DESTDIR}
hdiutil create -fs HFS+ -volname "IMP ${VER} for OS X ${TARGET_OSX_VER}" \
               -srcfolder imp-${VER}-package IMP-${VER}-${TARGET_OSX_VER}.dmg
hdiutil internet-enable -yes IMP-${VER}-${TARGET_OSX_VER}.dmg
