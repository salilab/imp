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

TMPDIR=/var/tmp/imp-build-$$
IMPTOP=/salilab/diva1/home/imp

rm -rf ${TMPDIR}
mkdir ${TMPDIR}
cd ${TMPDIR}

# Get top-most revision number (must be a nicer way of doing this?)
rev=$(svn log -q --limit 1 ${IMPSVNDIR} |grep '^r' | cut -f 1 -d' ')

# Get date and revision-specific install directories
IMPINSTALL=${IMPTOP}/`date -d '8 hours' "+%Y%m%d"`-${rev}
IMPVERSION="SVN.${rev}"
IMPSRCTGZ=${IMPINSTALL}/build/sources/imp-${IMPVERSION}.tar.gz
RMFSRCTGZ=${IMPINSTALL}/build/sources/rmf.tar.gz
rm -rf ${IMPINSTALL}
mkdir -p ${IMPINSTALL}/build/sources ${IMPINSTALL}/build/logs

# Make link so build system can find the install location
rm -f ${IMPTOP}/.SVN-new
ln -s ${IMPINSTALL} ${IMPTOP}/.SVN-new

# Get IMP code from SVN
svn export -q -${rev} ${IMPSVNDIR} imp

# Put version number, date and revision into relevant files
DATE=`date +'%Y/%m/%d'`
(cd imp/doc/doxygen && sed -e "s#^PROJECT_NUMBER.*#PROJECT_NUMBER = ${VER}, ${DATE}, ${rev}#" < Doxyfile.in > .dox && mv .dox Doxyfile.in)
echo "SVN.${rev}" > imp/VERSION

# Write out a version file
verfile="${IMPINSTALL}/build/imp-version"
mkdir -p "${IMPINSTALL}/build"
echo "${rev}" > $verfile

# Write out list of all components
compfile="${IMPINSTALL}/build/imp-components"
python <<END
import sys
sys.path.insert(0, 'imp/tools/build')
import tools

f = open('$compfile', 'w')
for m in tools.compute_sorted_order('imp', ''):
    print >> f, "module\t" + m
for a in tools.get_applications('imp'):
    print >> f, "application\t" + a[0]
for s in tools.get_biological_systems('imp'):
    print >> f, "system\t" + s[0]
END

# Write out a tarball:
mv imp imp-${IMPVERSION} && tar -czf ${IMPSRCTGZ} imp-${IMPVERSION}

# Make an RMF tarball from our git repo:
(cd /cowbell1/git && tar -czf ${RMFSRCTGZ} --exclude .git rmf)

# Cleanup
cd /
rm -rf ${TMPDIR}
