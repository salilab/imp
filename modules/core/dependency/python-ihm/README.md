[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.2603378.svg)](https://doi.org/10.5281/zenodo.2603378)
[![docs](https://readthedocs.org/projects/python-ihm/badge/)](https://python-ihm.readthedocs.org/)
[![conda package](https://img.shields.io/conda/vn/conda-forge/ihm.svg)](https://anaconda.org/conda-forge/ihm)
[![pypi package](https://badge.fury.io/py/ihm.svg)](https://badge.fury.io/py/ihm)
[![Linux Build Status](https://github.com/ihmwg/python-ihm/workflows/build/badge.svg)](https://github.com/ihmwg/python-ihm/actions?query=workflow%3Abuild)
[![Windows Build Status](https://ci.appveyor.com/api/projects/status/5o28oe477ii8ur4h?svg=true)](https://ci.appveyor.com/project/benmwebb/python-ihm)
[![codecov](https://codecov.io/gh/ihmwg/python-ihm/branch/main/graph/badge.svg)](https://codecov.io/gh/ihmwg/python-ihm)

This is a Python package to assist in handling [mmCIF](http://mmcif.wwpdb.org/)
and [BinaryCIF](https://github.com/dsehnal/BinaryCIF) files compliant with the
[integrative/hybrid modeling (IHM)](http://mmcif.wwpdb.org/dictionaries/mmcif_ihm.dic/Index/)
extension. It works with Python 2.7 or Python 3.

To handle non-integrative theoretical models (for example, homology models),
see the [python-modelcif](https://github.com/ihmwg/python-modelcif) package
which supports files compliant with the
[ModelCIF](https://mmcif.wwpdb.org/dictionaries/mmcif_ma.dic/Index/)
extension.

Please [see the documentation](https://python-ihm.readthedocs.org/)
or some
[worked examples](https://github.com/ihmwg/python-ihm/tree/main/examples)
for more details.

# Installation with conda or pip

If you are using [Anaconda Python](https://www.anaconda.com/), install with

```
conda install -c conda-forge ihm
```

Alternatively, install with pip:

```
pip install ihm
```

(Note that pip builds a C extension module for faster parsing of mmCIF files.
This requires that your system has a C compiler. If you don't want to build
the C extension module, install with
`pip install ihm --install-option="--without-ext"`.)

# Installation from source code

To build and install from a clone of the GitHub repository, run

```
python setup.py build
python setup.py install
```

Note that a C extension module is built for faster parsing of mmCIF files.
This requires that your system has a C compiler
and [SWIG](http://www.swig.org/). If either of these components are missing, you
can choose to build without the extension by adding `--without-ext` to both
`setup.py` command lines above.

If you want to read or write [BinaryCIF](https://github.com/dsehnal/BinaryCIF)
files, you will also need the
Python [msgpack](https://github.com/msgpack/msgpack-python) package.

# Testing

There are a number of testcases in the `test` directory. Each one can be run
like a normal Python script to test the library. They can also be all run at
once using [nose](https://nose.readthedocs.io/en/latest/)
or [pytest](https://docs.pytest.org/en/latest/). They will also test
the C extension module if it is first built with
`python setup.py build_ext --inplace`.
