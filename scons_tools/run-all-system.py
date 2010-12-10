import IMP.test
from IMP.test import unittest
import sys
import os
import os.path


def _test_example(dir, working_dir, index, filename, shortname):
    skip = "pass"
    if shortname.startswith("sample"):
        args=["1", "1000"]
    else:
        args=[]
    if shortname == "setup":
        pass
    else:
        if shortname != "sample_0.py":
            args= args+["-i",os.path.join(working_dir, "files_"+str(index-1))]
        args = args+["-o",os.path.join(working_dir, "files_"+str(index))]
    exec(
"""class RunExample(IMP.test.ApplicationTestCase):
        def test_run_example(self):
            "Run %(shortname)s"
            import os
            os.chdir("%(dir)s")
            cmd="python %(script)s %(args)s"
            app=self.run_script("%(script)s", %(args)s)
            out = app.stdout.readlines()
            ret, err= app.wait()
            self.assertApplicationExitedCleanly(ret, err)
            return""" % {"shortname":shortname, "dir":dir, "script":os.path.join(dir,filename), "args":str(args)})
    return RunExample("test_run_example")

global files
global excluded_modules
global working_dir
def regressionTest():
    modobjs = []
    suite = unittest.TestSuite()

    # For all examples that don't have an explicit test to exercise them,
    # just run them to make sure they don't crash
    for i,f in enumerate(files):
        nm= os.path.split(f)[1]
        dir= os.path.split(f)[0]
        suite.addTest(_test_example(dir, working_dir, i, f, nm))

    return suite


if __name__ == "__main__":
    files = sys.argv[2:]
    working_dir= sys.argv[1]
    sys.argv=[sys.argv[0], "-v"]
    unittest.main(defaultTest="regressionTest", testRunner=IMP.test._TestRunner)
