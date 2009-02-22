#!/bin/sh
echo "Module is " $1
svn propset svn:ignore config.h modules/$1/include
svn propset svn:ignore version_info.h modules/$1/include/internal
svn propset svn:ignore version_info.cpp modules/$1/src/internal
echo "IMP.$1.py\n$1_config.i\n$1_wrap.cc\n$1_wrap.h\n*.os\n*.obj" > /tmp/props
svn propset svn:ignore --file /tmp/props modules/$1/pyext