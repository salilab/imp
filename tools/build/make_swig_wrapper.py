#!/usr/bin/python

import tools
from optparse import OptionParser
import os.path

parser = OptionParser()
parser.add_option("-s", "--swig", dest="swig", default="swig",
                  help="Swig command to use.")
parser.add_option("-m", "--module",
                  dest="module", help="Module to run.")
parser.add_option("-p", "--swigpath",
                  dest="swigpath", default="", help="Module to run.")

def run_swig(outputdir, options):
    flags=[]
    flags.append("-castmode")
    flags.append("-interface _IMP_%s"%options.module)
    flags.append("-DPySwigIterator=IMP_%s_PySwigIterator"%options.module.upper())
    flags.append("-DSwigPyIterator=IMP_%s_SwigPyIterator"%options.module.upper())
    flags.append("-python")
    flags.append("-c++")
    flags.append("-naturalvar")
    flags.append("-fvirtual")
    flags.append("-Wextra")
    flags.append("-I../../include")
    flags.append("-I../../swig")
    flags.append("-oh wrap.h-in")
    flags.append("-o wrap.cpp-in")
    if options.module=="base":
        flags.append("-DIMP_SWIG_BASE")
    for p in tools.split(options.swigpath):
        flags.append("-I%s"%p)
    flags.append(os.path.abspath("./swig/IMP_%s.i"%options.module))

    cmd="cd %s; "%outputdir + options.swig+ " " + " ".join(flags)
    print cmd
    os.system(cmd)
    if len(open("src/%s_swig/IMP.%s.py"%(options.module, options.module), "r").read()) < 10:
        raise IOError("Empty swig wrapper file")
    tools.link("src/%s_swig/IMP.%s.py"%(options.module, options.module),
                "lib/IMP/%s/__init__.py"%options.module, verbose=True)


# 1. Workaround for SWIG bug #1863647: Ensure that the PySwigIterator class
#    (SwigPyIterator in 1.3.38 or later) is renamed with a module-speci\fic
#    prefix, to avoid collisions when using multiple modules
# 2. If module names contain '.' characters, SWIG emits these into the CPP
#    macros used in the director header. Work around this by replacing them
#    with '_'. A longer term fix is not to call our modules "IMP.foo" but
#    to say %module(package=IMP) foo but this doesn't work in SWIG stable
#    as of 1.3.36 (Python imports incorrectly come out as 'import foo'
#    rather than 'import IMP.foo'). See also IMP bug #41 at
#    https://salilab.org/imp/bugs/show_bug.cgi?id=41
def patch_file(infile, out, options):
    lines = file(infile, 'r').readlines()
    preproc= "IMP_%s"%options.module.upper()
    repl1 = '"swig::%s_PySwigIterator *"' % preproc
    repl2 = '"swig::%s_SwigPyIterator *"' % preproc
    orig = 'SWIG_IMP.%s_WRAP_H_' % options.module
    repl = 'SWIG_IMP_%s_WRAP_H_' % options.module
    for i in range(len(lines)):
        line= lines[i]
        line = line.replace('"swig::PySwigIterator *"', repl1)
        line = line.replace('"swig::SwigPyIterator *"', repl2)
        line = line.replace(orig, repl)
        line = line.replace("wrap.h-in", "wrap.h")
        # for some reason swig has issues with directors and VersionInfo
        # when %extend is used
        line = line.replace(" VersionInfo ", " IMP::VersionInfo ")
        line = line.replace("(VersionInfo ", "(IMP::VersionInfo ")
        line = line.replace("<VersionInfo ", "<IMP::VersionInfo ")
        line = line.replace("<:", "< :") # swig generates bad C++ code
        lines[i]=line
    tools.rewrite(out, "".join(lines))

def main():
    (options, args) = parser.parse_args()
    outputdir= os.path.abspath(os.path.join("src", "%s_swig"%options.module))
    tools.mkdir(outputdir, clean=False)
    run_swig(outputdir, options)
    patch_file(os.path.join(outputdir, "wrap.cpp-in"),
               os.path.join(outputdir, "wrap.cpp"), options)
    patch_file(os.path.join(outputdir, "wrap.h-in"),
               os.path.join(outputdir, "wrap.h"), options)

if __name__ == '__main__':
    main()
