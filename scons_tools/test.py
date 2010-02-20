from SCons.Script import Glob, Dir, File, Builder, Action
import pyscanner
import os
import os.path



def _action_unit_test(target, source, env):
    #app = "cd %s; %s %s %s -v > /dev/null"
    fsource=[]
    for x in source[2:]:
        if str(x).endswith(".py"):
            fsource.append(x.abspath)
    app = "%s %s %s %s > /dev/null" \
          % (source[0].abspath, env['PYTHON'],
             source[1].abspath,
             " ".join(fsource))
    if env.Execute(app) == 0:
        file(str(target[0]), 'w').write('PASSED\n')
    else:
        print "IMP.%s unit tests FAILED" % env['IMP_MODULE']
        return 1


def _print_unit_test(target, source, env):
    print "Generating unit testing"

UnitTest = Builder(action=Action(_action_unit_test,
                                _print_unit_test),
                   source_scanner=pyscanner.PythonScanner)



def _action_cpp_test(target, source, env):
    #app = "cd %s; %s %s %s -v > /dev/null"
    out= open(target[0].abspath, "w")
    print >> out, """
import unittest
import IMP
import IMP.test
import os
import os.path

class DirectoriesTests(IMP.test.TestCase):
"""
    for t in source:
        nm= os.path.split(str(t))[1].replace(".", "_")
        print >> out, """
    def test_%(name)s(self):
       \"\"\"Running C++ test %(name)s\"\"\"
       self.assert_(os.system("%(path)s")==0)
""" %{'name':nm, 'path':t.abspath}
    print >> out, """
if __name__ == '__main__':
    unittest.main()
"""


def _print_cpp_test(target, source, env):
    print "Generating cpp testing harness"

CPPTestHarness = Builder(action=Action(_action_cpp_test,
                                       _print_cpp_test))
