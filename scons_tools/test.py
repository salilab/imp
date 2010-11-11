from SCons.Script import Glob, Dir, File, Builder, Action
import pyscanner
import environment
import os
import os.path
import scons_tools.module

# List of all disabled IMP modules (populated at configure time)
disabled_modules = []

def _get_name(env):
    return environment.get_current_name(env)

def _action_unit_test(target, source, env):
    global disabled_modules
    #app = "cd %s; %s %s %s -v > /dev/null"
    fsource=[]
    for x in source[2:-1]:
        if str(x).endswith(".py"):
            fsource.append(x.abspath)
    if source[-1] == 'example':
        # Quote list of modules so that the shell passes an empty parameter
        # to the script if there are no disabled modules (rather than the
        # script treating the first file as the list of disabled modules)
        disab = ' "%s"' % ":".join(disabled_modules)
    else:
        disab = ''

    app = "mkdir -p %s; cd %s; %s %s %s%s %s > /dev/null" \
          % (Dir("#/build/tmp").abspath, Dir("#/build/tmp").abspath, source[0].abspath, env['PYTHON'],
             source[1].abspath, disab,
             " ".join(fsource))
    if env.Execute(app) == 0:
        file(str(target[0]), 'w').write('PASSED\n')
        print "%s %ss succeeded" % (_get_name(env), source[-1])
    else:
        print "%s %ss FAILED" % (_get_name(env), source[-1])
        return 1


def _print_unit_test(target, source, env):
    print "Running %s %ss" % (_get_name(env), source[-1])

UnitTest = Builder(action=Action(_action_unit_test,
                                _print_unit_test),
                   source_scanner=pyscanner.PythonScanner)


def add_test(env, source, type):
    test=UnitTest(env, target="test.passed", source=source+[env.Value(type)])
    env.Requires(test, "#/build/lib/compat_python")
    env.AlwaysBuild("test.passed")
    env.Requires(test, env.Alias(environment.get_current_name(env)+"-build"))
    env.Requires(test, "tools/imppy.sh")
    if type=='unit test':
        env.Alias(env.Alias(environment.get_current_name(env)+"-test"), test)
    elif type=='example':
        env.Alias(env.Alias(environment.get_current_name(env)+"-test-examples"), test)
    env.Alias(env.Alias('test'), test)
    return test


def _action_cpp_test(target, source, env):
    #app = "cd %s; %s %s %s -v > /dev/null"
    out= open(target[0].abspath, "w")
    print >> out, \
"""import IMP
import IMP.test
import sys
try:
    import subprocess
except ImportError:
    subprocess = None

class TestCppProgram(IMP.test.TestCase):"""
    for t in source:
        nm= os.path.split(str(t))[1].replace(".", "_")
        print >> out, \
"""    def test_%(name)s(self):
        \"\"\"Running C++ test %(name)s\"\"\"
        if subprocess is None:
            self.skipTest("subprocess module unavailable")
        # Note: Windows binaries look for needed DLLs in the current
        # directory. So we need to change into the directory where the DLLs have
        # been installed for the binary to load correctly.
        p = subprocess.Popen(["%(path)s"],
                             shell=False, cwd="%(libdir)s")
        self.assertEqual(p.wait(), 0)""" \
       %{'name':nm, 'path':t.abspath, 'libdir':env.Dir('#/build/lib').abspath}
    print >> out, """
if __name__ == '__main__':
    IMP.test.main()"""


def _print_cpp_test(target, source, env):
    print "Generating cpp testing harness"

CPPTestHarness = Builder(action=Action(_action_cpp_test,
                                       _print_cpp_test))
