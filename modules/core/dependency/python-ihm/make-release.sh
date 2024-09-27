#!/bin/bash -e

# First, do
# - Update AuditConformDumper to match latest IHM dictionary if necessary
# - Run util/validate-outputs.py to make sure all example outputs validate
#   (cd util; PYTHONPATH=.. python3 ./validate-outputs.py)
# - Make sure all python-modelcif tests work using new IHM version
# - Update ChangeLog.rst, util/debian/changelog, and util/python-ihm.spec
#   with the release number and date
# - Update release number in ihm/__init__.py, MANIFEST.in, and setup.py
# - Commit, tag, and push
# - Make release on GitHub
# - Upload the release tarball from
#   https://github.com/ihmwg/python-ihm/releases to Zenodo as a new release
# - Make sure there are no extraneous .py files (setup.py will include them
#   in the pypi package)

# Make SWIG wrapper so users don't need SWIG
rm -rf build src/ihm_format_wrap.c
python3 setup.py build_ext --inplace
VERSION=$(python3 setup.py --version)
mv src/ihm_format_wrap.c "src/ihm_format_wrap_${VERSION}.c"

python3 setup.py sdist
rm -f "src/ihm_format_wrap_${VERSION}.c"

echo "Now use 'twine upload dist/ihm-${VERSION}.tar.gz' to publish the release on PyPi."
echo "Then, update the conda-forge, COPR, PPA, and Homebrew packages to match."
echo "For COPR, use dist/ihm-${VERSION}.tar.gz together with util/python-ihm.spec"
echo "For the PPA, use the GitHub release tarball, renamed to python-ihm_${VERSION}.orig.tar.gz"
