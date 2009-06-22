import unittest
import sys
import os
import re
import imp

global path

class _TestModuleImporter(object):
    """Import a Python test module. The module
       is given a unique name (_test_0, _test_1 etc.) so that modules with
       the same name but in different directories can be read without
       overwriting each other in sys.modules."""
    def __init__(self):
        self._serial = 0
    def __call__(self, name):
        fh, pathname, desc = imp.find_module(name)
        self._serial += 1
        try:
            return imp.load_module('_test_%d' % self._serial, fh,
                                   pathname, desc)
        finally:
            # Make sure the file is closed regardless of what happens
            if fh:
                fh.close()
_import_test = _TestModuleImporter()

def regressionTest():
    """Run all tests in files called test_*.py in current directory and
       subdirectories"""
    os.environ['TEST_DIRECTORY'] = path
    modobjs = []
    test = re.compile("test_.*\.py$", re.IGNORECASE)
    for subdir in [''] + [x for x in os.listdir(path) \
                          if os.path.isdir(os.path.join(path, x))]:
        subpath = os.path.join(path, subdir)
        modnames = [os.path.splitext(f)[0] for f in os.listdir(subpath) \
                    if test.match(f)]
        sys.path.insert(0, subpath)
        modobjs.extend([_import_test(m) for m in modnames])
        sys.path.pop(0)

    tests = [unittest.defaultTestLoader.loadTestsFromModule(o) for o in modobjs]
    return unittest.TestSuite(tests)

if __name__ == "__main__":
    path = sys.argv[1]
    sys.argv=[sys.argv[0]]+sys.argv[2:]
    unittest.main(defaultTest="regressionTest")
