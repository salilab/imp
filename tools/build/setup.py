#!/usr/bin/python

import glob
import os
import sys
import os.path
import shutil

# helper functions

def mkdir(path, clean=True):
    if os.path.isdir(path):
        # remove any old links
        if clean:
            for f in glob.glob(os.path.join(path, "*")):
                if os.path.islink(f):
                    os.unlink(f)
        return
    if os.path.isfile(path):
        os.unlink(path)
    os.makedirs(path)

def link(source, target):
    # TODO make it copy the file on windows
    tpath= os.path.abspath(target)
    spath= os.path.abspath(source)
    #print tpath, spath
    if not os.path.exists(source):
        return
    if os.path.islink(tpath):
        return
    if os.path.isdir(tpath):
        shutil.rmtree(tpath)
    if os.path.exists(tpath):
        os.unlink(tpath)
    os.symlink(spath, tpath)


def link_dir(source_dir, target_dir, match=["*"], clean=True):
    if type(match) != list:
        adkfjads;lkfjd;laskjfdl;k
    #print "linking", source_dir, target_dir
    mkdir(target_dir, clean=clean)
    files=[]
    for m in match:
        files.extend(glob.glob(os.path.join(source_dir, m)))
    for g in files:
        name=os.path.split(g)[1]
        if name != "SConscript":
            #print g, name, os.path.join(target_dir, name)
            link(g, os.path.join(target_dir, name))

def get_modules(source):
    path=os.path.join(source, "modules", "*")
    globs=glob.glob(path)
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")]

def get_biological_systems(source):
    path=os.path.join(source, "biological_systems", "*")
    globs=glob.glob(path)
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")]

def get_applications(source):
    path=os.path.join(source, "applications", "*")
    globs=glob.glob(path)
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")]

# main loops

def _make_all_header(source, module, filename):
    f= open(filename, "w")
    if module=="kernel":
        includepath="IMP/"
    else:
        includepath="IMP/"+module+"/"
    headers=glob.glob(os.path.join(source, "modules", module, "include", "*.h"))
    headers.sort()
    for h in headers:
        name= os.path.split(h)[1]
        f.write("#include <"+includepath+name+">\n")

# link all the headers from the module/include directories into the correct place in the build dir
def link_headers(source):
    target=os.path.join("build", "include")
    mkdir(target)
    root=os.path.join(target, "IMP")
    mkdir(root)
    for (module, g) in get_modules(source):
        #print g, module
        if module== "SConscript":
            continue
        if module=="kernel":
            link_dir(os.path.join(g, "include"), root, match=["*.h"])
            link_dir(os.path.join(g, "include", "internal"), os.path.join(root, "internal"),
                     match=["*.h"])
            _make_all_header(source, "kernel", os.path.join("build", "include", "IMP.h"))
        else:
            link_dir(os.path.join(g, "include"), os.path.join(root, module), match=["*.h"])
            link_dir(os.path.join(g, "include", "internal"), os.path.join(root, module, "internal"),
                     match=["*.h"])
            _make_all_header(source, module, os.path.join("build", "include", "IMP", module+".h"))

# link example scripts and data from the source dirs into the build tree
def link_examples(source):
    target=os.path.join("build", "doc", "examples")
    mkdir(target)
    for module, g in get_modules(source):
        link_dir(os.path.join(g, "examples"), os.path.join(target, module))

# link files from the module/data directries from the source into the build tree
def link_data(source):
    target=os.path.join("build", "data")
    mkdir(target)
    for module, g in get_modules(source):
        link_dir(os.path.join(g, "data"), os.path.join(target, module))

# link swig .i files from the source into the build tree
def link_swig(source):
    target=os.path.join("build", "swig")
    mkdir(target)
    for module, g in get_modules(source):
        # they all go in the same dir, so don't remove old links
        link_dir(os.path.join(g, "pyext"), target, match=["*.i"], clean=False)
        if os.path.exists(os.path.join(g, "pyext", "include")):
            link_dir(os.path.join(g, "pyext", "include"), target, match=["*.i"], clean=False)

# link python source files from pyext/src into the build tree
def link_python(source):
    target=os.path.join("build", "lib")
    mkdir(target)
    for module, g in get_modules(source):
        if module=="kernel":
            modulepath="IMP"
        else:
            modulepath= os.path.join("IMP", module)
        path= os.path.join(target, modulepath)
        mkdir(path)
        for old in glob.glob(os.path.join(path, "*.py")):
            # don't unlink the generated file
            if os.path.split(old)[1] != "__init__.py":
                os.unlink(old)
        link_dir(os.path.join(g, "pyext", "src"), path)

# link all the dox files and other documentation related files from the source tree
# into the build tree
def link_dox(source):
    target=os.path.join("build", "doxygen")
    mkdir(target)
    for module, g in get_modules(source):
        link_dir(os.path.join(g, "doc"), os.path.join(target, module))
        link_dir(os.path.join(g, "doc"), os.path.join("build", "doc", "html"), match=["*.png", "*.pdf"],
                 clean=False)
    for bs, g in get_biological_systems(source):
        link_dir(g, os.path.join(target, bs))
        link_dir(g, os.path.join("build", "doc", "html"), match=["*.png", "*.pdf"], clean=False)
    for app, g in get_applications(source):
        link_dir(g, os.path.join(target, app))
        link_dir(g, os.path.join("build", "doc", "html"), match=["*.png", "*.pdf"], clean=False)
    link_dir(os.path.join(source, "doc"), os.path.join(target, "IMP"))
    link_dir(os.path.join(source, "doc"), os.path.join("build", "doc", "html"), match=["*.png", "*.pdf"],
             clean=False)

def generate_standards_tests(source):
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
    target=os.path.join("build", "test")
    mkdir(target)
    for module, g in get_modules(source):
        targetdir= os.path.join(target, module)
        mkdir(targetdir)
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
        if module=="kernel":
            impmodule="IMP"
        else:
            impmodule="IMP."+module
        test=template%({'module':impmodule,
                        'plural_exceptions':str(plural_exceptions),
                        'show_exceptions':str(show_exceptions),
                        'function_name_exceptions':str(function_name_exceptions),
                        'value_object_exceptions':str(value_object_exceptions),
                        'class_name_exceptions':str(class_name_exceptions),
                        'spelling_exceptions':str(spelling_exceptions)})
        open(os.path.join("build", "test", module, "test_standards.py"), "w").write(test)


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
        open(os.path.join("build", "doxygen", "Doxyfile.html"), "w").write(doxygenrhtml)
        open(os.path.join("build", "doxygen", "Doxyfile.xml"), "w").write(doxygenrxml)

# generate the pages that list biological systems and applications
def generate_overview_pages(source):
    ai= open("build/doxygen/applications.dox", "w")
    ai.write("/** \\page applications_index Application Index \n")
    for bs, g in get_applications(source):
        ai.write("- \\ref application_%s \"%s\"\n"%(bs,bs))
    ai.write("*/")
    ai= open("build/doxygen/biological_systems.dox", "w")
    ai.write("/** \\page systems_index Biological Systems Index \n")
    ai.write("See \\ref biosys_sug_struct \"Biological Systems Suggested Structure\" for how we suggest them to be structured.\n")
    for app, g in get_biological_systems(source):
        ai.write("- \\ref system_%s \"%s\"\n"%(app,app))
    ai.write("*/")


def main():
    source=sys.argv[1]
    link_headers(source)
    link_examples(source)
    link_dox(source)
    link_swig(source)
    link_python(source)
    link_data(source)
    generate_overview_pages(source)
    generate_doxyfile(source)
    generate_standards_tests(source)

if __name__ == '__main__':
    main()
