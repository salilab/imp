#!/bin/sh


cd %(workdir)s
%(cmake)s %(srcdir)s \
-DCMAKE_INSTALL_PREFIX="%(builddir)s" \
-DCMAKE_BUILD_TYPE="%(buildtype)s" \
-DCMAKE_LIBRARY_PATH="%(libpath)s" \
-DCMAKE_INCLUDE_PATH="%(includepath)s" \
-DCMAKE_CXX_FLAGS="%(cxxflags)s" \
-DCMAKE_SHARED_LINKER_FLAGS="%(linkflags)s"
make -j %(jobs)s

make install
