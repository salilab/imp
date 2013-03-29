#!/bin/sh

# Make a Win32 installer

# First run the following in the binary directory to install files:
# cmake <source_dir> DCMAKE_INSTALL_PYTHONDIR=/pylib/2.6 \
#       -DSWIG_PYTHON_LIBRARIES=$w32py/2.6/lib/python26.lib \
#       -DPYTHON_INCLUDE_DIRS=$w32py/2.6/include/ \
#       -DPYTHON_INCLUDE_PATH=$w32py/2.6/include/ \
#       -DPYTHON_LIBRARIES=$w32py/2.6/lib/python26.lib
# make DESTDIR=`pwd`/w32-inst install
#
# Where $w32py is the path containing Python headers and libraries.
# Repeat for all Python versions 2.4 through 2.7.
#
# Then run (still in the binary directory)
# <source_dir>/tools/w32/make-package.sh <version>
#
# where <version> is the IMP version number, e.g. 1.0

if [ $# -ne 1 ]; then
  echo "Usage: $0 <IMP version>"
  exit 1
fi

VER=$1
ROOT=w32-inst
TOOLDIR=`dirname $0`

# Put things in more w32-like arrangement
mv ${ROOT}/usr/local/include ${ROOT}/usr/local/bin ${ROOT} || exit 1
mv ${ROOT}/usr/local/lib/* ${ROOT}/bin || exit 1
rmdir ${ROOT}/usr/local/lib || exit 1

mv ${ROOT}/usr/local/share/IMP ${ROOT}/data || exit 1
mv ${ROOT}/usr/local/share/doc/IMP/examples ${ROOT} || exit 1

rm -rf ${ROOT}/usr/local/share/doc/IMP/html
rmdir ${ROOT}/usr/local/share/doc/IMP || exit 1
rmdir ${ROOT}/usr/local/share/doc || exit 1
rmdir ${ROOT}/usr/local/share || exit 1
rmdir ${ROOT}/usr/local || exit 1
rmdir ${ROOT}/usr || exit 1

# Add Windows-specific README
cp ${TOOLDIR}/README.txt ${ROOT} || exit 1

# Move pure Python code to Windows location
mkdir ${ROOT}/python || exit 1
mv ${ROOT}/pylib/2.6/*.py ${ROOT}/pylib/2.6/IMP ${ROOT}/python || exit 1
rm -rf ${ROOT}/pylib/*/*.py ${ROOT}/pylib/*/IMP || exit 1

# Patch IMP/__init__.py so it can find Python version-specific extensions
# and the IMP DLLs
patch -d ${ROOT}/python/IMP -p1 < ${TOOLDIR}/python-search-path.patch || exit 1

# Make Python version-specific directories for extensions (.pyd)
for PYVER in 2.4 2.5 2.6 2.7; do
  mkdir ${ROOT}/python/python${PYVER} || exit 1
  mv ${ROOT}/pylib/${PYVER}/*.pyd ${ROOT}/python/python${PYVER} || exit 1
  rmdir ${ROOT}/pylib/${PYVER} || exit 1
done
rmdir ${ROOT}/pylib || exit 1

# Remove scratch module and example application/system/dependency
# (if installed)
rm -rf ${ROOT}/bin/example \
       ${ROOT}/bin/libimp_example_system* \
       ${ROOT}/bin/libimp_scratch.* \
       ${ROOT}/bin/libexample* \
       ${ROOT}/include/example* \
       ${ROOT}/python/IMP/scratch ${ROOT}/python/*/_IMP_scratch.pyd \
       ${ROOT}/python/IMP/example_system_local \
       ${ROOT}/python/*/_IMP_example_system_local.pyd

# Remove any .svn directories
rm -rf `find ${ROOT} -name .svn`

# Add redist MSVC runtime DLLs
DLLSRC=/usr/lib/w32comp/windows/system
cp ${DLLSRC}/msvc*100.dll ${ROOT}/bin || exit 1

# Add other DLL dependencies
cp ${DLLSRC}/hdf5dll.dll ${DLLSRC}/libgsl.dll ${DLLSRC}/libgslcblas.dll \
   ${DLLSRC}/boost_filesystem-vc100-mt-1_44.dll \
   ${DLLSRC}/boost_program_options-vc100-mt-1_44.dll \
   ${DLLSRC}/boost_system-vc100-mt-1_44.dll \
   ${DLLSRC}/boost_date_time-vc100-mt-1_44.dll \
   ${DLLSRC}/boost_graph-vc100-mt-1_44.dll \
   ${DLLSRC}/boost_regex-vc100-mt-1_44.dll \
   ${DLLSRC}/boost_thread-vc100-mt-1_44.dll \
   ${DLLSRC}/boost_random-vc100-mt-1_44.dll \
   ${DLLSRC}/libfftw3-3.dll \
   ${DLLSRC}/avrocpp.dll \
   ${DLLSRC}/libTAU1.dll \
   ${DLLSRC}/opencv_core220.dll ${DLLSRC}/opencv_highgui220.dll \
   ${DLLSRC}/opencv_ffmpeg220.dll \
   ${DLLSRC}/opencv_imgproc220.dll ${ROOT}/bin || exit 1

# Check all installed binaries for DLL dependencies, to make sure we
# didn't miss any
# We should really parse the PE files properly rather than using 'strings' here!
strings ${ROOT}/bin/*.exe ${ROOT}/bin/*.dll ${ROOT}/python/*/*.pyd \
        | grep -i '\.dll' | sort -u | tr '[:upper:]' '[:lower:]' > w32.deps
(cd ${ROOT}/bin && ls *.dll) | tr '[:upper:]' '[:lower:]' > w32.dlls

# Add standard Windows DLLs
echo "kernel32.dll" >> w32.dlls
echo "advapi32.dll" >> w32.dlls
echo "avicap32.dll" >> w32.dlls
echo "avifil32.dll" >> w32.dlls
echo "comctl32.dll" >> w32.dlls
echo "gdi32.dll" >> w32.dlls
echo "msvcrt.dll" >> w32.dlls
echo "msvfw32.dll" >> w32.dlls
echo "ole32.dll" >> w32.dlls
echo "user32.dll" >> w32.dlls
echo "wsock32.dll" >> w32.dlls
echo "ws2_32.dll" >> w32.dlls

# Add DLLs of our prerequisites (Python)
for PYVER in 24 25 26 27; do
  echo "python${PYVER}.dll" >> w32.dlls
done

if grep -v -f w32.dlls w32.deps > w32.unmet_deps; then
  echo "The following non-standard libraries are linked against, and were"
  echo "not bundled:"
  echo
  cat w32.unmet_deps
  echo
  find .
  exit 1
fi

rm -f w32.dlls w32.deps w32.unmet_deps

${TOOLDIR}/gen-w32instlist w32-inst > w32files.tmp || exit 1
sed -e '/\.pyc"$/d' < w32files.tmp > w32files.install || exit 1
tac w32files.tmp | sed -e 's/File "w32-inst\\/Delete "$INSTDIR\\/' -e 's/^SetOutPath/RMDir/' > w32files.uninstall || exit 1
makensis -DVERSION=${VER} -NOCD ${TOOLDIR}/w32-install.nsi || exit 1
