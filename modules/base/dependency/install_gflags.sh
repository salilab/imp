#!/bin/sh

cd %(workdir)s

if test \! -e %(builddir)s/src/gflags/Makefile; then
  %(srcdir)s/configure --prefix=%(builddir)s
fi
make -j %(jobs)s
make install