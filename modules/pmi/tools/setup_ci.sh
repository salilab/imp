#!/bin/bash -e

# Set up an environment to run CI tests, e.g. with GitHub Actions or Travis

if [ $# -ne 1 ]; then
  echo "Usage: $0 python_version"
  exit 1
fi

python_version=$1

conda config --remove channels defaults  # get conda-forge, not main, packages
conda create --yes -q -n python${python_version} -c salilab -c conda-forge python=${python_version} pip scipy matplotlib imp-nightly gxx_linux-64 eigen swig cmake
eval "$(conda shell.bash hook)"
conda activate python${python_version}
pip install pytest-cov coverage pytest-flake8
