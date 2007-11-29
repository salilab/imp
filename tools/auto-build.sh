#!/bin/sh

# Script to get SVN HEAD and make a .tar.gz on a shared disk, for use by
# autobuild scripts on our build machines.
#
# Should be run on the SVN server (or some other system that can access SVN
# readonly without a password) from a crontab, e.g.
#
# 10 1 * * * /cowbell1/home/ben/imp/tools/auto-build.sh
#
# In this case, the build user is in the apache group, and so has readonly
# access to the repositories.

VER=SVN
IMPSVNDIR=file:///cowbell1/svn/imp/trunk/

TMPDIR=/var/tmp/modeller-build-$$
MODINSTALL=/diva1/home/modeller/.${VER}-new
IMPSRCTGZ=${MODINSTALL}/build/imp.tar.gz

# Make build directory
mkdir -m 0700 -p ${MODINSTALL}/build

rm -rf ${TMPDIR}
mkdir ${TMPDIR}
cd ${TMPDIR}

# Get IMP code from SVN
svn export -q ${IMPSVNDIR} imp

# Write out a tarball:
tar -czf ${IMPSRCTGZ} imp

# Cleanup
cd /
rm -rf ${TMPDIR}
