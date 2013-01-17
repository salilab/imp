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
    relpath=os.path.relpath(source, target)
    #print source, target, relpath
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


def link_dir(source_dir, target_dir, match="*", clean=True):
    #print "linking", source_dir, target_dir
    mkdir(target_dir, clean=clean)
    for g in glob.glob(os.path.join(source_dir, match)):
        name=os.path.split(g)[1]
        if name != "SConscript":
            #print g, name, os.path.join(target_dir, name)
            link(g, os.path.join(target_dir, name))

def get_modules(source):
    path=os.path.join(source, "modules", "*")
    globs=glob.glob(path)
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")]


# main loops

def _make_all_header(source, module, filename):
    f= open(filename, "w")
    if module=="kernel":
        includepath="IMP/"
    else:
        includepath="IMP/"+module+"/"
    for h in glob.glob(os.path.join(source, "modules", module, "include", "*.h")):
        name= os.path.split(h)[1]
        f.write("#include <"+includepath+name+">\n")

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
            link_dir(os.path.join(g, "include"), root, match="*.h")
            link_dir(os.path.join(g, "include", "internal"), os.path.join(root, "internal"),
                     match="*.h")
            _make_all_header(source, "kernel", os.path.join("build", "include", "IMP.h"))
        else:
            link_dir(os.path.join(g, "include"), os.path.join(root, module), match="*.h")
            link_dir(os.path.join(g, "include", "internal"), os.path.join(root, module, "internal"),
                     match="*.h")
            _make_all_header(source, module, os.path.join("build", "include", "IMP", module+".h"))


def link_examples(source):
    target=os.path.join("build", "doc", "examples")
    mkdir(target)
    for module, g in get_modules(source):
        link_dir(os.path.join(g, "examples"), os.path.join(target, module))

def link_swig(source):
    target=os.path.join("build", "swig")
    mkdir(target)
    for module, g in get_modules(source):
        # they all go in the same dir, so don't remove old links
        link_dir(os.path.join(g, "pyext"), target, "*.i", clean=False)

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
            os.unlink(old)
        link_dir(os.path.join(g, "pyext", "src"), path)

def link_dox(source):
    target=os.path.join("build", "doxygen")
    mkdir(target)
    for module, g in get_modules(source):
        link_dir(os.path.join(g, "doc"), os.path.join(target, module))
    link_dir(os.path.join(source, "doc"), os.path.join(target, "IMP"))

def make_doxygen(source):
    doxygen= open(os.path.join(source, "doc", "doxygen", "Doxyfile.in"), "r").read()
    doxygenr= doxygen.replace( "@IMP_SOURCE_PATH@", sys.argv[1])
    open(os.path.join("build", "doxygen", "Doxyfile"), "w").write(doxygenr)

def main():
    source=sys.argv[1]
    link_headers(source)
    link_examples(source)
    link_dox(source)
    link_swig(source)
    link_python(source)
    make_doxygen(source)

if __name__ == '__main__':
    main()
