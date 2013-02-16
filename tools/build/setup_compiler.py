#!/usr/bin/python

import glob
import os
import sys
import os.path
import shutil
import platform
import tools
from optparse import OptionParser



def get_flags(options, flagtypes):
    ret=[]
    import distutils.sysconfig
    #opt=' '.join([x for x in distutils.sysconfig.get_config_vars('OPT')])
    #cflags=' '.join([x for x in distutils.sysconfig.get_config_vars('BASECFLAGS')])
    #basecflags=[x for x in opt.split()+cflags.split() \
    #if x not in ['-Werror', '-Wall', '-Wextra',
    #'-O2', '-O3', '-O1', '-Os',
    #'-fstack-protector', '-Wstrict-prototypes',
    #'-g', '-dynamic', '-DNDEBUG',
    #"-fwrapv", "-fno-strict-aliasing"]]
    #ret=[x for x in basecflags if '_FORTIFY_SOURCE' not in x]
    ret=[]

    if options.compiler == "GNU":
        # "-Werror",  "-Wno-uninitialized"
        ret+=["-Wall", "-Wextra",  "-Wno-deprecated",
              "-Winit-self", "-Wstrict-aliasing=2",
              "-Wcast-align", "-fno-operator-names",
              "-Woverloaded-virtual", "-Wno-unknown-pragmas"]
        if float(options.version) >= 4.2:
            if sys.platform == 'darwin':
                ret+=["-Wmissing-prototypes"]
            else:
                ret+=["-Wmissing-declarations"]
        if float(options.version) >= 4.6:
            ret+=["-Wno-c++0x-compat"]
        if float(options.version) >= 4.3 and float(options.version) < 4.7:
            ret+=["-std=gnu++0x"]
        elif float(options.version) >= 4.7:
            ret+=["-std=c++11"]
        #if dependency.gcc.get_version(env)>= 4.3:
        #    env.Append(CXXFLAGS=["-Wunsafe-loop-optimizations"])
        # gcc 4.0 on Mac doesn't like -isystem, so we don't use it there.
        # But without -isystem, -Wundef throws up lots of Boost warnings.
        if sys.platform != 'darwin' or float(options.version) > 4.0:
            ret+=["-Wundef"]
        """if options.build == 'FAST':
            ret+=["-O3", "-fexpensive-optimizations",
                                 "-ffast-math", "-ftree-vectorize",
                                 '-ffinite-math-only',
                                 '-fstrict-aliasing',
                                 '-fno-trapping-math',
                                 '-fno-signaling-nans',
                                 '-fno-float-store',
                                 '-funsafe-loop-optimizations',
                                 '--param','inline-unit-growth=200',
                                 '-fearly-inlining',]
            if options.version:
                ret+=['-fno-signed-zeros',
                      '-freciprocal-math',
                      '-fassociative-math']
        elif options.build == 'RELEASE':
            ret+=["-O2", "-g"]
        elif options.build == 'COMPILE':
            pass
        elif options.build == 'DEBUG':
            ret+=["-g"]"""
    elif options.compiler == "Clang":
        # would be nice, but too much
        #ret+=["-Weverything"]
        # otherwise it whines about our nullptr support
        #ret+=["-Wno-c++98-compat", "-Wno-c++98-compat-pedantic"]
        # otherwise it whines padding in everything
        ret+=["-Wall", "-Wno-unknown-pragmas",
        "-Wno-padded",
        "-Wno-c++11-extensions",
        "-std=c++11",
        "-Wno-array-bounds",
        "-Wno-unused-label",
        "-Wno-missing-prototypes",
        "-Wno-missing-declarations",
        "-Wno-unused-function",
        "-Wno-self-assign",
        "-Wno-unused-value"]

        """if options.build == 'FAST':
            ret+=["-O3"]
        elif options.build == 'RELEASE':
            ret+=["-O2", "-g"]
        elif options.build == 'COMPILE':
            pass
        elif options.build == 'DEBUG':
            # gdb should break on __asan_report_error
            # can't use addresssanitizer at the moment
            # "-faddress-sanitizer"
            """
        if options.build=="Debug":
            ret+=["-fno-omit-frame-pointer",
                  "-fcatch-undefined-behavior"]

    if options.compiler == "GNU":
        if flagtypes == "python":
            ret = [x for x in ret
                   if x not in ['-Wall', '-Wextra', '-Wformat',
                                '-Wstrict-aliasing=2',
                                '-O3', '-O2',
                                "-Wmissing-prototypes",
                                "-Wmissing-declarations",
                                "-Wunused-function",]]
            ret.append("-Wno-deprecated-declarations")
        if flagtypes=="bin":
            ret = [x for x in ret if x not in ["-Wmissing-prototypes",
                                               "-Wmissing-declarations"]]
    elif options.compiler == "Clang":
        # just remove warning flags
        if flagtypes == "python":
            ret = [x for x in ret if x not in ["-Weverything", "-fcatch-undefined-behavior"]]
            ret.append("-Wno-deprecated-declarations")
        if flagtypes == "bin":
            ret = [x for x in ret if x not in ["-Wno-missing-prototypes", "-Wno-missing-declarations"]]

    return ret

parser = OptionParser()
parser.add_option("-c", "--compiler", dest="compiler", default="GNU",
                  help="One of GNU, Clang, cl.")
parser.add_option("-f", "--flags", dest="flags", default="",
                  help="Manually specified flags.")
parser.add_option("-v", "--version", dest="version", default="",
                  help="The compiler version.")
parser.add_option("-b", "--build", dest="build", default="",
                  help="The build type.")
parser.add_option("-o", "--output", dest="output", default="",
                  help="Which type to output: lib, bin, python.")



def main():
    (options, args) = parser.parse_args()
    if options.output not in ["bin", "python", "lib"]:
        print "bad output type"
        return 1
    print " ".join(get_flags(options, options.output)),

if __name__ == '__main__':
    main()
