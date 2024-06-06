#!/usr/bin/env python

""" Run swig and patch the output.
"""

import tools
from argparse import ArgumentParser
import os.path

parser = ArgumentParser()
parser.add_argument("--include", help="Extra header include path",
                    default=None)
parser.add_argument("--swig_include", help="Extra SWIG include path",
                    default=[], action="append")
parser.add_argument("-s", "--swig", dest="swig", default="swig",
                    help="Swig command to use.")
parser.add_argument("-m", "--module",
                    dest="module", help="Module to run.")
parser.add_argument("-p", "--swigpath", dest="swigpath", action="append",
                    default=[], help="Module to run.")
parser.add_argument("-i", "--includepath", dest="includepath", action="append",
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
    for p in options.swig_include:
        args.append("-I%s" % tools.from_cmake_path(p))
    args.extend(["-oh", "wrap.h-in"])
    args.extend(["-o", "wrap.cpp-in"])
    if options.module == "kernel":
        args.append("-DIMP_SWIG_KERNEL")
    for p in options.swigpath:
        args.append("-I%s" % tools.from_cmake_path(p))
    for p in options.includepath:
        args.append("-I%s" % tools.from_cmake_path(p))
    args.append(os.path.abspath("./swig/IMP_%s.i" % options.module))

    _ = tools.run_subprocess(args, cwd=outputdir)
    patch_py_wrapper("src/%s_swig/IMP.%spy"
                     % (options.module, '' if options.module == 'kernel'
                                        else options.module + '.'),
                     os.path.join(
                        "lib", "IMP",
                        "" if options.module == 'kernel' else options.module,
                        "__init__.py"),
                     options.module)


def patch_py_wrapper(infile, outfile, module):
    """Patch Python wrappers.
       Work around SWIG bugs."""
    # outfile might be a symlink
    if os.path.exists(outfile):
        os.unlink(outfile)
    outfh = open(outfile, "w")
    with open(infile) as infh:
        for line in infh:
            if module == 'kernel':
                # Work around SWIG 3.0.8 bug
                # https://github.com/swig/swig/issues/583
                line = line.replace('except Exception:', 'except:')
                # List wrappers use IMP._list_util for all modules
                # *except* kernel
                line = line.replace('IMP._list_util', '_list_util')
            outfh.write(line)
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
# 3. Handle our custom %ifdelete directive
#    (see modules/kernel/pyext/include/IMP_kernel.exceptions.i)
def patch_file(infile, out, options):
    with open(infile, 'r') as fh:
        lines = fh.readlines()
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
        if line.startswith('#ifdelete'):
            preproc, symname = line.split()
            if symname.startswith('delete_'):
                line = '#if 1\n'
            else:
                line = '#if 0\n'
        lines[i] = line
    tools.rewrite(out, "".join(lines), verbose=False)


def main():
    args = parser.parse_args()
    outputdir = os.path.abspath(
        os.path.join(
            "src",
            "%s_swig" %
            args.module))
    tools.mkdir(outputdir, clean=False)
    run_swig(outputdir, args)
    patch_file(os.path.join(outputdir, "wrap.cpp-in"),
               os.path.join(outputdir, "wrap.cpp"), args)
    patch_file(os.path.join(outputdir, "wrap.h-in"),
               os.path.join(outputdir, "wrap.h"), args)


if __name__ == '__main__':
    main()
