#!/bin/bash -e

# Set up an environment to run tests under Travis CI (see ../.travis.yml)

if [ $# -ne 1 ]; then
  echo "Usage: $0 python_version"
  exit 1
fi

cur_dir=$(pwd)
python_version=$1
temp_dir=$(mktemp -d)

cd ${temp_dir}

conda update --yes -q conda
conda create --yes -q -n python${python_version} -c salilab python=${python_version} pip scipy matplotlib imp-nightly gxx_linux-64 eigen swig cmake
eval "$(conda shell.bash hook)"
conda activate python${python_version}
pip install pytest-cov coverage pytest-flake8

cd ${cur_dir}

rm -rf ${temp_dir}
