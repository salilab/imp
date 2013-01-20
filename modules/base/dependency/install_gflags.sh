#!/bin/sh

cd %(workdir)s

if test \! -e %(builddir)s/src/gflags/Makefile; then
  %(srcdir)s/configure --prefix=%(builddir)s > /dev/null
fi
make -j %(jobs)s > /dev/null
make install > /dev/null