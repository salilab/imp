#!/bin/sh

# Script to get a git branch and make a .tar.gz on a shared disk, for use by
# autobuild scripts on our build machines.
#
# Should be run on from a crontab on a machine that has access to a git
# clone, e.g.
#
# 10 1 * * * /cowbell1/home/ben/imp/tools/auto-build.sh develop

if [ $# -ne 1 ]; then
  echo "Usage: $0 branch"
  exit 1
fi

GIT_TOP=/cowbell1/git

BRANCH=$1

TMPDIR=/var/tmp/imp-build-$$
IMPTOP=/salilab/diva1/home/imp/$BRANCH
mkdir -p ${IMPTOP}

cd ${GIT_TOP}/imp.git

# Get top-most revision of branch we're interested in
rev=`git rev-parse ${BRANCH}`
shortrev=`git rev-parse --short ${BRANCH}`

# Get old revision
oldrev_file=${IMPTOP}/.SVN-new/build/imp-gitrev
if [ -f "${oldrev_file}" ]; then
  oldrev=`cat ${oldrev_file}`
fi

# For non-develop builds, skip if the revision hasn't changed
if [ ${BRANCH} != "develop" -a "${oldrev}" = "${rev}" ]; then
  exit 0
fi

rm -rf ${TMPDIR}
mkdir ${TMPDIR}
cd ${TMPDIR}

# Get IMP code from git
git clone -b ${BRANCH} -q ${GIT_TOP}/imp.git

# Update any submodules, etc. if necessary
(cd imp && git submodule --quiet update --init --recursive) > /dev/null
(cd imp && ./setup_git.py) > /dev/null || exit 1

if [-d imp/modules/rmf/dependency/RMF_source ]; then
  # Get submodule revision
  RMF_rev=`(cd imp/modules/rmf/dependency/RMF_source && git rev-parse HEAD)`
else
  # Harder to do for subtree; parse the specially-formatted log message
  RMF_rev=`(cd imp && git log --grep='git\-subtree\-dir: modules\/rmf\/dependency\/RMF' -n 1|grep git-subtree-split|cut -d: -f 2)`
fi

# Get date and revision-specific install directories
SORTDATE=`date -u "+%Y%m%d"`
DATE=`date -u +'%Y/%m/%d'`
IMPINSTALL=${IMPTOP}/${SORTDATE}-${shortrev}
# Make sure VERSION file is reasonable
(cd imp && rm -f VERSION && tools/build/make_version.py --source=.)
if [ ${BRANCH} = "develop" ]; then
  # For nightly builds, prepend the date so the packages are upgradeable
  IMPVERSION="${SORTDATE}.develop.${shortrev}"
else
  IMPVERSION="`cat imp/VERSION | sed -e 's/[ /-]/./g'`"
  # For stable releases, assign submodules the same version as IMP itself
  rm -f imp/modules/*/VERSION
fi
IMPSRCTGZ=${IMPINSTALL}/build/sources/imp-${IMPVERSION}.tar.gz
rm -rf ${IMPINSTALL}
mkdir -p ${IMPINSTALL}/build/sources ${IMPINSTALL}/build/logs

# Make link so build system can find the install location
rm -f ${IMPTOP}/.SVN-new
ln -s ${IMPINSTALL} ${IMPTOP}/.SVN-new

# Add build date to nightly docs
if [ ${BRANCH} = "develop" ]; then
  IMPVER="develop.${shortrev}"
  (cd imp/tools/build/doxygen_templates && sed -e "s#^PROJECT_NUMBER.*#PROJECT_NUMBER = ${IMPVER}, ${DATE}#" < Doxyfile.in > .dox && mv .dox Doxyfile.in)
fi

# Write out version files
verfile="${IMPINSTALL}/build/imp-version"
revfile="${IMPINSTALL}/build/imp-gitrev"
RMF_revfile="${IMPINSTALL}/build/rmf-gitrev"
mkdir -p "${IMPINSTALL}/build"
echo "${IMPVERSION}" > $verfile
echo "${rev}" > $revfile
echo "${RMF_rev}" > $RMF_revfile

# Write out log from previous build to this one
logfile="${IMPINSTALL}/build/imp-gitlog"
if [ -n "${oldrev}" ]; then
  (cd imp && git log ${oldrev}..${rev} --format="%H%x00%an%x00%ae%x00%s") > ${logfile}
fi

# Write out list of all components
compfile="${IMPINSTALL}/build/imp-components"
python <<END
import sys
sys.path.insert(0, 'imp/tools/build')
import tools

f = open('$compfile', 'w')
for m in tools.compute_sorted_order('imp', ''):
    print >> f, "module\t" + m
if hasattr(tools, 'get_applications'):
    for a in tools.get_applications('imp'):
        print >> f, "application\t" + a[0]
END

# Write out a tarball:
mv imp imp-${IMPVERSION} && tar --exclude .git -czf ${IMPSRCTGZ} imp-${IMPVERSION}

# Cleanup
cd /
rm -rf ${TMPDIR}
