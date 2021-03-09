#!/usr/bin/env python

"""
Set up the build environment:
- remove .pyc files from the source dir
- write the list of disabled modules (probably should go elsewhere)
- sort the modules based inter-module dependencies
- link module headers in the include dir
- link examples in the example dir
- link doxygen files into the doxygen dir
- link swig files into the swig dir
- link python source into the lib/IMP dir
- create pages that list things in the documentation
- setup the doxygen config files
- generate standards tests

No repository directories are changed.
"""

import os
import os.path
import platform
import tools
from optparse import OptionParser

# main loops


# link all the headers from the module/include directories into the
# correct place in the build dir
def link_headers(modules):
    target = os.path.join("include")
    tools.mkdir(target)
    root = os.path.join(target, "IMP")
    tools.mkdir(root)
    for module in modules:
        modroot = os.path.join(root,
                               '' if module.name == 'kernel' else module.name)
        tools.link_dir(os.path.join(module.path, "include"),
                       modroot, match=["*.h"])
        tools.link_dir(os.path.join(module.path, "include", "internal"),
                       os.path.join(modroot, "internal"), match=["*.h"])
        # ick
        if os.path.exists(os.path.join(module.path, "include", "eigen3")):
            tools.link_dir(os.path.join(module.path, "include", "eigen3"),
                           os.path.join(modroot, "eigen3"),
                           match=["*"])

# link example scripts and data from the source dirs into the build tree


def link_examples(modules):
    target = os.path.join("doc", "examples")
    tools.mkdir(target)
    for module in modules:
        tools.link_dir(os.path.join(module.path, "examples"),
                       os.path.join(target, module.name))

# link files from the module/data directries from the source into the
# build tree


def link_data(modules):
    target = os.path.join("data")
    tools.mkdir(target)
    for module in modules:
        tools.link_dir(os.path.join(module.path, "data"),
                       os.path.join(target, module.name))

# link swig .i files from the source into the build tree


def link_swig(modules):
    target = os.path.join("swig")
    tools.mkdir(target)
    for module in modules:
        # they all go in the same dir, so don't remove old links
        tools.link_dir(os.path.join(module.path, "pyext"), target,
                       match=["*.i"], clean=False)
        if os.path.exists(os.path.join(module.path, "pyext", "include")):
            tools.link_dir(os.path.join(module.path, "pyext", "include"),
                           target, match=["*.i"], clean=False)
        tools.link(os.path.join(module.path, "pyext", "swig.i-in"),
                   os.path.join(target, "IMP_%s.impl.i" % module.name))

# link python source files from pyext/src into the build tree


# Don't link __init__.py from the top-level of any module (it is generated
# either by SWIG or by make_python_init.py)
class _ExcludeTopLevelPythonInit(object):
    def __init__(self, topdir):
        self.pyinit = os.path.join(topdir, '__init__.py')

    def __call__(self, fname):
        return fname != self.pyinit


def link_python(modules):
    target = os.path.join("lib")
    tools.mkdir(target, clean=False)
    for module in modules:
        path = os.path.join(target, "IMP",
                            '' if module.name == 'kernel' else module.name)
        tools.mkdir(path, clean=False)
        for old in tools.get_glob([os.path.join(path, "*.py")]):
            # don't unlink the generated file
            if (os.path.split(old)[1] != "__init__.py"
                    and os.path.split(old)[1] != "_version_check.py"):
                os.unlink(old)
                # print "linking", path
        topdir = os.path.join(module.path, "pyext", "src")
        tools.link_dir(topdir, path, clean=False, make_subdirs=True,
                       filt=_ExcludeTopLevelPythonInit(topdir))


def _make_test_driver(outf, cpps):
    out = open(outf, "w")
    out.write("""import IMP
import IMP.test
import sys
try:
    import subprocess
except ImportError:
    subprocess = None

class TestCppProgram(IMP.test.TestCase):
""")
    for t in cpps:
        tbase = os.path.splitext(t)[0]
        # remove suffix
        nm = os.path.split(str(tbase))[1].replace(".", "_")
        # Strip .exe extension, so test name on Windows matches other platforms
        exename = os.path.join(os.path.split(outf)[0], os.path.split(tbase)[1])
        if platform.system == "Windows":
            exename = exename + ".exe"
        out.write("""    def test_%(name)s(self):
        \"\"\"Running C++ test %(name)s\"\"\"
        if subprocess is None:
            self.skipTest("subprocess module unavailable")
        # Note: Windows binaries look for needed DLLs in the current
        # directory. So we need to change into the directory where the DLLs
        # have been installed for the binary to load correctly.
        p = subprocess.Popen(["%(path)s"],
                             shell=False, cwd="%(libdir)s")
        self.assertEqual(p.wait(), 0)
"""
                  % {'name': nm, 'path': os.path.abspath(exename),
                     'libdir': os.path.abspath("lib")})
    out.write("""
if __name__ == '__main__':
    IMP.test.main()
""")


def generate_tests(modules, scons):
    template = """import IMP
import IMP.test
import %(module)s

spelling_exceptions=%(spelling_exceptions)s
python_only=%(python_only)s

class StandardsTest(IMP.test.TestCase):
    def test_value_objects(self):
        "Test that module classes are either values or objects"
        if python_only:
            self.skipTest("this module is Python-only")
        exceptions= %(value_object_exceptions)s
        return self.assertValueObjects(%(module)s,exceptions)
    def test_classes(self):
        "Test that module class names follow the standards"
        exceptions=%(value_object_exceptions)s
        return self.assertClassNames(%(module)s, exceptions,
                                     spelling_exceptions)
    def test_functions(self):
        "Test that module function names follow the standards"
        exceptions= %(function_name_exceptions)s
        return self.assertFunctionNames(%(module)s, exceptions,
                                        spelling_exceptions)
    def test_show(self):
        "Test all objects have show"
        exceptions=%(show_exceptions)s
        return self.assertShow(%(module)s, exceptions)

if __name__ == '__main__':
    IMP.test.main()
    """
    target = os.path.join("test")
    tools.mkdir(target)
    for module in modules:
        targetdir = os.path.join(target, module.name)
        tools.mkdir(targetdir)
        exceptions = os.path.join(module.path, "test", "standards_exceptions")
        d = {'plural_exceptions': [],
             'show_exceptions': [],
             'function_name_exceptions': [],
             'value_object_exceptions': [],
             'class_name_exceptions': [],
             'spelling_exceptions': []}
        try:
            with open(exceptions, "r") as fh:
                exec(fh.read(), d)
        except IOError:
            pass
        impmodule = "IMP" if module.name == 'kernel' else "IMP." + module.name
        test = template % ({'module': impmodule,
                            'plural_exceptions': str(d['plural_exceptions']),
                            'show_exceptions': str(d['show_exceptions']),
                            'python_only': 'True' if module.python_only
                                           else 'False',
                            'function_name_exceptions':
                                str(d['function_name_exceptions']),
                            'value_object_exceptions':
                                str(d['value_object_exceptions']),
                            'class_name_exceptions':
                                str(d['class_name_exceptions']),
                            'spelling_exceptions':
                                str(d['spelling_exceptions'])})
        gen = tools.PythonFileGenerator()
        gen.write(os.path.join("test", module.name,
                               "medium_test_standards.py"),
                  test, show_diff=False)

        cpptests = tools.get_glob([os.path.join(module.path, "test",
                                                "test_*.cpp")])
        ecpptests = tools.get_glob(
            [os.path.join(module.path, "test", "expensive_test_*.cpp")])
        cppexamples = tools.get_glob([os.path.join(module.path, "examples",
                                                   "*.cpp")])

        if len(cpptests) > 0 and scons:
            _make_test_driver(
                os.path.join(
                    targetdir,
                    "test_cpp_tests.py"),
                cpptests)
        if len(ecpptests) > 0 and scons:
            _make_test_driver(
                os.path.join(targetdir,
                             "expensive_test_cpp_tests.py"),
                cpptests)
        if len(cppexamples) > 0 and scons:
            _make_test_driver(
                os.path.join(targetdir,
                             "cpp_examples_test.py"),
                cppexamples)


def clean_pyc(dir):
    for root, dirnames, filenames in os.walk('.'):
        for d in dirnames:
            for f in tools.get_glob([os.path.join(d, "*.pyc")]):
                os.unlink(f)


def generate_src_dirs(modules):
    """Make src directories for each module. This way we don't have to
       worry about whether it exists later."""
    for module in modules:
        tools.mkdir(os.path.join("src", module.name), clean=False)


parser = OptionParser()
parser.add_option("--build_dir", help="IMP build directory", default=None)
parser.add_option("--module_name", help="Module name", default=None)
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")
parser.add_option("-d", "--datapath", dest="datapath",
                  help="Extra data path for IMP.")
parser.add_option("-m", "--disabled", dest="disabled",
                  help="Disabled modules.")
parser.add_option("--scons", default=False, action="store_true",
                  help="Set if we are running scons.")


def main():
    (options, args) = parser.parse_args()
    mf = tools.ModulesFinder(source_dir=options.source,
                             external_dir=options.build_dir,
                             module_name=options.module_name)
    all_modules = [x for x in mf.values() if isinstance(x, tools.SourceModule)]
    clean_pyc(options.source)
    tools.mkdir(os.path.join("build_info"))
    tools.mkdir(os.path.join("cmake_tests"))
    tools.rewrite(os.path.join("build_info", "disabled"),
                  options.disabled.replace(":", "\n"))
    tools.set_sorted_order([m.name for m in mf.get_ordered()])
    link_headers(all_modules)
    link_examples(all_modules)
    link_swig(all_modules)
    link_python(all_modules)
    link_data(all_modules)
    generate_tests(all_modules, options.scons)
    generate_src_dirs(all_modules)


if __name__ == '__main__':
    main()
