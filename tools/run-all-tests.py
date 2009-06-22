import unittest, sys, os, re
global path

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
        modobjs.extend([__import__(m) for m in modnames])
        sys.path.pop(0)

    tests = [unittest.defaultTestLoader.loadTestsFromModule(o) for o in modobjs]
    return unittest.TestSuite(tests)

if __name__ == "__main__":
    path = sys.argv[1]
    sys.argv=[sys.argv[0]]+sys.argv[2:]
    unittest.main(defaultTest="regressionTest")
