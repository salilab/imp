#!/bin/sh

# Make a Win32 installer

# First run the following to install files:
# scons -j5 destdir=w32-inst wine=true modeller=/synth1/home/ben/modeller \
#           install w32-inst/usr/share/doc/imp/examples

ROOT=w32-inst

# Put things in more w32-like arrangement
mv ${ROOT}/usr/include ${ROOT}/usr/lib ${ROOT}
mv ${ROOT}/usr/bin/* ${ROOT}/lib
rmdir ${ROOT}/usr/bin

mv ${ROOT}/usr/share/imp ${ROOT}/data
mkdir ${ROOT}/doc
mv ${ROOT}/usr/share/doc/imp/examples ${ROOT}/doc

rmdir ${ROOT}/usr/share/doc/imp
rmdir ${ROOT}/usr/share/doc
rmdir ${ROOT}/usr/share
rmdir ${ROOT}/usr

mv ${ROOT}/lib/python2.6/site-packages/* ${ROOT}/lib
rmdir ${ROOT}/lib/python2.6/site-packages
rmdir ${ROOT}/lib/python2.6

# Add redist MSVC runtime DLLs
cp /usr/lib/w32comp/windows/system/msvc*71.dll ${ROOT}/lib

tools/w32/gen-w32instlist w32-inst > w32files.tmp
sed -e '/\.pyc"$/d' < w32files.tmp > w32files.install
tac w32files.tmp | sed -e 's/File "w32-inst\\/Delete "$INSTDIR\\/' -e 's/^SetOutPath/RMDir/' > w32files.uninstall
makensis -NOCD tools/w32/w32-install.nsi 
