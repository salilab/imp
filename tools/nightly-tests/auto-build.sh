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

# Write out list of all modules
modfile="${IMPINSTALL}/build/imp-modules"
python <<END
import glob
import sys
import os
modules = []
class tools:
    class paths:
        def get_sconscripts(env, mods):
            modules.extend(mods)
            return mods
        get_sconscripts = staticmethod(get_sconscripts)
sys.modules['scons_tools'] = tools
sys.modules['scons_tools.paths'] = tools.paths
def Import(var): pass
def SConscript(var): pass
env = {'local':True} 
for path in ('imp/applications/',
             'imp/biological_systems/'):
    def Glob(pattern):
        lp = len(path)
        return [x[lp:] for x in glob.glob(path + pattern)]
    exec(open(path + 'SConscript').read())

modules = os.listdir('imp/modules')
f = open('$modfile', 'w')
for m in modules:
    print >> f, "module\t" + m
for m in applications:
    print >> f, "application\t" + m
for m in systems:
    print >> f, "system\t" + m
END

# Write out a tarball:
mv imp imp-${IMPVERSION} && tar -czf ${IMPSRCTGZ} imp-${IMPVERSION}

# Make an RMF tarball from our git repo:
(cd /cowbell1/git && tar -czf ${RMFSRCTGZ} --exclude .git rmf)

# Cleanup
cd /
rm -rf ${TMPDIR}
