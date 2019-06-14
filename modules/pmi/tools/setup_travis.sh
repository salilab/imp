#!/bin/bash -e

# Set up an environment to run tests under Travis CI (see ../.travis.yml)

if [ $# -ne 2 ]; then
  echo "Usage: $0 conda_dir python_version"
  exit 1
fi

cur_dir=$(pwd)
conda_dir=$1
python_version=$2
temp_dir=$(mktemp -d)

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
conda create --yes -q -n python${python_version} -c salilab python=${python_version} pip scipy matplotlib nose imp-nightly gxx_linux-64 eigen swig cmake
source activate python${python_version}
pip install coverage

source ${CONDA_PREFIX}/etc/conda/activate.d/activate-gcc_linux-64.sh
source ${CONDA_PREFIX}/etc/conda/activate.d/activate-gxx_linux-64.sh

# IMP tests use sys.argv[0] to determine their location, which won't work if
# we use nosetests, so add a workaround
ln -sf $(which nosetests) ${cur_dir}/test/

cd ${cur_dir}

rm -rf ${temp_dir}
