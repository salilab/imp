import unittest, sys, os, re

testdir="modules/core/test/"

def regressionTest():
    """Run all tests in files called test_*.py in current directory and
       subdirectories"""
    if os.path.split(os.path.split(os.getcwd())[0])[0] == "kernel":
        root = os.path.split(os.path.split(os.getcwd())[0])[0]
        relpath= os.path.join(os.path.split(os.path.split(os.getcwd())[0])[1],\
                                  os.path.split(os.getcwd())[1])
    else:
        root = os.path.split(os.path.split(os.path.split(os.getcwd())[0])[0])[0]
        relpath= os.path.join(os.path.split(os.path.split(os.path.split(os.getcwd())[0])[0])[1],\
                           os.path.split(os.path.split(os.getcwd())[0])[1],\
                              os.path.split(os.getcwd())[1])
    print root
    print relpath
    os.chdir(root)
    subdirs = [os.path.join(relpath, x) for x in os.listdir(relpath) if os.path.isdir(os.path.join(relpath, x))]
    print subdirs
    modobjs=[]
    for subdir in subdirs:
        print subdir
        files = [os.path.join(subdir, x) for x in os.listdir(subdir) if x.endswith(".py")]
        print files
        #print files
        modnames = [os.path.splitext(f)[0] for f in files]
        print modnames
        sys.path.insert(0, subdir)
        for m in modnames:
            print m
            nm= os.path.split(m)[1]
            print nm
            modobjs.append(__import__(nm))
        sys.path.pop(0)
    print "all done"
    print modobjs
    tests = [unittest.defaultTestLoader.loadTestsFromModule(o) for o in modobjs]
    return unittest.TestSuite(tests)

if __name__ == "__main__":
    unittest.main(defaultTest="regressionTest")
