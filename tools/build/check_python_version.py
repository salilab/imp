#!/usr/bin/env python
import sys

if sys.version_info[0] == 2 and sys.version_info[1] < 6:
    sys.stderr.write("""
Python version 2.6 or later is required to build and use IMP (note that Python 3 is not currently supported).
The 'python' binary (found in your path at %s) is version %d.%d. Please ensure that you have a suitable version of Python available, and it is in your PATH. e.g. if you have Python 2.6 as /usr/local/bin/python26 something like the following would work:
    mkdir bin
    ln -sf /usr/local/bin/python26 bin/python
    PATH=`pwd`/bin:$PATH
""" % (sys.executable, sys.version_info[0], sys.version_info[1]))
    sys.exit(1)
