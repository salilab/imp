#!/bin/sh

# Make a Win32 installer

# First run the following in the binary directory to install files:
# cmake <source_dir> -DCMAKE_INSTALL_PYTHONDIR=/pylib/2.7 \
#       -DSWIG_PYTHON_LIBRARIES=$w32py/2.7/lib/python27.lib \
#       -DPYTHON_INCLUDE_DIRS=$w32py/2.7/include/ \
#       -DPYTHON_INCLUDE_PATH=$w32py/2.7/include/ \
#       -DPYTHON_LIBRARIES=$w32py/2.7/lib/python27.lib
# make DESTDIR=`pwd`/w32-inst install
#
# Where $w32py is the path containing Python headers and libraries.
# Repeat for all desired Python versions (2.7, 3.6, 3.7, 3.8, and 3.9 for us)
#
# Then run (still in the binary directory)
# <source_dir>/tools/w32/make-package.sh <version> <bits>
#
# where <version> is the IMP version number, e.g. 1.0
# and <bits> is 32 or 64

if [ $# -ne 2 ]; then
  echo "Usage: $0 <IMP version> <bits>"
  exit 1
fi

VER=$1
BITS=$2
ROOT=w32-inst
TOOLDIR=`dirname $0`

# Put things in more w32-like arrangement
mv ${ROOT}/usr/local/include ${ROOT}/usr/local/bin ${ROOT} || exit 1
mv ${ROOT}/usr/local/lib/cmake/IMP ${ROOT}/cmake || exit 1
rmdir ${ROOT}/usr/local/lib/cmake || exit 1
mv ${ROOT}/usr/local/lib/* ${ROOT}/bin || exit 1
rmdir ${ROOT}/usr/local/lib || exit 1

mv ${ROOT}/usr/local/share/IMP ${ROOT}/data || exit 1
mv ${ROOT}/usr/local/build_info/* ${ROOT}/data/build_info || exit 1
rmdir ${ROOT}/usr/local/build_info || exit 1
mv ${ROOT}/usr/local/share/doc/IMP/examples ${ROOT} || exit 1

rm -rf ${ROOT}/usr/local/share/doc/IMP/html
rmdir ${ROOT}/usr/local/share/doc/IMP || exit 1
rmdir ${ROOT}/usr/local/share/doc || exit 1
rmdir ${ROOT}/usr/local/share || exit 1
rmdir ${ROOT}/usr/local || exit 1
rmdir ${ROOT}/usr || exit 1

# Add Windows-specific README
cp ${TOOLDIR}/pkg-README.txt ${ROOT}/README.txt || exit 1

# Move pure Python code to Windows location
mkdir ${ROOT}/python || exit 1
mkdir ${ROOT}/python/ihm || exit 1
mv ${ROOT}/pylib/2.7/*.py ${ROOT}/pylib/2.7/IMP ${ROOT}/python || exit 1
mv ${ROOT}/pylib/2.7/ihm/*.py ${ROOT}/python/ihm || exit 1
rm -rf ${ROOT}/pylib/*/*.py ${ROOT}/pylib/*/ihm/*.py ${ROOT}/pylib/*/IMP || exit 1

# Patch IMP/__init__.py, ihm/__init__.py, and RMF.py so they can find Python
# version-specific extensions and the IMP/RMF DLLs
${TOOLDIR}/add_search_path.py ${ROOT}/python/IMP/__init__.py ${ROOT}/python/RMF.py ${ROOT}/python/ihm/__init__.py || exit 1

# If there are any Python applications that don't have a file extension,
# add .py extension and drop in wrapper so users can run them without an
# extension (see comments in pyapp_wrapper.c)
for app in ${ROOT}/bin/*; do
  if ! echo $app | egrep -q '\.[a-zA-Z]+$'; then
    if file -b $app | grep -iq python; then
      mv $app ${app}.py || exit 1
      cp ${TOOLDIR}/pyapp_wrapper.exe ${app}.exe || exit 1
    else
      echo "Application $app has no file extension but"
      echo "does not appear to be Python"
      exit 1
    fi
  fi
done

# Make Python version-specific directories for extensions (.pyd)
PYVERS="2.7 3.6 3.7 3.8 3.9"
for PYVER in ${PYVERS}; do
  mkdir ${ROOT}/python/python${PYVER} || exit 1
  mkdir ${ROOT}/python/python${PYVER}/_ihm_pyd || exit 1
  echo "pass" > ${ROOT}/python/python${PYVER}/_ihm_pyd/__init__.py || exit 1
  mv ${ROOT}/pylib/${PYVER}/*.pyd ${ROOT}/python/python${PYVER} || exit 1
  mv ${ROOT}/pylib/${PYVER}/ihm/*.pyd ${ROOT}/python/python${PYVER}/_ihm_pyd || exit 1
  rmdir ${ROOT}/pylib/${PYVER}/ihm || exit 1
  rmdir ${ROOT}/pylib/${PYVER} || exit 1
done
rmdir ${ROOT}/pylib || exit 1

# Patch ihm to find _format.pyd
perl -pi -e 's/from \. import _format/from _ihm_pyd import _format/' \
     ${ROOT}/python/ihm/format.py || exit 1

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

if [ "${BITS}" = "32" ]; then
  PYVERS="27 36 37 38 39"
  MAKENSIS="makensis"
  # Add redist MSVC runtime DLLs
  DLLSRC=/usr/lib/w32comp/windows/system
  cp ${DLLSRC}/msvc*100.dll ${ROOT}/bin || exit 1
  # Add other DLL dependencies
  cp ${DLLSRC}/hdf5.dll ${DLLSRC}/libgsl.dll ${DLLSRC}/libgslcblas.dll \
     ${DLLSRC}/boost_filesystem-vc100-mt-1_53.dll \
     ${DLLSRC}/boost_program_options-vc100-mt-1_53.dll \
     ${DLLSRC}/boost_system-vc100-mt-1_53.dll \
     ${DLLSRC}/boost_date_time-vc100-mt-1_53.dll \
     ${DLLSRC}/boost_graph-vc100-mt-1_53.dll \
     ${DLLSRC}/boost_regex-vc100-mt-1_53.dll \
     ${DLLSRC}/boost_thread-vc100-mt-1_53.dll \
     ${DLLSRC}/boost_random-vc100-mt-1_53.dll \
     ${DLLSRC}/boost_iostreams-vc100-mt-1_53.dll \
     ${DLLSRC}/boost_zlib-vc100-mt-1_53.dll \
     ${DLLSRC}/CGAL-vc100-mt-4.1.dll \
     ${DLLSRC}/libgmp-10.dll \
     ${DLLSRC}/libmpfr-4.dll \
     ${DLLSRC}/libfftw3-3.dll \
     ${DLLSRC}/libTAU1.dll \
     ${DLLSRC}/zlib1.dll \
     ${DLLSRC}/opencv_core220.dll ${DLLSRC}/opencv_highgui220.dll \
     ${DLLSRC}/opencv_ffmpeg220.dll \
     ${DLLSRC}/opencv_imgproc220.dll ${ROOT}/bin || exit 1
else
  PYVERS="27 36 37 38 39"
  MAKENSIS="makensis -DIMP_64BIT"
  # Add redist MSVC runtime DLLs
  DLLSRC=/usr/lib/w64comp/windows/system32
  cp ${DLLSRC}/msvc*110.dll ${ROOT}/bin || exit 1
  # Add other DLL dependencies
  cp ${DLLSRC}/hdf5.dll ${DLLSRC}/libgsl.dll ${DLLSRC}/libgslcblas.dll \
     ${DLLSRC}/boost_filesystem-vc110-mt-1_55.dll \
     ${DLLSRC}/boost_program_options-vc110-mt-1_55.dll \
     ${DLLSRC}/boost_system-vc110-mt-1_55.dll \
     ${DLLSRC}/boost_date_time-vc110-mt-1_55.dll \
     ${DLLSRC}/boost_graph-vc110-mt-1_55.dll \
     ${DLLSRC}/boost_regex-vc110-mt-1_55.dll \
     ${DLLSRC}/boost_thread-vc110-mt-1_55.dll \
     ${DLLSRC}/boost_random-vc110-mt-1_55.dll \
     ${DLLSRC}/boost_iostreams-vc110-mt-1_55.dll \
     ${DLLSRC}/boost_zlib-vc110-mt-1_55.dll \
     ${DLLSRC}/boost_chrono-vc110-mt-1_55.dll \
     ${DLLSRC}/CGAL-vc110-mt-4.4.dll \
     ${DLLSRC}/libgmp-10.dll \
     ${DLLSRC}/libmpfr-4.dll \
     ${DLLSRC}/libfftw3-3.dll \
     ${DLLSRC}/libTAU1.dll \
     ${DLLSRC}/zlib1.dll \
     ${DLLSRC}/opencv_core248.dll ${DLLSRC}/opencv_highgui248.dll \
     ${DLLSRC}/opencv_imgproc248.dll ${ROOT}/bin || exit 1
fi


# Check all installed binaries for DLL dependencies, to make sure we
# didn't miss any
dumpbin /DEPENDENTS ${ROOT}/bin/*.exe ${ROOT}/bin/*.dll ${ROOT}/python/*/*.pyd \
        | grep -i '\.dll' | grep -v 'Dump of file' \
        | tr '[:upper:]' '[:lower:]' | sort -u > w32.deps
(cd ${ROOT}/bin && ls *.dll) | tr '[:upper:]' '[:lower:]' > w32.dlls

# Add standard Windows DLLs
echo "kernel32.dll" >> w32.dlls
echo "advapi32.dll" >> w32.dlls
echo "avicap32.dll" >> w32.dlls
echo "avifil32.dll" >> w32.dlls
echo "comctl32.dll" >> w32.dlls
echo "gdi32.dll" >> w32.dlls
echo "shell32.dll" >> w32.dlls
echo "msvcrt.dll" >> w32.dlls
echo "msvfw32.dll" >> w32.dlls
echo "ole32.dll" >> w32.dlls
echo "oleaut32.dll" >> w32.dlls
echo "user32.dll" >> w32.dlls
echo "wsock32.dll" >> w32.dlls
echo "ws2_32.dll" >> w32.dlls

# People that want to run MPI-enabled binaries will need their own copy
# of MS-MPI - we don't bundle it.
echo "msmpi.dll" >> w32.dlls

# Add DLLs of our prerequisites (Python)
for PYVER in ${PYVERS}; do
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
${MAKENSIS} -DVERSION=${VER} -NOCD ${TOOLDIR}/w32-install.nsi || exit 1
