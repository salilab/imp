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
IMPSRCTGZ=${MODINSTALL}/build/sources/imp.tar.gz

rm -rf ${TMPDIR}
mkdir ${TMPDIR}
cd ${TMPDIR}

# Get top-most revision number (must be a nicer way of doing this?)
rev=$(svn log -q --limit 1 ${IMPSVNDIR} |grep '^r' | cut -f 1 -d' ')

# Get IMP code from SVN
svn export -q -${rev} ${IMPSVNDIR} imp

# Put version number, date and revision into relevant files
DATE=`date +'%Y/%m/%d'`
(cd imp/kernel/doc/internal && sed -e "s#^PROJECT_NUMBER.*#PROJECT_NUMBER = ${VER}, ${DATE}, ${rev}#" < doxygen.conf > .dox && mv .dox doxygen.conf)
(cd imp/kernel/doc && sed -e "s#</title>#</title><subtitle>Documentation for nightly build ${VER}, ${DATE}, ${rev}</subtitle>#" < manual.xml > .man && mv .man manual.xml)

# Write out a version file
verfile="${MODINSTALL}/build/imp-version"
echo "${rev}" > $verfile

# Write out a tarball:
tar -czf ${IMPSRCTGZ} imp

# Cleanup
cd /
rm -rf ${TMPDIR}
