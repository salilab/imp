from SCons.Script import Glob, Dir, File, Builder, Action
import pyscanner
import cpp_coverage
import environment
import os
import os.path
import module
import utility
import data

# List of all disabled IMP modules (populated at configure time)
disabled_modules = []

def _get_name(env):
    return environment.get_current_name(env)

def _action_unit_test(target, source, env):
    global disabled_modules
    #app = "cd %s; %s %s %s -v > /dev/null"
    fsource=[]
    type= source[-1].get_contents()
    for x in source[1:-1]:
        if str(x).endswith(".py"):
            fsource.append(x.abspath)
    if source[-1] == 'example':
        # Quote list of modules so that the shell passes an empty parameter
        # to the script if there are no disabled modules (rather than the
        # script treating the first file as the list of disabled modules)
        disab = ' "%s"' % ":".join(disabled_modules)
    else:
        disab = ''
    tmpdir=Dir("#/build/tmp").abspath
    tf=target[0].abspath
    filename= target[0].abspath
    execute = env.Execute
    if type.endswith("unit test"):
        cmd= File("#/scons_tools/run-all-tests.py").abspath
        if env['cppcoverage'] != 'no':
            c = cpp_coverage._CoverageTester(env, env['cppcoverage'], type, tf)
            execute = c.Execute
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
            appname = _get_name(env)
            cmd += ' --application=%s --pycoverage=%s' % (appname,
                                                          env['pycoverage'])
            for pyexe in env.get('IMP_PYTHON_EXECUTABLES', []):
                cmd += ' --pyexe=%s' % pyexe
            if env['pycoverage'] == 'lines':
                cmd += ' --output=%s.pycoverage' % tf
            data.get(env).env.Append(IMP_TESTS=[(appname, filename)])
        if env.get('html_coverage', 'no') != 'no':
            cmd += ' --html_coverage=%s' \
                   % Dir("#/build/coverage").abspath
        if 'group' in env.get('html_coverage', 'no'):
            cmd += ' --global_coverage'
        cmd+=" --results="+filename
        #if len(fsource) > 0:
        #    env.Append(ENV={'TEST_DIRECTORY':fsource[0][0:fsource[0].find("/test/")+6]})
        #    #print "test dir", os.environ['TEST_DIRECTORY']
    elif type=='example':
        exname = _get_name(env)
        if exname == 'kernel':
            exname = 'IMP'
        elif not exname.startswith('IMP'):
            exname = 'IMP.' + exname
        exname += ' examples'
        cmd= File("#/scons_tools/run-all-examples.py").abspath
        dmod=[]
        for d in data.get(env).modules.keys():
            if not data.get(env).modules[d].ok:
                dmod.append(d)
        cmd= cmd+ ' --excluded='+":".join(dmod)
        cmd+=" --results="+filename
        data.get(env).env.Append(IMP_TESTS=[(exname, filename)])
    elif type=='system':
        sysname = _get_name(env)
        cmd= File("#/scons_tools/run-all-system.py").abspath + " " \
             + " --results=" + filename + " " + Dir("#/build/tmp").abspath
        data.get(env).env.Append(IMP_TESTS=[(sysname, filename)])
    else:
        utility.report_error(env, "Unknown test type "+type)
    app = "mkdir -p %s; cd %s; (%s %s %s%s %s >%s) > /dev/null" \
              % (tmpdir, tmpdir, source[0].abspath, env['PYTHON'],
                 cmd, disab,
                 " ".join(fsource), tf+".out")
    print app
    if execute(app) == 0:
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
    #testpy = "#/build/lib/IMP/test/__init__.py"
    dta= data.get(env)
    test=UnitTest(env, target="fast-test.results",
                  source=["#/tools/imppy.sh"]+source+[env.Value(type)])
    # bring in kernel and test to make sure kernel python support is there
    # and since examples don't tend to import test directly. test will pull
    # in kernel
    etest=UnitTest(env, target="test.results",
                   source=["#/tools/imppy.sh"]+source \
                          +expensive_source+[env.Value(type)])
    if "test" in dta.modules.keys():
        env.Depends(test, [dta.modules["test"].alias])
        env.Depends(etest, [dta.modules["test"].alias])
    if "kernel" in dta.modules.keys():
        env.Depends(test, [dta.modules["kernel"].alias])
        env.Depends(etest, [dta.modules["kernel"].alias])
    env.AlwaysBuild("fast-test.results")
    env.AlwaysBuild("test.results")
    #env.Requires(test, env.Alias(environment.get_current_name(env)))
    #env.Requires(test, "tools/imppy.sh")
    if type.endswith('unit test'):
        dta.add_to_alias(environment.get_current_name(env)+"-test-fast", test)
        data.get(env).add_to_alias(environment.get_current_name(env)+"-test", etest)
    elif type=='example':
        data.get(env).add_to_alias(environment.get_current_name(env)+"-test-examples",
                                               test)
    elif type=="system":
        data.get(env).add_to_alias(environment.get_current_name(env)+"-test", test)
    env.Alias(env.Alias('test'), etest)
    env.Alias(env.Alias('test-fast'), test)
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
        # Strip .exe extension, so test name on Windows matches other platforms
        if nm.endswith('_exe'):
            nm = nm[:-4]
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
       %{'name':nm, 'path':t.abspath, 'libdir':env.Dir('#/build/lib').abspath}
    print >> out, """
if __name__ == '__main__':
    IMP.test.main()"""


def _print_cpp_test(target, source, env):
    print "Generating cpp testing harness"

CPPTestHarness = Builder(action=Action(_action_cpp_test,
                                       _print_cpp_test))
