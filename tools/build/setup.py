#!/usr/bin/python

import glob
import os
import sys
import os.path
import shutil
import platform
import tools
from optparse import OptionParser

# main loops

# link all the headers from the module/include directories into the correct place in the build dir
def link_headers(source):
    target=os.path.join("include")
    tools.mkdir(target)
    root=os.path.join(target, "IMP")
    tools.mkdir(root)
    for (module, g) in tools.get_modules(source):
        #print g, module
        if module== "SConscript":
            continue
        tools.link_dir(os.path.join(g, "include"), os.path.join(root, module), match=["*.h"])
        tools.link_dir(os.path.join(g, "include", "internal"), os.path.join(root, module, "internal"),
                        match=["*.h"])

# link example scripts and data from the source dirs into the build tree
def link_examples(source):
    target=os.path.join("doc", "examples")
    tools.mkdir(target)
    for module, g in tools.get_modules(source):
        tools.link_dir(os.path.join(g, "examples"), os.path.join(target, module))

# link files from the module/data directries from the source into the build tree
def link_data(source):
    target=os.path.join("data")
    tools.mkdir(target)
    for module, g in tools.get_modules(source):
        tools.link_dir(os.path.join(g, "data"), os.path.join(target, module))

# link swig .i files from the source into the build tree
def link_swig(source):
    target=os.path.join("swig")
    tools.mkdir(target)
    for module, g in tools.get_modules(source):
        # they all go in the same dir, so don't remove old links
        tools.link_dir(os.path.join(g, "pyext"), target, match=["*.i"], clean=False)
        if os.path.exists(os.path.join(g, "pyext", "include")):
            tools.link_dir(os.path.join(g, "pyext", "include"), target, match=["*.i"], clean=False)

# link python source files from pyext/src into the build tree
def link_python(source):
    target=os.path.join("lib")
    tools.mkdir(target, clean=False)
    for module, g in tools.get_modules(source):
        path= os.path.join(target, "IMP", module)
        tools.mkdir(path, clean=False)
        for old in glob.glob(os.path.join(path, "*.py")):
            # don't unlink the generated file
            if os.path.split(old)[1] != "__init__.py" and os.path.split(old)[1] != "_version_check.py":
                os.unlink(old)
                #print "linking", path
        tools.link_dir(os.path.join(g, "pyext", "src"), path, clean=False)

# link all the dox files and other documentation related files from the source tree
# into the build tree
def link_dox(source):
    target=os.path.join("doxygen")
    tools.mkdir(target)
    for module, g in tools.get_modules(source):
        tools.link_dir(os.path.join(g, "doc"), os.path.join(target, module))
        tools.link_dir(os.path.join(g, "doc"), os.path.join("doc", "html"), match=["*.png", "*.pdf"],
                 clean=False)
    for bs, g in tools.get_biological_systems(source):
        tools.link_dir(g, os.path.join(target, bs))
        tools.link_dir(g, os.path.join("doc", "html"), match=["*.png", "*.pdf"], clean=False)
    for app, g in tools.get_applications(source):
        tools.link_dir(g, os.path.join(target, app))
        tools.link_dir(g, os.path.join("doc", "html"), match=["*.png", "*.pdf"], clean=False)
    tools.link_dir(os.path.join(source, "doc"), os.path.join(target, "IMP"))
    tools.link_dir(os.path.join(source, "doc"), os.path.join("doc", "html"), match=["*.png", "*.pdf"],
             clean=False)

def _make_test_driver(outf, cpps):
    out= open(outf, "w")
    print >> out, \
"""import IMP
import IMP.test
import sys
try:
    import subprocess
except ImportError:
    subprocess = None

class TestCppProgram(IMP.test.TestCase):"""
    for t in cpps:
        tbase= os.path.splitext(t)[0]
        # remove suffix
        nm= os.path.split(str(tbase))[1].replace(".", "_")
        # Strip .exe extension, so test name on Windows matches other platforms
        exename= os.path.join(os.path.split(outf)[0], os.path.split(tbase)[1])
        if platform.system=="Windows":
            exename=exename+".exe"
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
       %{'name':nm, 'path':os.path.abspath(exename), 'libdir':os.path.abspath("lib")}
    print >> out, """
if __name__ == '__main__':
    IMP.test.main()"""

def generate_tests(source):
    template="""import IMP
import IMP.test
import %(module)s

spelling_exceptions=%(spelling_exceptions)s

class StandardsTest(IMP.test.TestCase):
    def test_value_objects(self):
        "Test that module classes are either values or objects"
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
    target=os.path.join("test")
    tools.mkdir(target)
    for module, g in tools.get_modules(source):
        targetdir= os.path.join(target, module)
        tools.mkdir(targetdir)
        exceptions= os.path.join(g, "test", "standards_exceptions")
        plural_exceptions=[]
        show_exceptions=[]
        function_name_exceptions=[]
        value_object_exceptions=[]
        class_name_exceptions=[]
        spelling_exceptions=[]
        try:
            exec open(exceptions, "r").read()
        except:
            pass
        impmodule="IMP."+module
        test=template%({'module':impmodule,
                        'plural_exceptions':str(plural_exceptions),
                        'show_exceptions':str(show_exceptions),
                        'function_name_exceptions':str(function_name_exceptions),
                        'value_object_exceptions':str(value_object_exceptions),
                        'class_name_exceptions':str(class_name_exceptions),
                        'spelling_exceptions':str(spelling_exceptions)})
        open(os.path.join("test", module, "test_standards.py"), "w").write(test)

        cpptests= glob.glob(os.path.join(g, "test", "test_*.cpp"))
        ecpptests= glob.glob(os.path.join(g, "test", "expensive_test_*.cpp"))
        cppexamples= glob.glob(os.path.join(g, "examples", "*.cpp"))

        if len(cpptests)>0:
            _make_test_driver(os.path.join(targetdir, "test_cpp_tests.py"), cpptests)
        if len(ecpptests)>0:
            _make_test_driver(os.path.join(targetdir, "expensive_test_cpp_tests.py"), cpptests)
        if len(cppexamples)>0:
            _make_test_driver(os.path.join(targetdir, "cpp_examples_test.py"), cppexamples)


def generate_doxyfile(source):
    doxyin=os.path.join(source, "doc", "doxygen", "Doxyfile.in")
    version="develop"
    versionpath=os.path.join(source, "VERSION")
    if os.path.exists(versionpath):
        version= open(versionpath, "r").read().split('\n')[0].replace(" ", ":")
    # for building of modules without IMP
    if os.path.exists(doxyin):
        doxygen= open(doxyin, "r").read()
        doxygenr= doxygen.replace( "@IMP_SOURCE_PATH@", sys.argv[1]).replace("@VERSION@", version)
        doxygenrhtml= doxygenr.replace( "@IS_HTML@", "YES").replace("@IS_XML@", "NO")
        doxygenrxml= doxygenr.replace( "@IS_XML@", "YES").replace("@IS_HTML@", "NO")
        open(os.path.join("doxygen", "Doxyfile.html"), "w").write(doxygenrhtml)
        open(os.path.join("doxygen", "Doxyfile.xml"), "w").write(doxygenrxml)

# generate the pages that list biological systems and applications
def generate_overview_pages(source):
    ai= open(os.path.join("doxygen", "applications.dox"), "w")
    ai.write("/** \\page applications_index Application Index \n")
    for bs, g in tools.get_applications(source):
        ai.write("- \\ref application_%s \"%s\"\n"%(bs,bs))
    ai.write("*/")
    ai= open(os.path.join("doxygen", "biological_systems.dox"), "w")
    ai.write("/** \\page systems_index Biological Systems Index \n")
    ai.write("See \\ref biosys_sug_struct \"Biological Systems Suggested Structure\" for how we suggest them to be structured.\n")
    for app, g in tools.get_biological_systems(source):
        ai.write("- \\ref system_%s \"%s\"\n"%(app,app))
    ai.write("*/")


def generate_all_cpp(source):
    target=os.path.join("src")
    tools.mkdir(target)
    for module, g in tools.get_modules(source):
        sources= glob.glob(os.path.join(g, "src", "*.cpp"))\
            +glob.glob(os.path.join(g, "src", "internal", "*.cpp"))
        targetf=os.path.join(target, module+"_all.cpp")
        sources.sort()
        tools.rewrite(targetf, "\n".join(["#include <%s>"%os.path.abspath(s) for s in sources]))

def clean_pyc(dir):
    for root, dirnames, filenames in os.walk('.'):
        for d in dirnames:
            for f in glob.glob(os.path.join(d, "*.pyc")):
                os.unlink(f)


parser = OptionParser()
parser.add_option("-s", "--source", dest="source",
                  help="IMP source directory.")
parser.add_option("-d", "--datapath", dest="datapath",
                  help="Extra data path for IMP.")
parser.add_option("-m", "--disabled", dest="disabled",
                  help="Disabled modules.")

def main():
    (options, args) = parser.parse_args()
    clean_pyc(options.source)
    tools.mkdir(os.path.join("data", "build_info"))
    tools.rewrite(os.path.join("data", "build_info", "disabled"),
                  options.disabled.replace(":", "\n"))
    tools.setup_sorted_order(options.source,
                             options.datapath)
    link_headers(options.source)
    link_examples(options.source)
    link_dox(options.source)
    link_swig(options.source)
    link_python(options.source)
    link_data(options.source)
    generate_overview_pages(options.source)
    generate_doxyfile(options.source)
    generate_tests(options.source)

if __name__ == '__main__':
    main()
