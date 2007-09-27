#!/bin/sh

# Script to get SVN HEAD and make a .tar.gz on a shared disk, for use by
# autobuild scripts on our build machines.
#
# Should be run on the SVN server (or some other system that can access SVN
# readonly without a password) from a crontab, e.g.
#
# 10 1 * * * /cowbell1/home/ben/imp/tools/auto-build.sh

VER=SVN
BUILDUSER=ben
IMPSVNDIR=file:///cowbell1/svn/imp/trunk/

TMPDIR=/var/tmp/modeller-build-$$
MODINSTALL=/diva1/home/modeller/.${VER}-new
IMPSRCTGZ=${MODINSTALL}/build/imp.tar.gz

# Make build directory
su ${BUILDUSER} -c "mkdir -m 0700 -p ${MODINSTALL}/build"

rm -rf ${TMPDIR}
mkdir ${TMPDIR}
cd ${TMPDIR}

# Get IMP code from SVN
svn export -q ${IMPSVNDIR} imp

# Write out a tarball as the build user:
su ${BUILDUSER} -c "tar -czf ${IMPSRCTGZ} imp"

# Cleanup
cd
rm -rf ${TMPDIR}
