#!/bin/sh

# Make a Win32 installer

# First run the following to install files:
# scons -j3 destdir=w32-inst wine=true install
#
# Then run (still in the top-level IMP directory)
# tools/w32/make-package.sh <version>
#
# where <version> is the IMP version number, e.g. 1.0

if [ $# -ne 1 ]; then
  echo "Usage: $0 <IMP version>"
  exit 1
fi

VER=$1
ROOT=w32-inst

# Put things in more w32-like arrangement
mv ${ROOT}/usr/include ${ROOT}/usr/lib ${ROOT} || exit 1
mv ${ROOT}/usr/bin/* ${ROOT}/lib || exit 1
rmdir ${ROOT}/usr/bin || exit 1

mv ${ROOT}/usr/share/imp ${ROOT}/data || exit 1
mv ${ROOT}/usr/share/doc/imp/examples ${ROOT} || exit 1

rmdir ${ROOT}/usr/share/doc/imp || exit 1
rmdir ${ROOT}/usr/share/doc || exit 1
rmdir ${ROOT}/usr/share || exit 1
rmdir ${ROOT}/usr || exit 1

# Note that Python extensions are installed in the 2.4 location, since we
# cross-compile (and Python in CentOS 5 is 2.4)
mv ${ROOT}/lib/python2.4/site-packages/* ${ROOT}/lib || exit 1
rmdir ${ROOT}/lib/python2.4/site-packages || exit 1
rmdir ${ROOT}/lib/python2.4 || exit 1

# Add redist MSVC runtime DLLs
DLLSRC=/usr/lib/w32comp/windows/system
cp ${DLLSRC}/msvc*100.dll ${ROOT}/lib || exit 1

# Add other DLL dependencies
cp ${DLLSRC}/hdf5dll.dll ${DLLSRC}/libgsl.dll ${ROOT}/lib || exit 1
cp ${DLLSRC}/boost_filesystem-vc100-mt-1_44.dll ${ROOT}/lib || exit 1
cp ${DLLSRC}/boost_system-vc100-mt-1_44.dll ${ROOT}/lib || exit 1

# todo: check all installed DLLs for dependencies, to make sure we didn't
# miss any

tools/w32/gen-w32instlist w32-inst > w32files.tmp || exit 1
sed -e '/\.pyc"$/d' < w32files.tmp > w32files.install || exit 1
tac w32files.tmp | sed -e 's/File "w32-inst\\/Delete "$INSTDIR\\/' -e 's/^SetOutPath/RMDir/' > w32files.uninstall || exit 1
makensis -DVERSION=${VER} -NOCD tools/w32/w32-install.nsi || exit 1
