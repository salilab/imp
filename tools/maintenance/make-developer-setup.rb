#!/usr/bin/ruby -v

checkout=%(mkdir -p imp
cd imp
svn co https://svn.salilab.org/imp/nightly/last_rw svn > /dev/null
echo "repository='../svn/trunk'" >> global_config.py)
build=%(cd imp
dir=`pwd`
mkdir debug
cd debug
ln -s ../svn/trunk/SConstruct
ln -s ../svn/trunk/scons_tools
ln -s ../svn/trunk/tools/gdbinit .gdbinit
echo "execfile('$dir/global_config.py')" > config.py
echo "build='debug'" >> config.py
cd ..
mkdir release
cd release
ln -s ../svn/trunk/SConstruct
ln -s ../svn/trunk/scons_tools
ln -s ../svn/trunk/tools/gdbinit .gdbinit
echo "execfile('$dir/global_config.py')" > config.py
echo "build='release'" >> config.py
cd ..
mkdir fast
cd fast
ln -s ../svn/trunk/SConstruct
ln -s ../svn/trunk/scons_tools
ln -s ../svn/trunk/tools/gdbinit .gdbinit
echo "execfile('$dir/global_config.py')" > config.py
echo "build='fast'" >> config.py)
scripts=%(
cd imp
echo "#\!/bin/csh" > update-to-nightly
echo "svn update trunk" >> update-to-nightly
chmod a+x update-to-nightly
echo "#\!/bin/csh" > update-to-head
echo "svn update trunk" >> update-to-head
chmod a+x update-to-head
echo "#\!/bin/csh" > show-changes
echo 'svn cat http://svn.salilab.org/imp/trunk/doc/history.dox | diff $0:h/svn/trunk/doc/history.dox - -U 0 | grep -v "^+++" | grep -v "^---" | grep -v "^@@" | cut -c2-' >> show-changes
chmod a+x show-changes)
print "checking out imp from svn..."
system checkout
print "done\n"
print "setting up build directories..."
system build
print "done\n"
print "creating scripts..."
system scripts
print "done\n"
