#!/bin/sh

# Script to generate a simple IMP installer for Mac
#
# First build and install IMP itself with something like
# scons -j3 destdir=<destdir> prefix=/usr/local install
#
# Then run this script with something like
# tools/mac/make-package.sh <destdir> <IMP version>
#
# e.g.
# scons -j3 destdir=/tmp/impinstall prefix=/usr/local install
# tools/mac/make-package.sh /tmp/impinstall 1.0


if [ $# -ne 2 ]; then
  echo "Usage: $0 <destdir> <IMP version>"
  exit 1
fi

DESTDIR=$1
VER=$2

TARGET_OSX_VER=`sw_vers -productVersion | cut -f 1-2 -d.`
case ${TARGET_OSX_VER} in
  10.4):
    PYTHON=2.3
    ;;
  10.6):
    PYTHON=2.6
    ;;
  *):
    echo "This script currently only works on Mac OS X 10.4 or 10.6 machines"
    exit 1
    ;;
esac

PREFIX=/usr/local

# Make sure we can find the rest of our input files
MAC_TOOL_DIR=`dirname "$0"`
# Make absolute path
MAC_TOOL_DIR=`cd "${MAC_TOOL_DIR}" && pwd`
if [ ! -f "${MAC_TOOL_DIR}/Info.plist.in" -o \
     ! -f "${MAC_TOOL_DIR}/Description.plist.in" ]; then
  echo "Could not find plist files in script directory"
  exit 1
fi

# Make sure that DESTDIR/PREFIX points to an IMP installation
# (libraries and binaries)
if [ ! -f ${DESTDIR}/${PREFIX}/lib/libimp.dylib ]; then
  echo "Could not find IMP libraries in ${DESTDIR}/${PREFIX}/lib/"
  exit 1
fi

if [ ! -f ${DESTDIR}/${PREFIX}/bin/foxs ]; then
  echo "Could not find foxs in ${DESTDIR}/${PREFIX}/bin/"
  exit 1
fi

echo "Making IMP.pth to add IMP Python modules to the Python path..."
mkdir -p ${DESTDIR}/Library/Python/${PYTHON}/site-packages/ || exit 1
echo "${PREFIX}/lib/python${PYTHON}/site-packages/" > ${DESTDIR}/Library/Python/${PYTHON}/site-packages/IMP.pth

# Determine current library install name path
LIBNAMEPATH=$( dirname `otool -L ${DESTDIR}/${PREFIX}/lib/libimp.dylib |grep libimp_compat|cut -d\( -f 1` )

echo "Setting library name paths and IDs..."
cd ${DESTDIR}/${PREFIX}/lib

# Get all C++ binaries (not Python scripts)
bins=""
for bin in ../bin/*; do
  if file $bin | grep -q 'Mach-O.*executable'; then
    bins="${bins} ${bin}"
  fi
done

for lib in *.dylib; do
  # Make sure library is correct
  install_name_tool -id ${PREFIX}/lib/$lib $lib || exit 1

  # Make sure install name paths of dependent libraries are correct
  for dep in *.dylib; do
    install_name_tool -change ${LIBNAMEPATH}/$dep \
                              ${PREFIX}/lib/$dep $lib || exit 1
  done
  for py in python*/site-packages/*.so; do
    install_name_tool -change ${LIBNAMEPATH}/$lib \
                              ${PREFIX}/lib/$lib $py || exit 1
  done

  # Update library name paths in IMP binaries
  for bin in ${bins}; do
    install_name_tool -change ${LIBNAMEPATH}/$lib \
                              ${PREFIX}/lib/$lib $bin || exit 1
  done
done

# Bundle non-standard library dependencies (e.g. boost) and make IMP libraries
# and binaries point to them
echo "Bundling non-standard library dependencies (e.g. Boost, GSL, HDF5)..."
BUNDLED_LIBS="/opt/local/lib/libboost_system-mt.dylib \
              /opt/local/lib/libboost_filesystem-mt.dylib \
              /opt/local/lib/libboost_program_options-mt.dylib \
              /opt/local/lib/libboost_thread-mt.dylib \
              /opt/local/lib/libfftw3.3.dylib \
              /opt/local/lib/libgsl.0.dylib \
              /opt/local/lib/libgslcblas.0.dylib \
              /opt/local/lib/libhdf5.7.dylib"
BUNDLED_LIB_DIR="${PREFIX}/lib/imp-3rd-party"
mkdir -p ${DESTDIR}/${BUNDLED_LIB_DIR} || exit 1

for lib in ${BUNDLED_LIBS}; do
  # Copy bundled library and update its id
  cp ${lib} ${DESTDIR}/${BUNDLED_LIB_DIR} || exit 1
  base=`basename $lib`
  install_name_tool -id ${BUNDLED_LIB_DIR}/$base \
                        ${DESTDIR}/${BUNDLED_LIB_DIR}/$base || exit 1

  # Make sure any references in the bundled lib to other bundled libs
  # are updated
  for dep in ${BUNDLED_LIBS}; do
    depbase=`basename $dep`
    install_name_tool -change ${dep} ${BUNDLED_LIB_DIR}/$depbase \
                              ${DESTDIR}/${BUNDLED_LIB_DIR}/$base || exit 1
  done

  # Make sure all IMP libraries and binaries point to the bundled lib
  for user in *.dylib ${bins} python*/site-packages/*.so; do
    install_name_tool -change ${lib} ${BUNDLED_LIB_DIR}/$base ${user} || exit 1
  done
done


# Make sure we don't link against any non-standard libraries that aren't bundled
otool -L *.dylib ${bins} python*/site-packages/*.so |grep -Ev '/usr/(local/)?lib|:'|sort -u > /tmp/non-standard.$$
if [ -s /tmp/non-standard.$$ ]; then
  echo "The following non-standard libraries are linked against, and were"
  echo "not bundled:"
  echo
  cat /tmp/non-standard.$$
  echo
  rm -f /tmp/non-standard.$$
  exit 1
else
  rm -f /tmp/non-standard.$$
fi

cd /tmp

# Substitute version number in plist files
for fname in Info Description; do
  sed -e "s/@VERSION@/${VER}/g" \
      < "${MAC_TOOL_DIR}/${fname}.plist.in" > ${fname}.plist.$$
done

echo "Making IMP installer package..."
mkdir imp-${VER}-package || exit 1
/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker \
       -build -i Info.plist.$$ \
       -d Description.plist.$$ -ds \
       -p "imp-${VER}-package/IMP ${VER} ${TARGET_OSX_VER}.pkg" -f ${DESTDIR} \
       || exit 1
cp ${MAC_TOOL_DIR}/README.txt imp-${VER}-package || exit 1
rm -f Info.plist.$$ Description.plist.$$

echo "Making disk image (.dmg)..."
hdiutil create -fs HFS+ -volname "IMP ${VER} for OS X ${TARGET_OSX_VER}" \
               -srcfolder imp-${VER}-package IMP-${VER}-${TARGET_OSX_VER}.dmg \
        || exit 1
hdiutil internet-enable -yes IMP-${VER}-${TARGET_OSX_VER}.dmg || exit 1

rm -rf imp-${VER}-package
echo "Disk image generated as /tmp/IMP-${VER}-${TARGET_OSX_VER}.dmg"
