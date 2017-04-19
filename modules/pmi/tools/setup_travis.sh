#!/bin/bash -e

# Set up an environment to run tests under Travis CI (see ../.travis.yml)

if [ $# -ne 2 ]; then
  echo "Usage: $0 conda_dir python_version"
  exit 1
fi

pmi_dir=`pwd`
conda_dir=$1
python_version=$2
temp_dir=`mktemp -d`

cd ${temp_dir}

# Use miniconda Python rather than the Travis environment

# Clean up after a potential previous install failure
rm -rf ${conda_dir}
# Save on some downloading if the version is the same
if [ "${python_version}" == "2.7" ]; then
  wget https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh -O miniconda.sh
else
  wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
fi
bash miniconda.sh -b -p ${conda_dir}
export PATH=${conda_dir}/bin:$PATH
conda update --yes -q conda
conda create --yes -q -n python${python_version} -c salilab python=${python_version} pip scipy nose imp
source activate python${python_version}
pip install coverage

# Replace PMI in IMP with that from git
IMP_PATH=$(echo "import IMP, sys, os; sys.stdout.write(os.path.dirname(IMP.__file__))" | python)
cd ${IMP_PATH}
mv pmi pmi.orig
cp -sr ${pmi_dir}/pyext/src pmi
cp pmi.orig/__init__.py pmi.orig/_version_check.py pmi/

# IMP tests use sys.argv[0] to determine their location, which won't work if
# we use nosetests, so add a workaround
ln -sf `which nosetests` ${pmi_dir}/test/

rm -rf ${temp_dir}
