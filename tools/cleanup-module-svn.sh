#!/bin/sh
echo "Module is " $1
tmpfile="/tmp/props"
if test -d modules/$1/include; then
  svn propset svn:ignore config.h modules/$1/include
  svn propset svn:ignore version_info.h modules/$1/include/internal
fi
if test -d modules/$1/src; then
  echo "*.os
*.obj
version_info.cpp
link_0.cpp
link_1.cpp" > $tmpfile
  svn propset svn:ignore --file $tmpfile  modules/$1/src/internal
  rm -rf $tmpfile
  echo "*.os
*.obj" > $tmpfile
  svn propset svn:ignore --file $tmpfile  modules/$1/src/
  rm -rf $tmpfile
fi
echo "IMP.$1.py
$1_config.i
$1_wrap.cc
$1_wrap.h
*.os
*.obj" > $tmpfile
svn propset svn:ignore --file $tmpfile modules/$1/pyext
rm -rf $tmpfile
echo "*.pyc
test.passed" > $tmpfile
svn propset svn:ignore --file $tmpfile modules/$1/test
rm -rf $tmpfile
for dir in modules/$1/test/*; do
  if test -d "$dir" -a "$dir" != "modules/$1/test/input"; then
    svn propset svn:ignore "*.pyc" "$dir"
  fi
done
