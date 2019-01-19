#!/usr/bin/env python

""" Run swig and patch the output.
"""

import tools
from optparse import OptionParser
import os.path
import subprocess

parser = OptionParser()
parser.add_option("--include", help="Extra header include path", default=None)
parser.add_option("--swig_include", help="Extra SWIG include path",
                  default=None)
parser.add_option("-s", "--swig", dest="swig", default="swig",
                  help="Swig command to use.")
parser.add_option("-m", "--module",
                  dest="module", help="Module to run.")
parser.add_option("-p", "--swigpath", dest="swigpath", action="append",
                  default=[], help="Module to run.")
parser.add_option("-i", "--includepath", dest="includepath", action="append",
                  default=[], help="Module to run.")


def run_swig(outputdir, options):
    args = [options.swig]
    args.append("-castmode")
    args.extend(["-interface", "_IMP_%s" % options.module])
    args.append(
        "-DPySwigIterator=IMP_%s_PySwigIterator" %
        options.module.upper())
    args.append(
        "-DSwigPyIterator=IMP_%s_SwigPyIterator" %
        options.module.upper())
    args.append("-python")
    args.append("-c++")
    args.append("-naturalvar")
    args.append("-modern")
    args.append("-fvirtual")
    args.append("-Wextra")
    args.append("-I" + os.path.join("..", "..", "include"))
    args.append("-I" + os.path.join("..", "..", "swig"))
    if options.include:
        args.append("-I" + options.include)
    if options.swig_include:
        args.append("-I" + options.swig_include)
    args.extend(["-oh", "wrap.h-in"])
    args.extend(["-o", "wrap.cpp-in"])
    if options.module == "kernel":
        args.append("-DIMP_SWIG_KERNEL")
    for p in options.swigpath:
        args.append("-I%s" % tools.from_cmake_path(p))
    for p in options.includepath:
        args.append("-I%s" % tools.from_cmake_path(p))
    args.append(os.path.abspath("./swig/IMP_%s.i" % options.module))

    ret = tools.run_subprocess(args, cwd=outputdir)
    patch_py_wrapper("src/%s_swig/IMP.%spy"
                     % (options.module, '' if options.module == 'kernel'
                                        else options.module + '.'),
                     os.path.join("lib", "IMP",
                          "" if options.module == 'kernel' else options.module,
                          "__init__.py"),
                     options.module)

def patch_py_wrapper(infile, outfile, module):
    """Patch Python wrappers.
       Work around SWIG bugs. Also add custom header; this is more properly
       done by SWIG's %pythonbegin directive, but only very recent versions
       of SWIG understand that."""
    # outfile might be a symlink
    if os.path.exists(outfile):
        os.unlink(outfile)
    outfh = open(outfile, "w")
    in_initial_comment = True
    header = """# This wrapper is part of IMP,
# Copyright 2007-2019 IMP Inventors. All rights reserved.

from __future__ import print_function, division, absolute_import
"""
    with open(infile) as infh:
        for line in infh:
            if module == 'kernel':
                # Work around SWIG 3.0.8 bug
                # https://github.com/swig/swig/issues/583
                line = line.replace('except Exception:', 'except:')
            outfh.write(line)
            if in_initial_comment and not line.startswith('#'):
                in_initial_comment = False
                outfh.write(header)
        if in_initial_comment:
            raise IOError("Empty SWIG wrapper file")
    outfh.close()

# 1. Workaround for SWIG bug #1863647: Ensure that the PySwigIterator class
#    (SwigPyIterator in 1.3.38 or later) is renamed with a module-specific
#    prefix, to avoid collisions when using multiple modules
# 2. If module names contain '.' characters, SWIG emits these into the CPP
#    macros used in the director header. Work around this by replacing them
#    with '_'. A longer term fix is not to call our modules "IMP.foo" but
#    to say %module(package=IMP) foo but this doesn't work in SWIG stable
#    as of 1.3.36 (Python imports incorrectly come out as 'import foo'
# rather than 'import IMP.foo'). See also IMP bug #41 at
#    https://salilab.org/imp/bugs/show_bug.cgi?id=41
def patch_file(infile, out, options):
    lines = open(infile, 'r').readlines()
    preproc = "IMP_%s" % options.module.upper()
    repl1 = '"swig::%s_PySwigIterator *"' % preproc
    repl2 = '"swig::%s_SwigPyIterator *"' % preproc
    orig = 'SWIG_IMP.%s_WRAP_H_' % options.module
    repl = 'SWIG_IMP_%s_WRAP_H_' % options.module
    for i in range(len(lines)):
        line = lines[i]
        line = line.replace('"swig::PySwigIterator *"', repl1)
        line = line.replace('"swig::SwigPyIterator *"', repl2)
        line = line.replace(orig, repl)
        line = line.replace("wrap.h-in", "wrap.h")
        # for some reason swig has issues with directors and VersionInfo
        # when %extend is used
        line = line.replace(" VersionInfo ", " IMP::VersionInfo ")
        line = line.replace("(VersionInfo ", "(IMP::VersionInfo ")
        line = line.replace("<VersionInfo ", "<IMP::VersionInfo ")
        line = line.replace("<:", "< :")  # swig generates bad C++ code
        lines[i] = line
    tools.rewrite(out, "".join(lines), verbose=False)


def main():
    (options, args) = parser.parse_args()
    outputdir = os.path.abspath(
        os.path.join(
            "src",
            "%s_swig" %
            options.module))
    tools.mkdir(outputdir, clean=False)
    run_swig(outputdir, options)
    patch_file(os.path.join(outputdir, "wrap.cpp-in"),
               os.path.join(outputdir, "wrap.cpp"), options)
    patch_file(os.path.join(outputdir, "wrap.h-in"),
               os.path.join(outputdir, "wrap.h"), options)

if __name__ == '__main__':
    main()
