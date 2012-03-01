from SCons.Script import Glob, Dir, File, Builder, Action
import pyscanner
import environment
import os
import os.path
import module
import utility
import data
import tempfile

# List of all disabled IMP modules (populated at configure time)
disabled_modules = []

def _get_name(env):
    return environment.get_current_name(env)

def _action_unit_test(target, source, env):
    global disabled_modules
    #app = "cd %s; %s %s %s -v > /dev/null"
    fsource=[]
    type= source[-1].get_contents()
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
    tmpdir=Dir(env["builddir"]+"/tmp").abspath
    tf=target[0].abspath
    filename= target[0].abspath
    if type.endswith("unit test"):
        cmd= File("#/scons_tools/run-all-tests.py").abspath
        if type.startswith('module'):
            modname = _get_name(env)
            if modname == 'kernel':
                modname = 'IMP'
            else:
                modname = 'IMP.' + modname
            cmd += ' --module=%s --pycoverage=%s' % (modname, env['pycoverage'])
            if env['pycoverage'] == 'lines':
                cmd += ' --output=%s.pycoverage' % tf
            data.get(env).env.Append(IMP_TESTS=[(modname, filename)])
        elif type.startswith('application'):
            cmd += ' --application=%s --pycoverage=%s' % (_get_name(env),
                                                          env['pycoverage'])
            for pyexe in env.get('IMP_PYTHON_EXECUTABLES', []):
                cmd += ' --pyexe=%s' % pyexe
            if env['pycoverage'] == 'lines':
                cmd += ' --output=%s.pycoverage' % tf

        cmd+=" --results="+filename
        #if len(fsource) > 0:
        #    env.Append(ENV={'TEST_DIRECTORY':fsource[0][0:fsource[0].find("/test/")+6]})
        #    #print "test dir", os.environ['TEST_DIRECTORY']
    elif type=='example':
        cmd= File("#/scons_tools/run-all-examples.py").abspath
        dmod=[]
        for d in data.get(env).modules.keys():
            if not data.get(env).modules[d].ok:
                dmod.append(d)
        cmd= cmd+ ' --excluded='+":".join(dmod)
        cmd+=" --results="+filename
    elif type=='system':
        cmd= File("#/scons_tools/run-all-system.py").abspath + " " +Dir(env["builddir"]+"/tmp").abspath
    else:
        utility.report_error(env, "Unknown test type "+type)
    print "tempfile", tf, tmpdir, target[0].path+".result"
    app = "mkdir -p %s; cd %s; (%s %s %s%s %s >%s) > /dev/null" \
              % (tmpdir, tmpdir, source[0].abspath, env['PYTHON'],
                 cmd, disab,
                 " ".join(fsource), tf+".out")
    print app
    if env.Execute(app) == 0:
        #file(str(target[0]), 'w').write('PASSED\n')
        print "%s %ss succeeded" % (_get_name(env), source[-1])
    else:
        print "%s %ss FAILED" % (_get_name(env), source[-1])
        return 1


def _print_unit_test(target, source, env):
    print "Running %s %ss" % (_get_name(env), source[-1])

UnitTest = Builder(action=Action(_action_unit_test,
                                _print_unit_test),
                   source_scanner=pyscanner.PythonScanner)


def add_tests(env, source, type, expensive_source=[]):
    # Since all of the test scripts involve "import IMP.test", ensure this
    # is a source so that any Python dependencies of IMP.test (e.g. IMP.base)
    # are automatically picked up by pyscanner
    testpy = env["builddir"] + "/lib/IMP/test/__init__.py"
    test=UnitTest(env, target="fast-test.results",
                  source=["#/tools/imppy.sh", testpy]+source+[env.Value(type)])
    etest=UnitTest(env, target="test.results",
                   source=["#/tools/imppy.sh", testpy]+source \
                          +expensive_source+[env.Value(type)])
    for t in test, etest:
        env.Requires(t, env["builddir"]+"/lib/compat_python")
    env.AlwaysBuild("test.results")
    #env.Requires(test, env.Alias(environment.get_current_name(env)))
    #env.Requires(test, "tools/imppy.sh")
    if type.endswith('unit test'):
        data.get(env).add_to_alias(environment.get_current_name(env)+"-test-fast", test)
        data.get(env).add_to_alias(environment.get_current_name(env)+"-test", etest)
    elif type=='example':
        data.get(env).add_to_alias(environment.get_current_name(env)+"-test-examples",
                                               test)
    elif type=="system":
        data.get(env).add_to_alias(environment.get_current_name(env)+"-test", test)
    env.Alias(env.Alias('test'), test)
    env.Alias(env.Alias('test-fast'), etest)
    return test, etest


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
        #print "path is ", t.abspath
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
       %{'name':nm, 'path':t.abspath, 'libdir':env.Dir(env['builddir']+'/lib').abspath}
    print >> out, """
if __name__ == '__main__':
    IMP.test.main()"""


def _print_cpp_test(target, source, env):
    print "Generating cpp testing harness"

CPPTestHarness = Builder(action=Action(_action_cpp_test,
                                       _print_cpp_test))
