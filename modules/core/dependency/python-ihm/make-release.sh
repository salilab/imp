#!/bin/sh -e

# First, do
# - Update ChangeLog.rst with the release number
# - Update release number in ihm/__init__.py, MANIFEST.in, and setup.py
# - Commit and tag

# Make SWIG wrapper so users don't need SWIG
rm -rf build src/ihm_format_wrap.c
python3 setup.py build_ext --inplace
VERSION=`python3 setup.py --version`
mv src/ihm_format_wrap.c src/ihm_format_wrap_${VERSION}.c

python3 setup.py sdist
rm -f src/ihm_format_wrap_${VERSION}.c

echo "Now use 'twine upload dist/ihm-${VERSION}.tar.gz' to publish the release on PyPi"
