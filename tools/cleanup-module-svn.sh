#!/bin/sh
echo "Module is " $1
tmpfile="/tmp/props"
svn propset svn:ignore config.h modules/$1/include
svn propset svn:ignore version_info.h modules/$1/include/internal
echo "version_info.cpp
link_0.cpp
link_1.cpp" > $tmpfile
svn propset svn:ignore --file $tmpfile  modules/$1/src/internal
rm -rf $tmpfile
echo "*.os
*.obj" > $tmpfile
svn propset svn:ignore --file $tmpfile  modules/$1/src/
rm -rf $tmpfile
echo "IMP.$1.py
$1_config.i
$1_wrap.cc
$1_wrap.h
*.os
*.obj" > $tmpfile
svn propset svn:ignore --file $tmpfile modules/$1/pyext
svn propset svn:ignore "test.passed" modules/$1/test
rm -rf $tmpfile