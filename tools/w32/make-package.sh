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
cp ${DLLSRC}/hdf5dll.dll ${DLLSRC}/libgsl.dll ${DLLSRC}/libgslcblas.dll \
   ${DLLSRC}/boost_filesystem-vc100-mt-1_44.dll \
   ${DLLSRC}/boost_program_options-vc100-mt-1_44.dll \
   ${DLLSRC}/boost_system-vc100-mt-1_44.dll \
   ${DLLSRC}/libfftw3-3.dll \
   ${DLLSRC}/opencv_core220.dll ${DLLSRC}/opencv_highgui220.dll \
   ${DLLSRC}/opencv_imgproc220.dll ${ROOT}/lib || exit 1

# Check all installed binaries for DLL dependencies, to make sure we
# didn't miss any
# We should really parse the PE files properly rather than using 'strings' here!
strings ${ROOT}/lib/*.exe ${ROOT}/lib/*.pyd ${ROOT}/lib/*.dll \
        | grep -i '\.dll' | sort -u | tr '[:upper:]' '[:lower:]' > w32.deps
(cd ${ROOT}/lib && ls *.dll) | tr '[:upper:]' '[:lower:]' > w32.dlls

# Add standard Windows DLLs and those of our prerequisites (Python 2.6)
echo "kernel32.dll" >> w32.dlls
echo "advapi32.dll" >> w32.dlls
echo "python26.dll" >> w32.dlls

if grep -v -f w32.dlls w32.deps > w32.unmet_deps; then
  echo "The following non-standard libraries are linked against, and were"
  echo "not bundled:"
  echo
  cat w32.unmet_deps
  exit 1
fi

rm -f w32.dlls w32.deps w32.unmet_deps

tools/w32/gen-w32instlist w32-inst > w32files.tmp || exit 1
sed -e '/\.pyc"$/d' < w32files.tmp > w32files.install || exit 1
tac w32files.tmp | sed -e 's/File "w32-inst\\/Delete "$INSTDIR\\/' -e 's/^SetOutPath/RMDir/' > w32files.uninstall || exit 1
makensis -DVERSION=${VER} -NOCD tools/w32/w32-install.nsi || exit 1
