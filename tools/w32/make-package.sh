#!/bin/sh

# Make a Win32 installer

# First run the following in the binary directory to install files:
# cmake <source_dir> -DCMAKE_INSTALL_PYTHONDIR=/pylib/3.9 \
#       -DSWIG_PYTHON_LIBRARIES=$w32py/3.9/lib/python39.lib \
#       -DPYTHON_INCLUDE_DIRS=$w32py/3.9/include/ \
#       -DPYTHON_INCLUDE_PATH=$w32py/3.9/include/ \
#       -DPYTHON_LIBRARIES=$w32py/3.9/lib/python39.lib
# make DESTDIR=`pwd`/w32-inst install
#
# Where $w32py is the path containing Python headers and libraries.
# Repeat for all desired Python versions (3.7, 3.8, 3.9, 3.10, 3.11,
# and 3.12 for us)
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

# Drop Python 2
rm -rf ${ROOT}/pylib/2.7/

# Remove .pyc files
find ${ROOT} -name __pycache__ -exec rm -rf \{\} \; 2>/dev/null

# Put pure Python files in correct location
mv ${ROOT}/pylib/3.9/*.py ${ROOT}/pylib/3.9/IMP ${ROOT}/python || exit 1
mv ${ROOT}/pylib/3.9/ihm/*.py ${ROOT}/python/ihm || exit 1

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
PYVERS="3.7 3.8 3.9 3.10 3.11 3.12"
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

# Remove scratch module and example application/dependency
# (if installed)
rm -rf ${ROOT}/bin/imp_example_app.exe \
       ${ROOT}/bin/libimp_scratch.* \
       ${ROOT}/bin/libexample* \
       ${ROOT}/include/example* \
       ${ROOT}/python/IMP/scratch ${ROOT}/python/*/_IMP_scratch.pyd

# Remove any .svn directories
rm -rf `find ${ROOT} -name .svn`

PYVERS="36 37 38 39 310 311 312"
if [ "${BITS}" = "32" ]; then
  MAKENSIS="makensis"
  DLLSRC=/usr/lib/w32comp/windows/system
else
  MAKENSIS="makensis -DIMP_64BIT"
  DLLSRC=/usr/lib/w64comp/windows/system32
fi

# Add redist MSVC runtime DLLs
cp ${DLLSRC}/msvcp140.dll ${DLLSRC}/concrt140.dll ${DLLSRC}/vcruntime140.dll \
   ${ROOT}/bin || exit 1

# Add other DLL dependencies
cp ${DLLSRC}/hdf5.dll ${DLLSRC}/libgsl.dll ${DLLSRC}/libgslcblas.dll \
   ${DLLSRC}/boost_filesystem-vc141-mt-x${BITS}-1_83.dll \
   ${DLLSRC}/boost_program_options-vc141-mt-x${BITS}-1_83.dll \
   ${DLLSRC}/boost_system-vc141-mt-x${BITS}-1_83.dll \
   ${DLLSRC}/boost_date_time-vc141-mt-x${BITS}-1_83.dll \
   ${DLLSRC}/boost_graph-vc141-mt-x${BITS}-1_83.dll \
   ${DLLSRC}/boost_regex-vc141-mt-x${BITS}-1_83.dll \
   ${DLLSRC}/boost_thread-vc141-mt-x${BITS}-1_83.dll \
   ${DLLSRC}/boost_random-vc141-mt-x${BITS}-1_83.dll \
   ${DLLSRC}/boost_iostreams-vc141-mt-x${BITS}-1_83.dll \
   ${DLLSRC}/boost_zlib-vc141-mt-x${BITS}-1_83.dll \
   ${DLLSRC}/libgmp-10.dll \
   ${DLLSRC}/libmpfr-4.dll \
   ${DLLSRC}/libfftw3-3.dll \
   ${DLLSRC}/libTAU1.dll \
   ${DLLSRC}/zlib1.dll \
   ${DLLSRC}/opencv_core455.dll ${DLLSRC}/opencv_highgui455.dll \
   ${DLLSRC}/opencv_imgcodecs455.dll ${DLLSRC}/opencv_videoio455.dll \
   ${DLLSRC}/opencv_imgproc455.dll ${ROOT}/bin || exit 1

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
echo "mf.dll" >> w32.dlls
echo "mfplat.dll" >> w32.dlls
echo "mfreadwrite.dll" >> w32.dlls
echo "shlwapi.dll" >> w32.dlls

# Also exclude Universal C runtime and Windows API sets, which
# should be present on any up to date Windows 7 system (via KB2999226),
# or by default in Windows 10.
echo "ucrtbase.dll" >> w32.dlls

for crt in convert environment filesystem heap locale math multibyte \
           runtime stdio string time utility; do
  echo "api-ms-win-crt-${crt}-l1-1-0.dll" >> w32.dlls
done
for core in console-l1-1-0 datetime-l1-1-0 debug-l1-1-0 errorhandling-l1-1-0 \
            file-l1-1-0 file-l1-2-0 file-l2-1-0 handle-l1-1-0 heap-l1-1-0 \
            interlocked-l1-1-0 libraryloader-l1-1-0 localization-l1-2-0 \
            memory-l1-1-0 namedpipe-l1-1-0 processenvironment-l1-1-0 \
            processthreads-l1-1-0 processthreads-l1-1-1 profile-l1-1-0 \
            rtlsupport-l1-1-0 string-l1-1-0 synch-l1-1-0 synch-l1-2-0 \
            sysinfo-l1-1-0 timezone-l1-1-0 util-l1-1-0; do
  echo "api-ms-win-core-${core}.dll" >> w32.dlls
done

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
tac w32files.tmp | sed -e 's/File "w32-inst\\/Delete "$INSTDIR\\/' -e 's#^SetOutPath "\(.*\)"#RMDir /r "\1\\__pycache__"\nRMDir "\1"#' > w32files.uninstall || exit 1
${MAKENSIS} -DVERSION=${VER} -NOCD ${TOOLDIR}/w32-install.nsi || exit 1
