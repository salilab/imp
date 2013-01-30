#!/bin/sh

#-DCMAKE_INSTALL_PREFIX=%(builddir)s \
#-DCMAKE_INSTALL_PYTHONDIR=%(builddir)s/lib\
#-DCMAKE_INSTALL_LIBDIR=%(builddir)s/lib \
#-DCMAKE_INSTALL_SWIGDIR=%(builddir)s/swig \

cd %(workdir)s
CMAKE_ARGS=""
BUILDCMD="make"
if [ -e "`command -v ninja`" ]; then
    if cmake --help | grep Ninja >/dev/null ; then
        CMAKE_ARGS="-G Ninja"
        BUILDCMD="ninja"
        echo "Using Ninja"
        if [ -e Makefile ]; then
          rm -rf CM* M*
        fi
    fi
fi

# avoid rewriting the config files
if [ -e CMakeFiles ]; then
echo "Skipping cmake."
else
%(cmake)s %(srcdir)s \
${CMAKE_ARGS} \
-DCMAKE_BUILD_TYPE="%(buildtype)s" \
-DCMAKE_LIBRARY_PATH="%(libpath)s" \
-DCMAKE_INCLUDE_PATH="%(includepath)s" \
-DCMAKE_CXX_FLAGS="%(cxxflags)s" \
-DCMAKE_SHARED_LINKER_FLAGS="%(linkflags)s"
fi

${BUILDCMD} -j %(jobs)s

mkdir -p %(builddir)s/lib
if [ -e %(builddir)s/src/RMF/_RMF.pyd ]; then
  # Windows-style library names
  ln -sfF %(builddir)s/src/RMF/RMF.* %(builddir)s/src/RMF/_RMF.* %(builddir)s/lib/
  ln -sfF %(builddir)s/src/RMF/RMF_HDF5.* %(builddir)s/src/RMF/_RMF_HDF5.* %(builddir)s/lib/
  ln -sfF %(builddir)s/src/RMF/RMF.lib %(builddir)s/lib/libRMF.lib
else
  # Unix-style library names
ln -sfF %(builddir)s/src/RMF/libRMF* %(builddir)s/src/RMF/_RMF.so %(builddir)s/src/RMF/RMF.py %(builddir)s/lib/
ln -sfF %(builddir)s/src/RMF/libRMF* %(builddir)s/src/RMF/_RMF_HDF5.so %(builddir)s/src/RMF/RMF_HDF5.py %(builddir)s/lib/
fi

mkdir -p %(builddir)s/include/RMF
ln -sfF %(srcdir)s/include/RMF/* %(builddir)s/include/RMF/
ln -sfF %(builddir)s/src/RMF/include/RMF/* %(builddir)s/include/RMF/
ln -sfF %(builddir)s/src/RMF/include/RMF.h %(builddir)s/include/
mkdir -p %(builddir)s/swig
ln -sfF %(srcdir)s/swig/*.i %(builddir)s/swig

if [ -e %(builddir)s/src/RMF/AvroCpp ]; then
  ln -sfF %(builddir)s/src/RMF/AvroCpp/libavro* %(builddir)s/lib/
  ln -sfF %(srcdir)s/AvroCpp/avro %(builddir)s/include
  mkdir -p %(builddir)s/bin
  ln -sfF %(builddir)s/src/RMF/AvroCpp/avrogencpp %(builddir)s/bin
fi
