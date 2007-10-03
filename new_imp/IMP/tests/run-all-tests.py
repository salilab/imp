import unittest, sys, os, re

sys.path.append('python_libs')

def regressionTest():
    path = os.path.abspath(os.path.dirname(sys.argv[0]))
    modobjs = []
    for subdir in ('connectivity', 'pair_connectivity', 'torus',
                   'coordinate', 'proximity', 'exclusion_volumes', 'xml',
                   'particles', 'distance'):
        files = os.listdir(os.path.join(path, subdir))
        test = re.compile("^test_.*\.py$", re.IGNORECASE)
        files = filter(test.search, files)
        modnames = [os.path.splitext(f)[0] for f in files]
        sys.path.insert(0, subdir)
        modobjs.extend([__import__(m) for m in modnames])
        sys.path.pop(0)

    tests = [unittest.defaultTestLoader.loadTestsFromModule(o) for o in modobjs]
    return unittest.TestSuite(tests)

if __name__ == "__main__":
    unittest.main(defaultTest="regressionTest")
