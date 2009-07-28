import unittest
import sys
import os
import re
import imp
import os.path
global files

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
    if len(files) ==0:
        return unittest.TestSuite([])
    a_file=files[0]
    # evil hack
    os.environ['TEST_DIRECTORY'] = a_file[0:a_file.find("/test/")+6]
    print os.environ['TEST_DIRECTORY']
    #return 0
    modobjs = []
    for f in files:
        print f
        nm= os.path.split(f)[1]
        print nm
        dir= os.path.split(f)[0]
        print dir
        modname = os.path.splitext(nm)[0]
        sys.path.insert(0, dir)
        modobjs.extend([_import_test(modname)])
        print modobjs
        sys.path.pop(0)

    tests = [unittest.defaultTestLoader.loadTestsFromModule(o) for o in modobjs]
    return unittest.TestSuite(tests)

if __name__ == "__main__":
    files = sys.argv[1:]
    print files
    sys.argv=[sys.argv[0], "-v"]
    unittest.main(defaultTest="regressionTest")
