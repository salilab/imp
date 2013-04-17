#!/bin/sh

# Script to get git HEAD and make a .tar.gz on a shared disk, for use by
# autobuild scripts on our build machines.
#
# Should be run on from a crontab on a machine that has access to a git
# clone, e.g.
#
# 10 1 * * * /cowbell1/home/ben/imp/tools/auto-build.sh

GIT_TOP=/cowbell1/git
GIT_DIR=${GIT_TOP}/imp/.git
export GIT_DIR

VER=git

TMPDIR=/var/tmp/imp-build-$$
IMPTOP=/salilab/diva1/home/imp

rm -rf ${TMPDIR}
mkdir ${TMPDIR}
cd ${TMPDIR}

# Get top-most revision
rev=`git rev-parse HEAD`
shortrev=`git rev-parse --short HEAD`

# Get date and revision-specific install directories
SORTDATE=`date -d '8 hours' "+%Y%m%d"`
DATE=`date -d '8 hours' +'%Y/%m/%d'`
IMPINSTALL=${IMPTOP}/${SORTDATE}-${shortrev}
IMPVERSION="${SORTDATE}.${VER}${shortrev}"
IMPSRCTGZ=${IMPINSTALL}/build/sources/imp-${IMPVERSION}.tar.gz
rm -rf ${IMPINSTALL}
mkdir -p ${IMPINSTALL}/build/sources ${IMPINSTALL}/build/logs

# Make link so build system can find the install location
rm -f ${IMPTOP}/.SVN-new
ln -s ${IMPINSTALL} ${IMPTOP}/.SVN-new

# Get IMP code from git
tar -C ${GIT_TOP} --exclude .git -cf - imp | tar -xf -

# Put version number, date and revision into relevant files
(cd imp/doc/doxygen && sed -e "s#^PROJECT_NUMBER.*#PROJECT_NUMBER = ${VER}${shortrev}, ${DATE}#" < Doxyfile.in > .dox && mv .dox Doxyfile.in)
echo "${VER}${shortrev}" > imp/VERSION

# Write out version files
verfile="${IMPINSTALL}/build/imp-version"
revfile="${IMPINSTALL}/build/imp-gitrev"
mkdir -p "${IMPINSTALL}/build"
echo "${IMPVERSION}" > $verfile
echo "${rev}" > $revfile

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
END

# Write out a tarball:
mv imp imp-${IMPVERSION} && tar -czf ${IMPSRCTGZ} imp-${IMPVERSION}

# Cleanup
cd /
rm -rf ${TMPDIR}
