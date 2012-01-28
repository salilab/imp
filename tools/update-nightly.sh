#!/bin/sh

# Update nightly build URLs.
#
# Should be run on the SVN server (or some other system that can access SVN
# readwrite without a password) from a crontab, e.g.
#
# 5 7 * * * /cowbell1/home/ben/imp/tools/update-nightly.sh

VER=SVN
IMPSVNDIR=file:///cowbell1/svn/imp
TMPDIR=/var/tmp/imp-build-$$

rm -rf ${TMPDIR}
mkdir ${TMPDIR}
cd ${TMPDIR}

REV=`cat /salilab/diva1/home/imp/nightly/build/imp-version`
DATE=`date +'%m/%d/%Y'`
svn co -q --ignore-externals ${IMPSVNDIR}/nightly && cd nightly

svn propset -q svn:externals \
    "trunk -${REV} http://svn.salilab.org/imp/trunk" last \
&& svn propset -q svn:externals \
    "trunk -${REV} https://svn.salilab.org/imp/trunk" last_rw \
&& svn ci -q --username autobuild -m \
     "Update for nightly build ${REV}, ${DATE}."

# change version of modules to have the date
cd ${TMPDIR}
svn co file:///cowbell1/svn/imp/trunk versions
cd versions
./tools/increment-versions modules/*/SConscript biological_systems/*/SConscript  applications/*/SConscript
version="nightly module versions update on "`date`
svn commit -m "\"$version\"" modules/*/SConscript biological_systems/*/SConscript applications/*/SConscript tools/increment-versions

# Cleanup
cd /
rm -rf ${TMPDIR}
