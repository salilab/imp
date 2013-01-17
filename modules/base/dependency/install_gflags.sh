#!/bin/sh

cd %(workdir)s

%(srcdir)s/configure --prefix=%(builddir)s
make -j %(jobs)s
make install