#!/bin/csh

pushd modules/rmf/dependency/RMF_source
git checkout develop
git pull origin develop
git submodule update
set hash=`git rev-parse HEAD`
popd
echo "Committing"
git commit  modules/rmf/dependency/RMF_source -m "bump RMF to salilab/rmf@$hash" --no-verify
