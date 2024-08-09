#!/usr/bin/env python
import sys

if (sys.version_info[0] < 3
        or (sys.version_info[0] == 3 and sys.version_info[1] < 6)):
    sys.stderr.write("""
Python version 3.6 or later is required to build and use IMP.
The Python binary (found in your path at %s) is version %d.%d. Please ensure that you have a suitable version of Python available, and it is in your PATH. e.g. if you have Python 3.6 as /usr/local/bin/python36 something like the following would work:
    mkdir bin
    ln -sf /usr/local/bin/python36 bin/python3
    PATH=`pwd`/bin:$PATH
    """ % (sys.executable, sys.version_info[0], sys.version_info[1]))   # noqa: E501
    sys.exit(1)
