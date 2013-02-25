#!/usr/bin/python

"""Build as many IMP components (modules, applications) as possible, even
   if some of them fail."""

import tools
import time
import os
import sys
import optparse
import subprocess
try:
    import cPickle as pickle
except ImportError:
    import pickle

class Component(object):
    """Represent an IMP application or module"""
    def __init__(self, name):
        self.name = name
        self.done = False
        self.build_result = 'notdone'
        self.build_time = 0.
        self.dep_failure = None
    def set_dep_modules(self, comps, modules):
        self.modules = [comps[m] for m in modules]
    def try_build(self, builder):
        if self.done:
            return False
        for m in self.modules:
            if not m.done:
                return False
            elif m.build_result != 0:
                # Propagate the original failure
                self.dep_failure = m.dep_failure or m
                print "%s: skipped due to previous failure to build %s" \
                      % (self.name, self.dep_failure.name)
                self.build_result = 'depfail'
                self.done = True
                return True
        # All dependencies built OK (or no dependencies), so we can build too
        self.build_result, self.build_time = builder.build(self)
        self.done = True
        return True
    def get_build_summary(self):
        return {'result': self.build_result,
                'time': self.build_time}


class Builder(object):
    def __init__(self, makecmd, outdir):
        self.makecmd = makecmd
        self.outdir = outdir

    def build(self, component):
        cmd = "%s IMP.%s" % (self.makecmd, component.name)
        if self.outdir:
            outfile = os.path.join(self.outdir, component.name)
            print "%s > %s" % (cmd, outfile)
            outfh = open(outfile, 'w')
            errfh = subprocess.STDOUT
        else:
            print cmd
            outfh = errfh = None
        starttime = time.time()
        ret = subprocess.call(cmd, shell=True, stdout=outfh, stderr=errfh)
        endtime = time.time()
        if ret != 0:
            print "%s: FAILED with exit code %d" % (component.name, ret)
        return (ret, endtime - starttime)


def get_all_components():
    comps = {}

    modules = tools.get_sorted_order()
    apps = tools.get_all_configured_applications()
    for m in modules:
        comps[m] = Component(m)
    for a in apps:
        comps[a] = Component(a)

    for m in modules:
        i = tools.get_module_info(m, "")
        comps[m].set_dep_modules(comps, i['modules'])
    for a in apps:
        i = tools.get_application_info(a, "")
        comps[a].set_dep_modules(comps, i['modules'])
    return comps

def write_summary_file(fh, comps):
    summary = {}
    for m in comps.values():
        summary[m.name] = m.get_build_summary()
    pickle.dump(summary, fh, -1)

def build_all(builder, opts):
    comps = get_all_components()

    while True:
        built = 0
        for m in comps.values():
            if m.try_build(builder):
                built += 1
                if opts.summary:
                    write_summary_file(open(opts.summary, 'w'), comps)
        if built == 0:
            break
    # If a component didn't build, there must be a dependency problem somewhere
    for m in comps.values():
        if not m.done:
            print "%s: did not build (circular dependency?)" % m.name
            m.build_result = 'circdep'
    if opts.summary:
        write_summary_file(open(opts.summary, 'w'), comps)
    for m in comps.values():
        if m.build_result != 0:
            sys.exit(1)

def parse_args():
    from optparse import OptionParser
    usage = """%prog [options] makecmd

Build all components (modules, applications) using the given makecmd
(e.g. "make", "ninja", "make -j8").

This is similar to just running the makecmd itself, but will build as many
components as possible, rather than stopping at the first failure. Build output
can also be sent to separate files for each component (--outdir).

Exit value is 0 if all components built successfully, 1 otherwise.
"""
    parser = OptionParser(usage=usage)
    parser.add_option("--summary",
                      default=None,
                      help="Dump summmary info as a Python pickle to the "
                           "named file. For each component, the time taken to "
                           "run makecmd is recorded, plus the build result, "
                           "which is either the return value of makecmd, or "
                           "'circdep' (the component was not built due to a "
                           "dependency problem), 'depfail' (not built because "
                           "a dependency failed to build), or 'notdone' (the "
                           "build hasn't started yet). The summary info is "
                           "updated after each component build.")
    parser.add_option("--outdir",
                      default=None,
                      help="Direct build output to the given directory; one "
                           "file for each component is generated in the "
                           "directory. If not given, output is sent to "
                           "standard output.")
    opts, args = parser.parse_args()
    if len(args) != 1:
        parser.error("incorrect number of arguments")
    return opts, args

def main():
    opts, args = parse_args()
    build_all(Builder(args[0], opts.outdir), opts)

if __name__ == '__main__':
    main()
