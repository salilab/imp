[![docs](https://readthedocs.org/projects/python-ihm/badge/)](https://python-ihm.readthedocs.org/)
[![Linux Build Status](https://travis-ci.org/ihmwg/python-ihm.svg?branch=master)](https://travis-ci.org/ihmwg/python-ihm)
[![Windows Build Status](https://ci.appveyor.com/api/projects/status/5o28oe477ii8ur4h?svg=true)](https://ci.appveyor.com/project/benmwebb/python-ihm)
[![codecov](https://codecov.io/gh/ihmwg/python-ihm/branch/master/graph/badge.svg)](https://codecov.io/gh/ihmwg/python-ihm)

This is a Python package to assist in handling [mmCIF](http://mmcif.wwpdb.org/)
and [BinaryCIF](https://github.com/dsehnal/BinaryCIF) files compliant with the
[integrative/hybrid modeling (IHM)](http://mmcif.wwpdb.org/dictionaries/mmcif_ihm.dic/Index/)
extension. It works with Python 2.6 or later (Python 3 is fully supported).

Please [see the documentation](https://python-ihm.readthedocs.org/)
or some
[worked examples](https://github.com/ihmwg/python-ihm/tree/master/examples)
for more details.

# Installation with pip

```
pip install ihm
```

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
once using [nose](https://nose.readthedocs.io/en/latest/). They will also test
the C extension module if it is first built with
`python setup.py build_ext --inplace`.
