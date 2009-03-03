import unittest, sys, os, re
#global path

def regressionTest():
    """Run all tests in files called test_*.py in current directory and
       subdirectories"""
    path ="."
    if path == '':
        path = '.'
    # Tell test cases where to find any input files needed
    os.environ['TEST_DIRECTORY'] = path
    modobjs = []
    for subdir in [''] + [x for x in os.listdir(path) \
                          if os.path.isdir(os.path.join(path, x))]:
        files = os.listdir(os.path.join(path, subdir))
        test = re.compile("^test_.*\.py$", re.IGNORECASE)
        files = filter(test.search, files)
        modnames = [os.path.splitext(f)[0] for f in files]
        sys.path.insert(0, os.path.join(path, subdir))
        modobjs.extend([__import__(m) for m in modnames])
        sys.path.pop(0)

    tests = [unittest.defaultTestLoader.loadTestsFromModule(o) for o in modobjs]
    return unittest.TestSuite(tests)

if __name__ == "__main__":
    #global path
    #path = os.path.dirname(sys.argv[1])
    #sys.argv=[sys.argv[0]]+sys.argv[2:]
    unittest.main(defaultTest="regressionTest")
